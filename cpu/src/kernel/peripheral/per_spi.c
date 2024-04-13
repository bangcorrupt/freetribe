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
 * @file    per_spi.c.
 *
 * @brief   Configuration and handling for SPI peripherals.
 *
 */

/// TODO: Rework similar to UART driver.

/// TODO: SPI_1 needs mutex to prevent DSP and flash access collision.

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "startup.h"

#include "hal_cp15.h"
#include "hal_interrupt.h"
#include "hal_spi.h"

#include "per_gpio.h"
#include "per_spi.h"

#include "ring_buffer.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static uint8_t *g_spi0_tx_buffer = NULL;
static uint32_t g_spi0_tx_length = 0;

static uint8_t *g_spi1_tx_buffer = NULL;
static uint32_t g_spi1_tx_length = 0;

static uint8_t *g_spi1_rx_buffer = NULL;
static uint32_t g_spi1_rx_length = 0;

static void (*_spi1_tx_callback)(void) = NULL;
static void (*_spi1_rx_callback)(void) = NULL;

static bool g_spi_initialised[2];

/// TODO: Encapsulate.
static uint32_t g_spi_base[2] = {SOC_SPI_0_REGS, SOC_SPI_1_REGS};

static uint32_t g_spi_data_format[4] = {SPI_DATA_FORMAT0, SPI_DATA_FORMAT1,
                                        SPI_DATA_FORMAT2, SPI_DATA_FORMAT3};

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _spi0_isr(void);
static void _spi1_isr(void);

/*----- Extern function implementations ------------------------------*/

void per_spi0_init(void) {

    // SPI0 in reset
    SPIReset(SOC_SPI_0_REGS);

    // SPI0 out of reset
    SPIOutOfReset(SOC_SPI_0_REGS);

    // SPI0 is Master
    SPIModeConfigure(SOC_SPI_0_REGS, SPI_MASTER_MODE);

    // SPI0 SIMO, CLK and CS0 are SPI functional pins
    HWREG(SOC_SPI_0_REGS + SPI_SPIPC(0)) =
        SPI_SPIPC0_SIMOFUN | SPI_SPIPC0_CLKFUN |
        SPI_SPIPC0_SCS0FUN0; // | SPI_SPIPC0_SCS0FUN2;

    /// TODO: Can we increase clock speed?
    ///          Factory firmware sets prescaler to 0x1d.
    //
    // SPI0 Data format
    HWREG(SOC_SPI_0_REGS + SPI_SPIFMT(0)) =
        SPI_SPIFMT_PHASE |
        (SPI_SPIFMT_PRESCALE & (0x4 << SPI_SPIFMT_PRESCALE_SHIFT)) |
        (SPI_SPIFMT_CHARLEN & 8);

    /// TODO: Can we use chip slelect for LCD A0 pin (Command/Data mode).
    //
    // Transfer chip select value.
    /* SPIDat1Config(SOC_SPI_0_REGS, SPI_DATA_FORMAT0 | SPI_CSHOLD, 0x5); */
    SPIDat1Config(SOC_SPI_0_REGS, SPI_DATA_FORMAT0 | SPI_CSHOLD, 0x1);

    /* SPIDelayConfigure(SOC_SPI_0_REGS, 0, 0, 32, 32); */

    /* SPICSTimerEnable(SOC_SPI_0_REGS, SPI_DATA_FORMAT0); */

    // Enable SPI0
    SPIEnable(SOC_SPI_0_REGS);

    // Set interrupt channel 8.
    IntChannelSet(SYS_INT_SPIINT0, 8);

    // Register the SPI0Isr in the Interrupt Vector Table of AINTC.
    IntRegister(SYS_INT_SPIINT0, _spi0_isr);

    // Enable system interrupt in AINTC.
    IntSystemEnable(SYS_INT_SPIINT0);

    // Enable transmit interrupt.
    // Don't enable until transmit function.
    /* SPIIntEnable(SOC_SPI_0_REGS, SPI_TRANSMIT_INT); */

    // Set interrupt level for transmit only.
    SPIIntLevelSet(SOC_SPI_0_REGS, SPI_TRANSMIT_INTLVL);

    g_spi_initialised[0] = true;
}

void per_spi1_init(void) {

    // SPI1 in reset.
    SPIReset(SOC_SPI_1_REGS);

    // SPI1 out of reset.
    SPIOutOfReset(SOC_SPI_1_REGS);

    // SPI1 is Master.
    SPIModeConfigure(SOC_SPI_1_REGS, SPI_MASTER_MODE);

    // SPI1 SOMI, SIMO, CLK, ENA, CS0 and CS1 are SPI functional pins.
    HWREG(SOC_SPI_1_REGS + SPI_SPIPC(0)) =
        SPI_SPIPC0_SOMIFUN | SPI_SPIPC0_SIMOFUN | SPI_SPIPC0_CLKFUN |
        SPI_SPIPC0_SCS0FUN0 | SPI_SPIPC0_SCS0FUN1 | SPI_SPIPC0_ENAFUN;

    // SPI1 Data format 0 Flash.
    HWREG(SOC_SPI_1_REGS + SPI_SPIFMT(0)) =
        SPI_SPIFMT_PHASE |
        (SPI_SPIFMT_PRESCALE & (0x3 << SPI_SPIFMT_PRESCALE_SHIFT)) |
        (SPI_SPIFMT_CHARLEN & 8);

    // SPI1 Data format 1 DSP Boot.
    HWREG(SOC_SPI_1_REGS + SPI_SPIFMT(1)) =
        SPI_SPIFMT_PHASE |
        (SPI_SPIFMT_PRESCALE & (0x3 << SPI_SPIFMT_PRESCALE_SHIFT)) |
        (SPI_SPIFMT_CHARLEN & 8);

    // SPI1 Data format 2 DSP Command.
    /* HWREG(SOC_SPI_1_REGS + SPI_SPIFMT(2)) = */
    /*     SPI_SPIFMT_PHASE | */
    /*     (SPI_SPIFMT_PRESCALE & (0x3 << SPI_SPIFMT_PRESCALE_SHIFT)) | */
    /*     (SPI_SPIFMT_CHARLEN & 16); */

    // Set timeout.
    SPIDelayConfigure(g_spi_base[1], 0xff, 0, 0, 0);

    // Enable timeout for data format 1.
    SPIWaitEnable(g_spi_base[1], SPI_DATA_FORMAT1);

    // Enable SPI1.
    SPIEnable(SOC_SPI_1_REGS);

    // Set interrupt channel 5.
    IntChannelSet(SYS_INT_SPIINT1, 5);

    // Register the SPI1Isr in the Interrupt Vector Table of AINTC.
    IntRegister(SYS_INT_SPIINT1, _spi1_isr);

    // Enable system interrupt in AINTC.
    IntSystemEnable(SYS_INT_SPIINT1);

    // Set interrupt level for transmit and receive.
    SPIIntLevelSet(SOC_SPI_1_REGS, SPI_TRANSMIT_INTLVL | SPI_RECV_INTLVL |
                                       SPI_TIMEOUT_INTLVL |
                                       SPI_DESYNC_SLAVE_INTLVL);

    g_spi_initialised[1] = true;
}

bool per_spi_initialised(uint8_t instance) {
    //
    return g_spi_initialised[instance];
}

// Select data format, set chip select value and hold.
void per_spi_chip_format(uint8_t instance, uint8_t data_format,
                         uint8_t chip_select, bool cshold) {

    if (cshold) {
        cshold = SPI_CSHOLD;
    }
    SPIDat1Config(g_spi_base[instance], g_spi_data_format[data_format] | cshold,
                  chip_select);
}

void per_spi0_tx(uint8_t *buffer, uint32_t length) {

    g_spi0_tx_buffer = buffer;
    g_spi0_tx_length = length;

    while (g_spi0_tx_length--) {
        // Write byte to SPI0
        SPITransmitData1(SOC_SPI_0_REGS, *g_spi0_tx_buffer++);
    }
}

void per_spi0_tx_int(uint8_t *buffer, uint32_t length) {

    g_spi0_tx_buffer = buffer;
    g_spi0_tx_length = length;

    // Enable TX interrupt.
    SPIIntEnable(SOC_SPI_0_REGS, SPI_TRANSMIT_INT);
}

/// TODO: Non-blocking implementation would be better.
///         Not a big problem as this function is
///         only used to boot the DSP.
//
void per_spi1_tx(uint8_t *buffer, uint32_t length) {

    if (buffer != NULL) {
        while (SPIIntStatus(SOC_SPI_1_REGS, SPI_RECV_INT))
            ;

        while (length--) {
            /// TODO: SPI ENA timeout/error?

            while (per_gpio_get(2, 12))
                ;

            // Wait until SPI transmit buffer ready.
            while (!SPIIntStatus(SOC_SPI_1_REGS, SPI_TRANSMIT_INT))
                ;
            // Write byte to SPI1
            SPITransmitData1(SOC_SPI_1_REGS, *buffer++);

            // Wait until SPI transmit buffer ready.
            while (!SPIIntStatus(SOC_SPI_1_REGS, SPI_TRANSMIT_INT))
                ;

            while (per_gpio_get(2, 12))
                ;

            if (SPIIntStatus(SOC_SPI_1_REGS, SPI_RECV_INT)) {
                SPIDataReceive(SOC_SPI_1_REGS);
            }
        }
    }
}

void per_spi1_tx_int(uint8_t *buffer, uint32_t length) {

    if (buffer != NULL && length != 0) {
        g_spi1_tx_buffer = buffer;
        g_spi1_tx_length = length;

        // Enable TX interrupt.
        SPIIntEnable(SOC_SPI_1_REGS, SPI_TRANSMIT_INT | SPI_TIMEOUT_INT);
    }
}

/// TODO: Support both instances.
///          See UART driver.
void per_spi1_transceive_int(uint8_t *tx_buffer, uint8_t *rx_buffer,
                             uint32_t length) {

    if (tx_buffer != NULL && tx_buffer != NULL && length != 0) {
        g_spi1_tx_buffer = tx_buffer;
        g_spi1_rx_buffer = rx_buffer;
        g_spi1_tx_length = length;
        g_spi1_rx_length = length;

        /// TODO: Error interrupts should be enabled in init function.
        ///         Should probably always remain enabled.
        //
        // Enable interrupts.
        SPIIntEnable(SOC_SPI_1_REGS, SPI_TRANSMIT_INT | SPI_RECV_INT |
                                         SPI_TIMEOUT_INT |
                                         SPI_DESYNC_SLAVE_INT);
    }
}

void per_spi_chip_select(uint32_t spi_base, uint8_t cs, bool state) {
    /// TODO: This sets chip select hold mode, does not assert chip select.

    if (state) {
        // Assert chip select
        SPIDat1Config(spi_base, (SPI_CSHOLD | SPI_DATA_FORMAT0), cs);
    } else {
        // Release chip select
        SPIDat1Config(spi_base, SPI_DATA_FORMAT0, cs);
    }
}

void per_spi_tx(uint32_t spi_base, uint8_t *p_tx, uint32_t len) {

    int i;

    while (SPIIntStatus(spi_base, SPI_RECV_INT))
        ;

    for (i = 0; i < len; i++) {
        while (!SPIIntStatus(spi_base, SPI_TRANSMIT_INT))
            ;
        SPITransmitData1(spi_base, p_tx[i]);

        while (!SPIIntStatus(spi_base, SPI_RECV_INT))
            ;
        SPIDataReceive(spi_base);
    }
}

void per_spi_rx(uint32_t spi_base, uint8_t *p_rx, uint32_t len) {

    int i;
    uint8_t tx = 0;

    while (SPIIntStatus(spi_base, SPI_RECV_INT))
        ;

    for (i = 0; i < len; i++) {
        while (!SPIIntStatus(spi_base, SPI_TRANSMIT_INT))
            ;

        SPITransmitData1(spi_base, tx);

        while (!SPIIntStatus(spi_base, SPI_RECV_INT))
            ;

        p_rx[i] = SPIDataReceive(spi_base);
    }
}

void per_spi1_register_callback(t_spi_event event, void (*callback)()) {

    switch (event) {

    case SPI_TX_COMPLETE:
        _spi1_tx_callback = callback;
        break;

    case SPI_RX_COMPLETE:
        _spi1_rx_callback = callback;
        break;

        // case SPI_ERROR:

    default:
        break;
    }
}

/*----- Static function implementations ------------------------------*/

/*
 * @brief   Interrupt Service Routine for SPI0.
 *          - Transmit buffer empty:
 *              - Read from SPI0 Tx buffer and write to SPI0 peripheral.
 *          - Receive buffer full:
 *              - Read from SPI0 peripheral and write to SPI0 Rx buffer.
 */
static void _spi0_isr(void) {

    // Cause of SPI0 interrupt.
    uint8_t int_id = 0;

#if NESTED_INTERRUPTS
    // System interrupt already cleared in IRQHandler.
#else
    // Clears the system interrupt status of SPI0 in AINTC.
    IntSystemStatusClear(SYS_INT_SPIINT0);
#endif

    int_id = SPIInterruptVectorGet(SOC_SPI_0_REGS);

    // Handle all pending interrupts.
    while (int_id) {

        switch (int_id) {

        // Tx interrupt.
        case SPI_TX_BUF_EMPTY:

            if (g_spi0_tx_length--) {
                // Write byte to SPI0
                SPITransmitData1(SOC_SPI_0_REGS, *g_spi0_tx_buffer++);

            } else {
                // Disable the Tx interrupt if buffer is empty.
                SPIIntDisable(SOC_SPI_0_REGS, SPI_TRANSMIT_INT);

                // TODO: spi_tx_callback.
            }
            break;

            // TODO: No Rx from LCD.
            //          Is it worth supporting full SPI for expansion?
            //              Is Rx pin exposed?
            // Rx interrupt.
            /* case SPI_RECV_FULL: */

        default:
            break;
        }
        int_id = SPIInterruptVectorGet(SOC_SPI_0_REGS);
    }

    return;
}

/*
 * @brief   Interrupt Service Routine for SPI1.
 *          - Transmit buffer empty:
 *              - Read from SPI1 Tx buffer and write to SPI1 peripheral.
 *          - Receive buffer full:
 *              - Read from SPI1 peripheral and write to SPI1 Rx buffer.
 */
static void _spi1_isr(void) {

    // Cause of SPI1 interrupt.
    uint8_t int_id = 0;

#if NESTED_INTERRUPTS
    // System interrupt already cleared in IRQHandler.
#else
    // Clears the system interrupt status of SPI1 in AINTC.
    IntSystemStatusClear(SYS_INT_SPIINT1);
#endif

    /// TODO: Assign int_id in while condition.
    //
    int_id = SPIInterruptVectorGet(SOC_SPI_1_REGS);

    // Handle all pending interrupts.
    while (int_id) {

        switch (int_id) {

        // Tx interrupt.
        case SPI_TX_BUF_EMPTY:
            // TODO: Test HWAIT/SPI_ENA ?

            if (g_spi1_tx_length--) {
                // Write byte to SPI0
                SPITransmitData1(SOC_SPI_1_REGS, *g_spi1_tx_buffer++);

                if (g_spi1_tx_length == 0) {
                    // Disable the Tx interrupt if buffer is empty.
                    SPIIntDisable(SOC_SPI_1_REGS, SPI_TRANSMIT_INT);

                    if (_spi1_tx_callback != NULL) {
                        _spi1_tx_callback();
                    }
                }
            }
            break;

        // Rx interrupt.
        case SPI_RECV_FULL:

            if (g_spi1_rx_length--) {
                // Read byte from SPI1
                *g_spi1_rx_buffer++ = SPIDataReceive(SOC_SPI_1_REGS);

                if (g_spi1_rx_length == 0) {
                    // Disable the Rx interrupt if buffer full.
                    SPIIntDisable(SOC_SPI_1_REGS, SPI_RECV_INT);

                    if (_spi1_rx_callback != NULL) {
                        _spi1_rx_callback();
                    }
                }
            }

            break;

        // TODO: Interrogate source of error and trigger callback.
        case SPI_ERR:
            while (true)
                ;

        default:
            break;
        }
        int_id = SPIInterruptVectorGet(SOC_SPI_1_REGS);
    }

    return;
}
