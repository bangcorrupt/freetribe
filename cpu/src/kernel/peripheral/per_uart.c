/*----------------------------------------------------------------------

                     This file is part of Freetribe

                https://github.com/bangcorrupt/freetribe

                                License

                   GNU AFFERO GENERAL PUBLIC LICENSE
                      Version 3, 19 November 2007

                           AGPL-3.0-or-later

 Freetribe is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
                  (at your option) any later version.

     Freetribe is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty
        of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
          See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.

                       Copyright bangcorrupt 2023

----------------------------------------------------------------------*/

/**
 * @file per_uart.c
 *
 * @brief  Configuration and handling for UART peripherals.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "soc_AM1808.h"

#include "hal_interrupt.h"
#include "hal_uart.h"

#include "per_uart.h"

#include "ring_buffer.h"

/*----- Macros and Definitions ---------------------------------------*/

#define UART_INSTANCES 2

typedef struct {
    uint32_t system_int;
    uint32_t address;

    uint8_t *tx_buffer;
    uint32_t tx_length;

    uint8_t *rx_buffer;
    uint32_t rx_length;

    void (*tx_callback)(void);
    void (*rx_callback)(void);

} t_uart;

/*----- Static function prototypes -----------------------------------*/

static void _uart_isr(t_uart *g_uart);
static void _uart0_isr(void);
static void _uart1_isr(void);

/*----- Static variable definitions ----------------------------------*/

static const uint32_t g_base_address[UART_INSTANCES] = {SOC_UART_0_REGS,
                                                        SOC_UART_1_REGS};

static const uint32_t g_system_interrupt[UART_INSTANCES] = {SYS_INT_UARTINT0,
                                                            SYS_INT_UARTINT1};

static const void *g_isr_address[UART_INSTANCES] = {&_uart0_isr, &_uart1_isr};

static t_uart g_uart[UART_INSTANCES];

/*----- Extern variable definitions ----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void per_uart_init(t_uart_config *config) {

    g_uart[config->instance].address = g_base_address[config->instance];
    g_uart[config->instance].system_int = g_system_interrupt[config->instance];
    g_uart[config->instance].tx_buffer = NULL;
    g_uart[config->instance].tx_length = 0;
    g_uart[config->instance].rx_buffer = NULL;
    g_uart[config->instance].rx_length = 0;
    g_uart[config->instance].tx_callback = NULL;
    g_uart[config->instance].rx_callback = NULL;

    UARTConfigSetExpClk(g_uart[config->instance].address, 150000000,
                        config->baud, UART_LCR_WLS_8BITS, config->oversample);

    /// TODO: This only supports 1 byte Rx FIFO.
    //
    if (config->fifo_enable) {
        // FIFO enable.
        UARTFIFOEnable(g_uart[config->instance].address);
        // Set Rx FIFO trigger level 1 byte.
        UARTFIFOLevelSet(g_uart[config->instance].address,
                         UART_RX_TRIG_LEVEL_1);
    }

    if (config->int_enable) {
        // Set interrupt channel.
        IntChannelSet(g_uart[config->instance].system_int, config->int_channel);

        // Registers ISR in Interrupt Vector Table.
        IntRegister(g_uart[config->instance].system_int,
                    g_isr_address[config->instance]);

        // Enable system interrupt in AINTC.
        IntSystemEnable(g_uart[config->instance].system_int);
    }

    // Enable UART.
    UARTEnable(g_uart[config->instance].address);
}

void per_uart_terminate(uint8_t instance) {

    // Empty FIFO.
    while ((UART_THR_TSR_EMPTY | UART_THR_EMPTY) !=
           (HWREG(g_uart[instance].address + UART_LSR) &
            (UART_THR_TSR_EMPTY | UART_THR_EMPTY)))
        ;

    // Disable UART.
    UARTDisable(g_uart[instance].address);
}

void per_uart_transmit(uint8_t instance, uint8_t *buffer, uint32_t length) {

    if (buffer != NULL) {
        while (length--) {
            UARTCharPut(g_uart[instance].address, *buffer++);
        }
    }
}

void per_uart_receive(uint8_t instance, uint8_t *buffer, uint32_t length) {

    if (buffer != NULL) {
        while (length--) {
            *buffer++ = UARTCharGet(g_uart[instance].address);
        }
    }
}

void per_uart_transmit_int(uint8_t instance, uint8_t *buffer, uint32_t length) {

    if ((buffer != NULL) && (length != 0)) {
        g_uart[instance].tx_buffer = buffer;
        g_uart[instance].tx_length = length;

        // Enable Tx interrupt.
        UARTIntEnable(g_uart[instance].address, UART_INT_TX_EMPTY);
    }
}

void per_uart_receive_int(uint8_t instance, uint8_t *buffer, uint32_t length) {

    if ((buffer != NULL) && (length != 0)) {
        g_uart[instance].rx_buffer = buffer;
        g_uart[instance].rx_length = length;

        // Enable Rx interrupt.
        UARTIntEnable(g_uart[instance].address, UART_INT_RXDATA_CTI);
    }
}

void per_uart_register_callback(uint8_t instance, t_uart_event event,
                                void (*callback)(void)) {

    switch (event) {
    case UART_TX_COMPLETE:
        g_uart[instance].tx_callback = callback;
        break;

    case UART_RX_COMPLETE:
        g_uart[instance].rx_callback = callback;
        break;

        // case UART_ERROR:

    default:
        break;
    }
}

/*----- Static function implementations ------------------------------*/

/// TODO: Implement DMA and higher level MCU device driver.
//          5 byte message buffer.
//              DMA continually services UART.
//                  DMA Complete interrupt pushes/pops ring buffer.

/*
 * @brief   Interrupt Service Routine for UART0.
 *          - Transmit buffer empty:
 *              - Read from Tx ring buffer and writes to UART.
 *          - Receive buffer full:
 *              - Read from UART and write to Rx ring buffer.
 *          - Receiver line error:
 *              - Clear byte from RBR if receiver line error has occured.
 */
static inline void _uart_isr(t_uart *uart) {

#if NESTED_INTERRUPTS
    // System interrupt already cleared in IRQHandler.
#else
    // Clear the system interrupt status of UART0 in AINTC.
    IntSystemStatusClear(uart->system_int);
#endif

    // Get highest priority pending interrupt.
    uint8_t int_id;

    uint8_t tx_fifo_level = 0;

    // Clear all pending interrupts.
    while ((int_id = UARTIntStatus(uart->address))) {
        switch (int_id) {

        // Tx interrupt.
        case UART_INTID_TX_EMPTY:

            // Fill FIFO with up to 16 bytes.
            while (uart->tx_length-- &&
                   (tx_fifo_level++ < UART_TX_FIFO_LENGTH)) {
                // Write a byte into the Tx FIFO.
                UARTCharPut(uart->address, *uart->tx_buffer++);

                if (uart->tx_length == 0) {
                    // Disable Tx interrupt if buffer empty.
                    UARTIntDisable(uart->address, UART_INT_TX_EMPTY);

                    // Trigger callback if registered.
                    if (uart->tx_callback != NULL) {
                        uart->tx_callback();
                    }
                }
            }
            tx_fifo_level = 0;
            break;

        case UART_INTID_CTI:
            /// TODO: Log character timeout.
            //
            // No break, fall through Rx case.

        // Rx interrupt.
        case UART_INTID_RX_DATA:

            if (uart->rx_length--) {
                // Get received byte,
                *uart->rx_buffer++ = UARTCharGet(uart->address);

                if (uart->rx_length == 0) {
                    // Disable Rx interrupt if buffer full.
                    UARTIntDisable(uart->address, UART_INT_RXDATA_CTI);

                    if (uart->rx_callback != NULL) {
                        uart->rx_callback();
                    }
                }
            }
            break;

        // Error interrupt.
        case UART_INTID_RX_LINE_STAT:

            /// TODO: Error callback.
            while (UARTRxErrorGet(uart->address)) {
                // Read a byte from the RBR if RBR has data.
                UARTCharGetNonBlocking(uart->address);
            }
            break;

        default:
            break;
        }
    }
    return;
}

static void _uart0_isr(void) { _uart_isr(&g_uart[0]); }

static void _uart1_isr(void) { _uart_isr(&g_uart[1]); }

/*----- End of file --------------------------------------------------*/
