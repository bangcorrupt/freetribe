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
 */

/// TODO: SPI_1 needs mutex to prevent DSP and flash access collision.

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "startup.h"

#include "csl_cp15.h"
#include "csl_interrupt.h"
#include "csl_spi.h"

#include "per_gpio.h"
#include "per_spi.h"

#include "csl_gpio.h"

#include "ring_buffer.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef struct {
    uint32_t address;
    uint32_t system_int;
    void (*isr)(void);

    uint8_t *tx_buffer;
    uint32_t tx_length;

    uint8_t *rx_buffer;
    uint32_t rx_length;

    void (*tx_callback)(void);
    void (*rx_callback)(void);

    bool initialised;

} t_spi;

/*----- Static function prototypes -----------------------------------*/

static void _spi0_isr(void);
static void _spi1_isr(void);

/*----- Static variable definitions ----------------------------------*/

static const uint32_t g_base_address[SPI_INSTANCES] = {SPI0_BASE, SPI1_BASE};

static const uint32_t g_system_interrupt[SPI_INSTANCES] = {SYS_INT_SPIINT0,
                                                           SYS_INT_SPIINT1};

static const void *g_isr_address[SPI_INSTANCES] = {&_spi0_isr, &_spi1_isr};

static t_spi g_spi[SPI_INSTANCES];

/*----- Extern variable definitions ----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void per_spi_init(t_spi_config *config) {

    t_spi *spi = &g_spi[config->instance];

    spi->address = g_base_address[config->instance];
    spi->system_int = g_system_interrupt[config->instance];
    spi->isr = g_isr_address[config->instance];
    spi->tx_buffer = NULL;
    spi->tx_length = 0;
    spi->rx_buffer = NULL;
    spi->rx_length = 0;
    spi->tx_callback = NULL;
    spi->rx_callback = NULL;
    spi->initialised = false;

    // SPI in reset
    SPIReset(spi->address);

    // SPI out of reset
    SPIOutOfReset(spi->address);

    // SPI is Master
    SPIModeConfigure(spi->address, SPI_MASTER_MODE);

    SPISetPinControl(spi->address, SPI_PIN_CTL_FUNC, config->pin_func);

    // Set valid defaults here, customise for each data format below.
    SPIClkConfigure(spi->address, SPI_MODULE_FREQ, SPI_DEFAULT_FREQ,
                    SPI_DEFAULT_DATA_FORMAT);

    // Set valid defaults here, customise for each data format below.
    SPIConfigClkFormat(spi->address, SPI_DEFAULT_PHASE,
                       SPI_DEFAULT_DATA_FORMAT);

    SPICharLengthSet(spi->address, SPI_DEFAULT_CHAR_LENGTH,
                     SPI_DEFAULT_DATA_FORMAT);

    // Enable SPI.
    SPIEnable(spi->address);

    if (config->int_enable) {
        // Set interrupt channel.
        IntChannelSet(spi->system_int, config->int_channel);

        // Register the SPI Isr in the Interrupt Vector Table of AINTC.
        IntRegister(spi->system_int, spi->isr);

        // Enable system interrupt in AINTC.
        IntSystemEnable(spi->system_int);

        // Set interrupt level for transmit and receive.
        SPIIntLevelSet(spi->address, config->int_level);
    }

    spi->initialised = true;
}

void per_spi_set_data_format(t_spi_format *format) {

    SPIClkConfigure(g_base_address[format->instance], SPI_MODULE_FREQ,
                    format->freq, format->index);

    SPIConfigClkFormat(g_base_address[format->instance], SPI_DEFAULT_PHASE,
                       format->index);

    SPICharLengthSet(g_base_address[format->instance], format->char_length,
                     format->index);

    /// TODO: Investigate SPI timings using logic analyser.
}

bool per_spi_initialised(uint8_t instance) {

    return g_spi[instance].initialised;
}

// Select data format, set chip select value and hold.
void per_spi_chip_format(uint8_t instance, uint8_t data_format,
                         uint8_t chip_select, bool cshold) {

    /// TODO: Tidy this up.
    //
    if (cshold) {
        SPIDat1Config(g_spi[instance].address,
                      (uint32_t)data_format | SPI_CSHOLD, chip_select);
    } else {
        SPIDat1Config(g_spi[instance].address, data_format, chip_select);
    }
}

/// TODO: Non-blocking implementation would be better.
///         Not a big problem as this function is
///         only used to boot the DSP.
///             This could be a single byte write,
///             with GPIO tested in device layer above.
///
//
void per_spi1_tx_wait(uint8_t *buffer, uint32_t length) {

    if (buffer != NULL) {
        while (SPIIntStatus(SPI1_BASE, SPI_RECV_INT))
            ;

        while (length--) {
            /// TODO: SPI ENA timeout/error?

            while (per_gpio_get(2, 12))
                ;

            // Wait until SPI transmit buffer ready.
            while (!SPIIntStatus(SPI1_BASE, SPI_TRANSMIT_INT))
                ;
            // Write byte to SPI1
            SPITransmitData1(SPI1_BASE, *buffer++);

            // Wait until SPI transmit buffer ready.
            while (!SPIIntStatus(SPI1_BASE, SPI_TRANSMIT_INT))
                ;

            while (per_gpio_get(2, 12))
                ;

            if (SPIIntStatus(SPI1_BASE, SPI_RECV_INT)) {
                SPIDataReceive(SPI1_BASE);
            }
        }
    }
}

void per_spi_tx_int(uint8_t instance, uint8_t *buffer, uint32_t length) {

    g_spi[instance].tx_buffer = buffer;
    g_spi[instance].tx_length = length;

    // Enable TX interrupt.
    SPIIntEnable(g_spi[instance].address, SPI_TRANSMIT_INT);
}

void per_spi_trx_int(uint8_t instance, uint8_t *tx_buffer, uint8_t *rx_buffer,
                     uint32_t length) {

    if (tx_buffer != NULL && tx_buffer != NULL && length != 0) {
        g_spi[instance].tx_buffer = tx_buffer;
        g_spi[instance].rx_buffer = rx_buffer;
        g_spi[instance].tx_length = length;
        g_spi[instance].rx_length = length;

        /// TODO: Error interrupts should be enabled in init function.
        ///         Should probably always remain enabled.
        //
        // Enable interrupts.
        SPIIntEnable(g_spi[instance].address, SPI_TRANSMIT_INT | SPI_RECV_INT |
                                                  SPI_TIMEOUT_INT |
                                                  SPI_DESYNC_SLAVE_INT);
    }
}

/// TODO: Do these non-interrupt functions work
///       if interrupts are not enabled?

/// TODO: Update to use g_spi structs.
//
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

/// TODO: Update to use g_spi structs.
//
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

void per_spi_register_callback(uint8_t instance, t_spi_event event,
                               void (*callback)()) {

    switch (event) {

    case SPI_TX_COMPLETE:
        g_spi[instance].tx_callback = callback;
        break;

    case SPI_RX_COMPLETE:
        g_spi[instance].rx_callback = callback;
        break;

        // case SPI_ERROR:

    default:
        break;
    }
}

/*----- Static function implementations ------------------------------*/

static inline void _spi_isr(t_spi *spi) {

    // Cause of SPI interrupt.
    uint8_t int_id = 0;

#if NESTED_INTERRUPTS
    // System interrupt already cleared in IRQHandler.
#else
    // Clears the system interrupt status of SPI1 in AINTC.
    IntSystemStatusClear(spi->system_int);
#endif

    // Handle all pending interrupts.
    while ((int_id = SPIInterruptVectorGet(spi->address))) {

        switch (int_id) {

        // Tx interrupt.
        case SPI_TX_BUF_EMPTY:

            if (spi->tx_length--) {
                // Write byte to SPI
                SPITransmitData1(spi->address, *spi->tx_buffer++);

                if (spi->tx_length == 0) {
                    // Disable the Tx interrupt if buffer is empty.
                    SPIIntDisable(spi->address, SPI_TRANSMIT_INT);

                    if (spi->tx_callback != NULL) {
                        spi->tx_callback();
                    }
                }
            }
            break;

        // Rx interrupt.
        case SPI_RECV_FULL:

            if (spi->rx_length--) {
                // Read byte from SPI
                *spi->rx_buffer++ = SPIDataReceive(spi->address);

                if (spi->rx_length == 0) {
                    // Disable the Rx interrupt if buffer full.
                    SPIIntDisable(spi->address, SPI_RECV_INT);

                    if (spi->rx_callback != NULL) {
                        spi->rx_callback();
                    }
                }
            }
            /// TODO: Should disable interrupts if triggered
            ///       when buffer length is zero.
            ///       This should never happen,
            ///       as interrupt is only enabled if
            ///       buffers set correctly.
            break;

        /// TODO: Interrogate source of error and trigger callback.
        case SPI_ERR:

            while (true)
                ;

            break;

        default:
            break;
        }
    }

    return;
}

static void _spi0_isr(void) { _spi_isr(&g_spi[0]); }

static void _spi1_isr(void) { _spi_isr(&g_spi[1]); }

/*----- End of file --------------------------------------------------*/
