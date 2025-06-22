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
 * @file    dev_dsp.c
 *
 * @brief   Device driver for communicating with Blackfin DSP.
 */

/// TODO: Separate modules for SPI and EMIFA.

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "per_gpio.h"
#include "per_spi.h"

#include "dev_dsp.h"

#include "ring_buffer.h"

/*----- Macros -------------------------------------------------------*/

#define DSP_SPI SPI_1
#define DSP_SPI_BOOT_DATA_FORMAT 1
#define DSP_SPI_COMMAND_DATA_FORMAT 2
#define DSP_SPI_CHIP_SELECT 2

/// TODO: Centralised header for interrupt priorities.
//          This should probably be lower priority than UART control input.
#define DSP_SPI_INT_CHANNEL 5

#define DSP_SPI_INT_LEVEL SPI_INT_LEVEL_TX_RX_TIMEOUT_DESYNC

#define DSP_SPI_PIN_FUNC                                                       \
    SPI_PIN_SOMI | SPI_PIN_SIMO | SPI_PIN_CLK | SPI_PIN_ENA | SPI_PIN_CS0 |    \
        SPI_PIN_CS1

#define DSP_SPI_BOOT_FREQ SPI_FREQ_50_MHZ
#define DSP_SPI_COMMAND_FREQ SPI_FREQ_50_MHZ
// #define DSP_SPI_COMMAND_FREQ SPI_FREQ_37_5_MHZ
// #define DSP_SPI_COMMAND_FREQ SPI_FREQ_30_MHZ
#define DSP_SPI_CHAR_LENGTH 8
#define DSP_SPI_ENA_TIMEOUT 0xff

#define DSP_SPI_CSHOLD false
#define DSP_SPI_COMMAND_CSHOLD false

/// TODO: Centralised header for queue lengths.
#define DSP_SPI_TX_BUF_LEN 0x100
#define DSP_SPI_RX_BUF_LEN 0x100

/// TODO: Use indexed GPIO functions for single pin id.
#define DSP_RESET_BANK 6
#define DSP_RESET_PIN 10

#define DSP_SPI_ENA_BANK 2
#define DSP_SPI_ENA_PIN 12

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

// DSP SPI RX ring buffer.
static rbd_t dsp_spi_rx_rbd;
static char dsp_spi_rx_rbmem[DSP_SPI_RX_BUF_LEN];

// DSP SPI TX ring buffer.
static rbd_t dsp_spi_tx_rbd;
static char dsp_spi_tx_rbmem[DSP_SPI_TX_BUF_LEN];

// static uint8_t g_dsp_spi_tx_byte;
static uint8_t g_dsp_spi_rx_byte;

volatile static bool g_dsp_spi_tx_complete = false;
volatile static bool g_dsp_spi_rx_complete = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

void _dsp_spi_init(void);

static int _dsp_spi_tx_dequeue(uint8_t *p_byte);
static void _dsp_spi_rx_enqueue(uint8_t *p_byte);

static void _dsp_spi_tx_byte(uint8_t *p_byte);
static void _dsp_spi_rx_byte(void);

static void _dsp_spi_tx_callback(void);
static void _dsp_spi_rx_callback(void);

bool _dsp_spi_enabled(void);

/*----- Extern function implementations ------------------------------*/

void dev_dsp_init(void) {
    //
    _dsp_spi_init();

    /// TODO: EMIFA driver.
    //
    // _dsp_emifa_init();
}

void dev_dsp_spi_tx_enqueue(uint8_t *p_byte) {

    /// TODO: Should catch overflow error and
    ///       redesign so this does not happen.
    //
    // Overwrite on overflow.
    ring_buffer_put_force(dsp_spi_tx_rbd, p_byte);

    if (g_dsp_spi_tx_complete) {

        // Start transmission.
        _dsp_spi_tx_callback();
    }
}

int dev_dsp_spi_rx_dequeue(uint8_t *p_byte) {

    return ring_buffer_get(dsp_spi_rx_rbd, p_byte);
}

// bool dev_dsp_spi_tx_complete(void) { return g_dsp_spi_tx_complete; }

void dev_dsp_spi_poll(void) { _dsp_spi_rx_byte(); }

void dev_dsp_spi_tx_boot(uint8_t *buffer, uint32_t length) {

    per_spi_chip_format(DSP_SPI, DSP_SPI_BOOT_DATA_FORMAT, DSP_SPI_CHIP_SELECT,
                        DSP_SPI_CSHOLD);

    per_spi1_tx_wait(buffer, length);
}

// True puts DSP in reset.
void dev_dsp_reset(bool state) {
    //
    per_gpio_set(DSP_RESET_BANK, DSP_RESET_PIN, !state);
}

/// /// TODO: Use GPIO interrupt to set flag.
/// //
/// // Return true if SPI enabled.
/// bool _dsp_spi_enabled(void) {
///     //
///     return !per_gpio_get(DSP_SPI_ENA_BANK, DSP_SPI_ENA_PIN);
/// }

// void dev_dsp_spi_transfer(void) {
//
//     if (_dsp_spi_enabled()) {
//
//         if ((g_dsp_spi_rx_complete && g_dsp_spi_tx_complete) &&
//             _dsp_spi_tx_dequeue(&g_dsp_spi_tx_byte) == 0) {
//
//             _dsp_spi_tx_byte(&g_dsp_spi_tx_byte);
//         }
//     }
// }

/*----- Static function implementations ------------------------------*/

/// TODO: Return status code.
//
void _dsp_spi_init(void) {

    /// TODO: SPI1 requires mutex to coordinate flash access.
    //
    // SPI 1 also used for flash.
    if (!per_spi_initialised(DSP_SPI)) {

        t_spi_config config = {
            .instance = DSP_SPI,
            .int_channel = DSP_SPI_INT_CHANNEL,
            .int_level = DSP_SPI_INT_LEVEL,
            .pin_func = DSP_SPI_PIN_FUNC,
            .int_enable = true,
        };

        per_spi_init(&config);
    }

    t_spi_format boot_format = {
        .instance = DSP_SPI,
        .index = DSP_SPI_BOOT_DATA_FORMAT,
        .freq = DSP_SPI_BOOT_FREQ,
        .char_length = DSP_SPI_CHAR_LENGTH,
    };

    per_spi_set_data_format(&boot_format);

    t_spi_format command_format = {
        .instance = DSP_SPI,
        .index = DSP_SPI_COMMAND_DATA_FORMAT,
        .freq = DSP_SPI_COMMAND_FREQ,
        .char_length = DSP_SPI_CHAR_LENGTH,
    };

    per_spi_set_data_format(&command_format);

    // Tx ring buffer attributes.
    rb_attr_t tx_attr = {sizeof(dsp_spi_tx_rbmem[0]),
                         ARRAY_SIZE(dsp_spi_tx_rbmem), dsp_spi_tx_rbmem};

    // Rx ring buffer attributes.
    rb_attr_t rx_attr = {sizeof(dsp_spi_rx_rbmem[0]),
                         ARRAY_SIZE(dsp_spi_rx_rbmem), dsp_spi_rx_rbmem};

    // Initialise DSP SPI message ring buffers.
    if (ring_buffer_init(&dsp_spi_tx_rbd, &tx_attr) ||
        ring_buffer_init(&dsp_spi_rx_rbd, &rx_attr) == 0) {

        // Register Tx callback.
        per_spi_register_callback(DSP_SPI, SPI_TX_COMPLETE,
                                  _dsp_spi_tx_callback);

        // Register Rx callback.
        per_spi_register_callback(DSP_SPI, SPI_RX_COMPLETE,
                                  _dsp_spi_rx_callback);

        g_dsp_spi_tx_complete = true;

        _dsp_spi_rx_byte();
    }
}

static int _dsp_spi_tx_dequeue(uint8_t *p_byte) {

    return ring_buffer_get(dsp_spi_tx_rbd, p_byte);
}

static void _dsp_spi_rx_enqueue(uint8_t *p_byte) {

    // Overwrite on overflow?
    ring_buffer_put_force(dsp_spi_rx_rbd, p_byte);
}

static void _dsp_spi_tx_byte(uint8_t *p_byte) {

    g_dsp_spi_tx_complete = false;
    g_dsp_spi_rx_complete = false;

    per_spi_chip_format(DSP_SPI, DSP_SPI_COMMAND_DATA_FORMAT,
                        DSP_SPI_CHIP_SELECT, DSP_SPI_COMMAND_CSHOLD);

    per_spi_trx_int(DSP_SPI, p_byte, &g_dsp_spi_rx_byte, 1);
}

static void _dsp_spi_rx_byte(void) {

    uint8_t dummy = 0x00;

    // Transmit dummy byte to receive.
    _dsp_spi_tx_byte(&dummy);
}

// static void _dsp_spi_tx_callback(void) { g_dsp_spi_tx_complete = true; }
static void _dsp_spi_tx_callback(void) {
    //
    static uint8_t byte;

    // Send next queued byte.
    if (_dsp_spi_tx_dequeue(&byte) == 0) {

        _dsp_spi_tx_byte(&byte);

    } else {

        /// TODO: Do we need to set or test g_dsp_spi_rx_complete
        ///       in this callback?
        ///       If tx complete interrupt always happens before rx interrupt
        ///       then no.

        g_dsp_spi_tx_complete = true;
    }
}

static void _dsp_spi_rx_callback(void) {

    _dsp_spi_rx_enqueue(&g_dsp_spi_rx_byte);

    if (g_dsp_spi_tx_complete == true) {

        /// TODO: Do we even need this?
        ///       It's not currently tested anywhere.
        ///       Rx is complete when tx is complete.

        g_dsp_spi_rx_complete = true;
    }
}

/*----- End of file --------------------------------------------------*/
