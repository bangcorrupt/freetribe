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

} t_uart_handle;

/*----- Static variable definitions ----------------------------------*/

static t_uart_handle h_uart[UART_INSTANCES];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _uart_isr(t_uart_handle *h_uart);
static void _uart0_isr(void);
static void _uart1_isr(void);

static uint32_t _uart_get_base_address(uint8_t instance);
static uint32_t _uart_get_system_interrupt(uint8_t instance);
static uint32_t _uart_get_isr_address(uint8_t instance);

/*----- Extern function implementations ------------------------------*/

void per_uart_init(t_uart_config *config) {

    t_uart_handle uart = {.address = _uart_get_base_address(config->instance),
                          .system_int =
                              _uart_get_system_interrupt(config->instance),
                          .tx_buffer = NULL,
                          .tx_length = 0,
                          .rx_buffer = NULL,
                          .rx_length = 0,
                          .tx_callback = NULL,
                          .rx_callback = NULL};

    UARTConfigSetExpClk(uart.address, 150000000, config->baud,
                        UART_LCR_WLS_8BITS, config->oversample);

    // TODO: This only supports 1 byte Rx FIFO.
    if (config->fifo_enable) {
        // FIFO enable.
        UARTFIFOEnable(uart.address);
        // Set Rx FIFO trigger level 1 byte.
        UARTFIFOLevelSet(uart.address, UART_RX_TRIG_LEVEL_1);
    }

    if (config->int_enable) {
        // Set interrupt channel.
        IntChannelSet(uart.system_int, config->int_channel);

        // Registers ISR in Interrupt Vector Table.
        IntRegister(uart.system_int, _uart_get_isr_address(config->instance));

        // Enable system interrupt in AINTC.
        IntSystemEnable(uart.system_int);
    }

    // Enable UART.
    UARTEnable(uart.address);

    h_uart[config->instance] = uart;
}

void per_uart_terminate(uint8_t instance) {

    // Empty FIFO.
    while ((UART_THR_TSR_EMPTY | UART_THR_EMPTY) !=
           (HWREG(h_uart[instance].address + UART_LSR) &
            (UART_THR_TSR_EMPTY | UART_THR_EMPTY)))
        ;

    // Disable UART.
    UARTDisable(h_uart[instance].address);
}

void per_uart_transmit(uint8_t instance, uint8_t *buffer, uint32_t length) {

    if (buffer != NULL) {
        while (length--) {
            UARTCharPut(h_uart[instance].address, *buffer++);
        }
    }
}

void per_uart_receive(uint8_t instance, uint8_t *buffer, uint32_t length) {

    if (buffer != NULL) {
        while (length--) {
            *buffer++ = UARTCharGet(h_uart[instance].address);
        }
    }
}

void per_uart_transmit_int(uint8_t instance, uint8_t *buffer, uint32_t length) {

    if ((buffer != NULL) && (length != 0)) {
        h_uart[instance].tx_buffer = buffer;
        h_uart[instance].tx_length = length;

        // Enable Tx interrupt.
        UARTIntEnable(h_uart[instance].address, UART_INT_TX_EMPTY);
    }
}

void per_uart_receive_int(uint8_t instance, uint8_t *buffer, uint32_t length) {

    if ((buffer != NULL) && (length != 0)) {
        h_uart[instance].rx_buffer = buffer;
        h_uart[instance].rx_length = length;

        // Enable Rx interrupt.
        UARTIntEnable(SOC_UART_0_REGS, UART_INT_RXDATA_CTI);
    }
}

void per_uart_register_callback(uint8_t instance, t_uart_event event,
                                void (*callback)(void)) {

    switch (event) {
    case UART_TX_COMPLETE:
        h_uart[instance].tx_callback = callback;
        break;

    case UART_RX_COMPLETE:
        h_uart[instance].rx_callback = callback;
        break;

        // case UART_ERROR:

    default:
        break;
    }
}

/*----- Static function implementations ------------------------------*/

static uint32_t _uart_get_system_interrupt(uint8_t instance) {

    const uint32_t system_interrupt[UART_INSTANCES] = {SYS_INT_UARTINT0,
                                                       SYS_INT_UARTINT1};
    return system_interrupt[instance];
}

static uint32_t _uart_get_base_address(uint8_t instance) {

    const uint32_t base_address[UART_INSTANCES] = {SOC_UART_0_REGS,
                                                   SOC_UART_1_REGS};
    return base_address[instance];
}

static uint32_t _uart_get_isr_address(uint8_t instance) {

    const uint32_t isr_address[UART_INSTANCES] = {_uart0_isr, _uart1_isr};

    return isr_address[instance];
}

// TODO: Implement DMA and higher level MCU device driver.
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
static inline void _uart_isr(t_uart_handle *uart) {

#if NESTED_INTERRUPTS
    // System interrupt already cleared in IRQHandler.
#else
    // Clear the system interrupt status of UART0 in AINTC.
    IntSystemStatusClear(uart->system_int);
#endif

    uint8_t tx_fifo_level = 0;

    // Get highest priority pending interrupt.
    uint8_t int_id = UARTIntStatus(uart->address);

    // Clear all pending interrupts.
    while (int_id) {
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
            // TODO: Maybe disable interrupt if buffer already empty.
            break;

        case UART_INTID_CTI:
            // TODO: Log character timeout.
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
            // TODO: Disable interrupt if triggered when no rx buffer?
            break;

        // Error interrupt.
        case UART_INTID_RX_LINE_STAT:

            // TODO: Error callback.
            while (UARTRxErrorGet(uart->address)) {
                // Read a byte from the RBR if RBR has data.
                UARTCharGetNonBlocking(uart->address);
            }
            break;

        default:
            break;
        }
        // Check for pending interrupt.
        int_id = UARTIntStatus(uart->address);
    }
    return;
}

static void _uart0_isr(void) { _uart_isr(&h_uart[0]); }

static void _uart1_isr(void) { _uart_isr(&h_uart[1]); }

/*----- End of file --------------------------------------------------*/
