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
 * @file    dev_mcu.c
 *
 * @brief   Device driver for communicating with panel board MCU.
 */

/// TODO: Support MCU flash modes.
///       Re-initialise with different ring buffers and callbacks.

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "per_uart.h"

#include "dev_mcu.h"

#include "ring_buffer.h"

/*----- Macros -------------------------------------------------------*/

/// TODO: Centralised header for interrupt priorities and queue sizes.

#define MCU_UART UART_0

#define MCU_UART_INT_CHANNEL 7

#define MCU_TX_BUF_LEN 0x200
#define MCU_RX_BUF_LEN 0x200
#define MCU_MSG_LEN 0x5

/*----- Typedefs -----------------------------------------------------*/

typedef void (*t_data_ready_callback)(void);

/*----- Static variable definitions ----------------------------------*/

// MCU RX ring buffer.
static rbd_t mcu_rx_rbd;
static uint8_t mcu_rx_rbmem[MCU_RX_BUF_LEN][MCU_MSG_LEN];

// MCU TX ring buffer.
static rbd_t mcu_tx_rbd;
static uint8_t mcu_tx_rbmem[MCU_TX_BUF_LEN][MCU_MSG_LEN];

static uint8_t g_mcu_rx_msg[MCU_MSG_LEN];

static bool g_mcu_tx_complete = true;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static int _mcu_tx_dequeue(uint8_t *mcu_msg);
static void _mcu_rx_enqueue(uint8_t *mcu_msg);

static void _mcu_tx_msg(uint8_t *mcu_msg);
static void _mcu_rx_msg(void);

static void _mcu_tx_callback(void);
static void _mcu_rx_callback(void);

static t_data_ready_callback p_data_ready_callback;

/*----- Extern function implementations ------------------------------*/

/// TODO: Return status code.
//
void dev_mcu_init(void) {

    static t_uart_config uart_cfg = {.instance = MCU_UART,
                                     .baud = 115200,
                                     .word_length = 8,
                                     .int_enable = true,
                                     .int_channel = MCU_UART_INT_CHANNEL,
                                     .fifo_enable = true,
                                     .oversample = OVERSAMPLE_13};
    // Tx ring buffer attributes.
    rb_attr_t tx_attr = {sizeof(mcu_tx_rbmem[0]), ARRAY_SIZE(mcu_tx_rbmem),
                         mcu_tx_rbmem};

    // Rx ring buffer attributes.
    rb_attr_t rx_attr = {sizeof(mcu_rx_rbmem[0]), ARRAY_SIZE(mcu_rx_rbmem),
                         mcu_rx_rbmem};

    // Initialise MCU message ring buffers.
    if (ring_buffer_init(&mcu_tx_rbd, &tx_attr) ||
        ring_buffer_init(&mcu_rx_rbd, &rx_attr) == 0) {

        // Initialise UART.
        per_uart_init(&uart_cfg);

        // Register Tx callback.
        per_uart_register_callback(MCU_UART, UART_TX_COMPLETE,
                                   _mcu_tx_callback);

        // Register Rx callback.
        per_uart_register_callback(MCU_UART, UART_RX_COMPLETE,
                                   _mcu_rx_callback);

        g_mcu_tx_complete = true;

        // Enable Rx callback.
        _mcu_rx_msg();
    }
}

t_status dev_mcu_tx_enqueue(uint8_t *mcu_msg) {

    t_status result = ERROR;

    // Overwrite on overflow.
    result = ring_buffer_put_force(mcu_tx_rbd, mcu_msg);

    if (g_mcu_tx_complete) {

        // Start transmission.
        _mcu_tx_callback();
    }

    return result;
}

int dev_mcu_rx_dequeue(uint8_t *mcu_msg) {

    return ring_buffer_get(mcu_rx_rbd, mcu_msg);
}

void dev_mcu_register_callback(uint8_t callback_id, void *callback) {

    /// TODO: Handle callback_id.
    //
    p_data_ready_callback = (t_data_ready_callback)callback;
}

/*----- Static function implementations ------------------------------*/

static int _mcu_tx_dequeue(uint8_t *mcu_msg) {

    return ring_buffer_get(mcu_tx_rbd, mcu_msg);
}

static void _mcu_rx_enqueue(uint8_t *mcu_msg) {

    // Overwrite on overflow.
    ring_buffer_put_force(mcu_rx_rbd, mcu_msg);

    if (p_data_ready_callback != NULL) {

        p_data_ready_callback();
    }
}

static void _mcu_tx_msg(uint8_t *mcu_msg) {

    g_mcu_tx_complete = false;
    per_uart_transmit_int(MCU_UART, mcu_msg, 5);
}

static void _mcu_rx_msg(void) {
    //
    per_uart_receive_int(MCU_UART, g_mcu_rx_msg, 5);
}

static void _mcu_tx_callback(void) {
    //
    static uint8_t mcu_msg[5];

    // Send next queued message.
    if (_mcu_tx_dequeue(mcu_msg) == 0) {
        _mcu_tx_msg(mcu_msg);

    } else {
        g_mcu_tx_complete = true;
    }
}

static void _mcu_rx_callback(void) {

    _mcu_rx_enqueue(g_mcu_rx_msg);

    _mcu_rx_msg();
}

/*----- End of file --------------------------------------------------*/
