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
 * @file    dev_cpu_spi.c
 *
 * @brief   Device driver for communicating with CPU via SPI.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "per_spi.h"

#include "ring_buffer.h"

/*----- Macros -------------------------------------------------------*/

/// TODO: Central header for queue sizes.
//
#define SPI_RX_BUF_LEN 0x200
#define SPI_TX_BUF_LEN 0x200

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/
//
// SPI Rx ring buffer.
static rbd_t g_spi_rx_rbd;
static uint8_t g_spi_rx_rbmem[SPI_RX_BUF_LEN];

// SPI Tx ring buffer.
static rbd_t g_spi_tx_rbd;
static uint8_t g_spi_tx_rbmem[SPI_TX_BUF_LEN];

static uint8_t g_cpu_spi_rx_byte;
static uint8_t g_cpu_spi_tx_byte;

static bool g_spi_tx_complete = true;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static int _cpu_spi_tx_dequeue(uint8_t *spi_byte);
static void _cpu_spi_rx_enqueue(uint8_t *spi_byte);

static void _cpu_spi_trx_byte(uint8_t *tx_byte, uint8_t *rx_byte);

static void _cpu_spi_trx_callback();

/*----- Extern function implementations ------------------------------*/

void dev_cpu_spi_init(void) {
    //
    // Tx ring buffer attributes.
    rb_attr_t tx_attr = {sizeof(g_spi_tx_rbmem[0]), ARRAY_SIZE(g_spi_tx_rbmem),
                         g_spi_tx_rbmem};

    // Rx ring buffer attributes.
    rb_attr_t rx_attr = {sizeof(g_spi_rx_rbmem[0]), ARRAY_SIZE(g_spi_rx_rbmem),
                         g_spi_rx_rbmem};

    // Initialise MCU message ring buffers.
    if (ring_buffer_init(&g_spi_tx_rbd, &tx_attr) ||
        ring_buffer_init(&g_spi_rx_rbd, &rx_attr) == 0) {

        per_spi_init();

        per_spi_register_callback(EVT_SPI_TRX_COMPLETE, _cpu_spi_trx_callback);

        // Initialise buffers to catch first interrupt.
        _cpu_spi_trx_byte(&g_cpu_spi_tx_byte, &g_cpu_spi_rx_byte);
    }
}

/// TODO: Check/return status.
void dev_cpu_spi_tx_enqueue(uint8_t *spi_byte) {

    // Overwrite on overflow?
    ring_buffer_put_force(g_spi_tx_rbd, spi_byte);
}

int dev_cpu_spi_rx_dequeue(uint8_t *spi_byte) {

    return ring_buffer_get(g_spi_rx_rbd, spi_byte);
}

/*----- Static function implementations ------------------------------*/

static int _cpu_spi_tx_dequeue(uint8_t *spi_byte) {

    return ring_buffer_get(g_spi_tx_rbd, spi_byte);
}

/// TODO: Check/return status.
static void _cpu_spi_rx_enqueue(uint8_t *spi_byte) {

    // Overwrite on overflow?
    ring_buffer_put_force(g_spi_rx_rbd, spi_byte);
}

static void _cpu_spi_trx_byte(uint8_t *tx_byte, uint8_t *rx_byte) {

    per_spi_trx_int(tx_byte, rx_byte, 1);
}

static void _cpu_spi_trx_callback() {

    _cpu_spi_rx_enqueue(&g_cpu_spi_rx_byte);

    // Transmit 0 if tx queue empty.
    if (_cpu_spi_tx_dequeue(&g_cpu_spi_tx_byte)) {
        g_cpu_spi_tx_byte = 0;
    };

    // CPU leads, so always re-enable transfer to catch next byte.
    _cpu_spi_trx_byte(&g_cpu_spi_tx_byte, &g_cpu_spi_rx_byte);
}

/*----- End of file --------------------------------------------------*/
