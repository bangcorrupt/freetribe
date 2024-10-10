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
 * @file    dev_trs.c
 *
 * @brief   Device driver for TRS serial port.
 */

/// TODO: Support serial mode, not just MIDI.
///       Support buffer transmit/receive.

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "per_uart.h"

#include "ring_buffer.h"

/*----- Macros -------------------------------------------------------*/

/// TODO: Centralised header for interrupt priorities and queue sizes.

#define TRS_UART UART_1
#define TRS_UART_INT_CHANNEL 6

#define TRS_TX_BUF_LEN 0x200
#define TRS_RX_BUF_LEN 0x200

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

// TRS RX ring buffer.
static rbd_t trs_rx_rbd;
static char trs_rx_rbmem[TRS_RX_BUF_LEN];

// TRS TX ring buffer.
static rbd_t trs_tx_rbd;
static char trs_tx_rbmem[TRS_TX_BUF_LEN];

static uint8_t g_trs_rx_byte;

static bool g_trs_tx_complete = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static int _trs_tx_dequeue(uint8_t *byte);
static void _trs_rx_enqueue(uint8_t *byte);

static void _trs_tx_byte(uint8_t *byte);
static void _trs_rx_byte(void);

static void _trs_tx_callback(void);
static void _trs_rx_callback(void);

/*----- Extern function implementations ------------------------------*/

/// TODO: Return status code.
//
void dev_trs_init(void) {

    static t_uart_config uart_cfg = {.instance = TRS_UART,
                                     .baud = 31250,
                                     .word_length = 8,
                                     .int_enable = true,
                                     .int_channel = TRS_UART_INT_CHANNEL,
                                     .fifo_enable = true,
                                     .oversample = OVERSAMPLE_16};
    // Tx ring buffer attributes.
    rb_attr_t tx_attr = {sizeof(trs_tx_rbmem[0]), ARRAY_SIZE(trs_tx_rbmem),
                         trs_tx_rbmem};

    // Rx ring buffer attributes.
    rb_attr_t rx_attr = {sizeof(trs_rx_rbmem[0]), ARRAY_SIZE(trs_rx_rbmem),
                         trs_rx_rbmem};

    // Initialise ring buffers.
    if (ring_buffer_init(&trs_tx_rbd, &tx_attr) ||
        ring_buffer_init(&trs_rx_rbd, &rx_attr) == 0) {

        // Initialise UART for MIDI.
        per_uart_init(&uart_cfg);

        // Register Tx callback.
        per_uart_register_callback(TRS_UART, UART_TX_COMPLETE,
                                   _trs_tx_callback);

        // Register Rx callback.
        per_uart_register_callback(TRS_UART, UART_RX_COMPLETE,
                                   _trs_rx_callback);

        g_trs_tx_complete = true;

        // Enable Rx callback.
        _trs_rx_byte();
    }
}

void dev_trs_tx_enqueue(uint8_t *byte) {

    ring_buffer_put_force(trs_tx_rbd, byte);

    if (g_trs_tx_complete) {

        // Start transmission.
        _trs_tx_callback();
    }
}

int dev_trs_rx_dequeue(uint8_t *byte) {

    return ring_buffer_get(trs_rx_rbd, byte);
}

/*----- Static function implementations ------------------------------*/

static int _trs_tx_dequeue(uint8_t *byte) {

    return ring_buffer_get(trs_tx_rbd, byte);
}

static void _trs_rx_enqueue(uint8_t *byte) {

    // Overwrite on overflow.
    ring_buffer_put_force(trs_rx_rbd, byte);
}

static void _trs_tx_byte(uint8_t *byte) {

    g_trs_tx_complete = false;
    per_uart_transmit_int(TRS_UART, byte, 1);
}

static void _trs_rx_byte(void) {
    //
    per_uart_receive_int(TRS_UART, &g_trs_rx_byte, 1);
}

static void _trs_tx_callback(void) {
    //
    static uint8_t byte;

    // Send next queued byte.
    if (_trs_tx_dequeue(&byte) == 0) {

        _trs_tx_byte(&byte);

    } else {
        g_trs_tx_complete = true;
    }
}

static void _trs_rx_callback(void) {

    _trs_rx_enqueue(&g_trs_rx_byte);

    _trs_rx_byte();
}

/*----- End of file --------------------------------------------------*/
