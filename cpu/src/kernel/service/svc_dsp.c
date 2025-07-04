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
 * @file    svc_dsp.c
 *
 * @brief   Interface to Blackfin DSP.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "dev_dsp.h"

#include "ft_error.h"

#include "svc_delay.h"
#include "svc_dsp.h"

#include "ring_buffer.h"

#include "bfin_ldr.h"

/*----- Macros -------------------------------------------------------*/

#define MSG_START 0xf0

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    STATE_INIT,
    STATE_ASSERT_RESET,
    STATE_RELEASE_RESET,
    STATE_BOOT,
    STATE_CHECK_READY,
    STATE_WAIT_READY,
    STATE_RUN,
    STATE_ERROR
} t_dsp_task_state;

typedef enum {
    PARSE_START,
    PARSE_MSG_TYPE,
    PARSE_MSG_ID,
    PARSE_PAYLOAD_LENGTH,
    PARSE_PAYLOAD
} t_msg_parse_state;

/*----- Static variable definitions ----------------------------------*/

static uint32_t g_pending_response;

static bool g_dsp_ready = false;

typedef void (*t_module_param_value_callback)(uint16_t module_id,
                                              uint16_t param_index,
                                              int32_t param_value);

typedef void (*t_system_port_state_callback)(uint16_t port_f, uint16_t port_g,
                                             uint16_t port_h);

typedef void (*t_system_profile_callback)(uint32_t period, uint32_t cycles);

static t_module_param_value_callback p_module_param_value_callback;
static t_system_port_state_callback p_system_port_state_callback;
static t_system_profile_callback p_system_profile_callback;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _dsp_init(void);
static void _dsp_boot(void);
static void _dsp_receive(uint8_t byte);
static void _dsp_check_ready(void);

static void _dsp_response_required(void);
static void _dsp_response_received(void);

static void _transmit_message(uint8_t msg_type, uint8_t msg_id,
                              uint8_t *payload, uint8_t length);

static void _handle_message(uint8_t msg_type, uint8_t msg_id, uint8_t *payload,
                            uint8_t length);

static t_status _handle_module_message(uint8_t msg_id, uint8_t *payload,
                                       uint8_t length);

static t_status _handle_system_message(uint8_t msg_id, uint8_t *payload,
                                       uint8_t length);

static t_status _handle_module_param_value(uint8_t *payload, uint8_t length);
static t_status _handle_system_port_state(uint8_t *payload, uint8_t length);
static t_status _handle_system_ready(void);

static t_status _handle_system_profile(uint8_t *payload, uint8_t length);

void _register_module_callback(uint8_t msg_id, void *callback);
void _register_system_callback(uint8_t msg_id, void *callback);

/*----- Extern function implementations ------------------------------*/

void svc_dsp_task(void) {

    static t_dsp_task_state state = STATE_INIT;

    static t_delay_state reset_delay;

    static uint8_t dsp_byte;

    switch (state) {

    case STATE_INIT:
        if (error_check(_dsp_init()) == SUCCESS) {
            state = STATE_ASSERT_RESET;
        }
        // Remain in INIT state until initialisation
        // successful.
        break;

    case STATE_ASSERT_RESET:

        dev_dsp_reset(true);

        delay_start(&reset_delay, 2100);

        state = STATE_RELEASE_RESET;
        break;

    case STATE_RELEASE_RESET:

        // Hold in reset for 2.1 ms.
        if (delay_us(&reset_delay)) {

            dev_dsp_reset(false);

            delay_start(&reset_delay, 1000);

            state = STATE_BOOT;
        }
        break;

    case STATE_BOOT:
        // Wait 1 ms after reset released.
        if (delay_us(&reset_delay)) {

            _dsp_boot();
            state = STATE_RUN;
        }
        break;

    case STATE_RUN:
        // Handle received bytes.
        if (dev_dsp_spi_rx_dequeue(&dsp_byte) == SUCCESS) {
            _dsp_receive(dsp_byte);
        }

        else if (g_pending_response > 0) {

            /// TODO: Can we use GPIO to signal?
            //
            // Poll Blackfin for queued bytes.
            dev_dsp_spi_poll();
        }

        g_dsp_ready = true;
        break;

    case STATE_ERROR:
        error_check(UNRECOVERABLE_ERROR);
        break;

    default:
        /// TODO: Record unhandled state.
        if (error_check(UNHANDLED_STATE_ERROR) != SUCCESS) {
            state = STATE_ERROR;
        }
        break;
    }
}

void svc_dsp_register_callback(uint8_t msg_type, uint8_t msg_id,
                               void *callback) {

    switch (msg_type) {

    case MSG_TYPE_MODULE:

        _register_module_callback(msg_id, callback);
        break;

    case MSG_TYPE_SYSTEM:
        _register_system_callback(msg_id, callback);
        break;

    default:
        break;
    }
}

/// TODO: Move to separate module.
void svc_dsp_set_module_param(uint16_t module_id, uint16_t param_index,
                              int32_t param_value) {

    const uint8_t msg_type = MSG_TYPE_MODULE;
    const uint8_t msg_id = MODULE_SET_PARAM_VALUE;

    /// TODO: Union struct / static allocation?
    uint8_t payload[] = {
        (module_id & 0xff),         (module_id >> 8) & 0xff,
        (param_index & 0xff),       (param_index >> 8) & 0xff,
        (param_value & 0xff),       (param_value >> 8) & 0xff,
        (param_value >> 16) & 0xff, (param_value >> 24) & 0xff};

    _transmit_message(msg_type, msg_id, payload, sizeof(payload));
}

void svc_dsp_get_module_param(uint16_t module_id, uint16_t param_index) {

    const uint8_t msg_type = MSG_TYPE_MODULE;
    const uint8_t msg_id = MODULE_GET_PARAM_VALUE;

    uint8_t payload[] = {(module_id & 0xff), (module_id >> 8) & 0xff,
                         (param_index & 0xff), (param_index >> 8) & 0xff};

    _dsp_response_required();

    _transmit_message(msg_type, msg_id, payload, sizeof(payload));
}

/// TODO: svc_dsp_get_module_param_count
///       svc_dsp_get_module_param_name

// Request state of Port F, Port G, Port H GPIO.
void svc_dsp_get_port_state(void) {

    const uint8_t msg_type = MSG_TYPE_SYSTEM;
    const uint8_t msg_id = SYSTEM_GET_PORT_STATE;

    _dsp_response_required();

    _transmit_message(msg_type, msg_id, NULL, 0);
}

void svc_dsp_get_profile(void) {

    const uint8_t msg_type = MSG_TYPE_SYSTEM;
    const uint8_t msg_id = SYSTEM_GET_PROFILE;

    _dsp_response_required();

    _transmit_message(msg_type, msg_id, NULL, 0);
}

bool svc_dsp_ready(void) { return g_dsp_ready; }

/*----- Static function implementations ------------------------------*/

void _dsp_check_ready(void) {

    const uint8_t msg_type = MSG_TYPE_SYSTEM;
    const uint8_t msg_id = SYSTEM_CHECK_READY;

    _dsp_response_required();

    _transmit_message(msg_type, msg_id, NULL, 0);
}

void _register_module_callback(uint8_t msg_id, void *callback) {

    switch (msg_id) {

    case MODULE_PARAM_VALUE:
        p_module_param_value_callback = (t_module_param_value_callback)callback;
        break;

    default:
        break;
    }
}

void _register_system_callback(uint8_t msg_id, void *callback) {

    switch (msg_id) {

    case SYSTEM_PORT_STATE:
        p_system_port_state_callback = (t_system_port_state_callback)callback;
        break;

    case SYSTEM_PROFILE:
        p_system_profile_callback = (t_system_profile_callback)callback;
        break;

    default:
        break;
    }
}

/// TODO: Return status.
static void _transmit_message(uint8_t msg_type, uint8_t msg_id,
                              uint8_t *payload, uint8_t length) {
    //
    uint8_t msg_start = MSG_START;

    dev_dsp_spi_tx_enqueue(&msg_start);
    dev_dsp_spi_tx_enqueue(&msg_type);
    dev_dsp_spi_tx_enqueue(&msg_id);
    dev_dsp_spi_tx_enqueue(&length);

    while (length--) {
        dev_dsp_spi_tx_enqueue(payload++);
    }
}

static t_status _dsp_init(void) {

    t_status result = TASK_INIT_ERROR;

    dev_dsp_init();

    result = SUCCESS;

    return result;
}

static void _dsp_boot(void) {

    /// TODO: Use queue and interrupt?
    ///         SPI hardware ENA max timeout
    ///         is too short for DSP boot.
    ///         Requires polling.
    //
    // Transmit LDR to DSP.
    dev_dsp_spi_tx_boot(bfin_ldr, bfin_ldr_len);
}

/// TODO: Move to protocol separate module.
///          Maybe this should be userspace?
//
static void _dsp_receive(uint8_t byte) {

    static t_msg_parse_state state = PARSE_START;

    static uint8_t msg_type;
    static uint8_t msg_id;
    static uint8_t length;
    static uint8_t payload[0xff];
    static uint8_t count;

    switch (state) {

        /// TODO: Handshake.

    case PARSE_START:
        if (byte == MSG_START) {
            state = PARSE_MSG_TYPE;
        }
        /// TODO: Error handling and reset protocol.
        break;

    case PARSE_MSG_TYPE:
        msg_type = byte;
        state = PARSE_MSG_ID;
        break;

    case PARSE_MSG_ID:
        msg_id = byte;
        state = PARSE_PAYLOAD_LENGTH;
        break;

    case PARSE_PAYLOAD_LENGTH:
        length = byte;
        state = PARSE_PAYLOAD;
        break;

    case PARSE_PAYLOAD:
        if (count < length) {
            payload[count] = byte;
            count++;
        } // else {
        // Handle message before returning,
        // else it is not handled until first
        // byte of next message is received.
        if (count >= length) {
            count = 0;
            _handle_message(msg_type, msg_id, payload, length);
            state = PARSE_START;
        }
        break;

    default:
        break;
    }
}

static void _handle_message(uint8_t msg_type, uint8_t msg_id, uint8_t *payload,
                            uint8_t length) {

    switch (msg_type) {

    case MSG_TYPE_MODULE:
        _handle_module_message(msg_id, payload, length);
        break;

    case MSG_TYPE_SYSTEM:
        _handle_system_message(msg_id, payload, length);
        break;

    default:
        break;
    }
}

static t_status _handle_module_message(uint8_t msg_id, uint8_t *payload,
                                       uint8_t length) {

    t_status result = ERROR;

    switch (msg_id) {

    case MODULE_PARAM_VALUE:
        result = _handle_module_param_value(payload, length);
        break;

    default:
        break;
    }

    /// TODO: Error handling and protocol reset.
    if (result == SUCCESS) {
        _dsp_response_received();
    }

    return result;
}

static t_status _handle_system_message(uint8_t msg_id, uint8_t *payload,
                                       uint8_t length) {

    t_status result = ERROR;

    switch (msg_id) {

    case SYSTEM_READY:
        result = _handle_system_ready();
        break;

    case SYSTEM_PORT_STATE:
        result = _handle_system_port_state(payload, length);
        break;

    case SYSTEM_PROFILE:
        result = _handle_system_profile(payload, length);
        break;

    default:
        break;
    }

    /// TODO: Error handling and protocol reset.
    if (result == SUCCESS) {
        _dsp_response_received();
    }

    return result;
}

/// TODO: Test payload length.
static t_status _handle_module_param_value(uint8_t *payload, uint8_t length) {

    t_status result = ERROR;

    if (p_module_param_value_callback != NULL) {

        /// TODO: Union struct for message parsing.

        uint16_t module_id = (payload[1] << 8) | payload[0];

        uint16_t param_index = (payload[3] << 8) | payload[2];

        uint32_t param_value = (payload[7] << 24) | (payload[6] << 16) |
                               (payload[5] << 8) | payload[4];

        p_module_param_value_callback(module_id, param_index, param_value);

        result = SUCCESS;
    }

    return result;
}

static t_status _handle_system_ready(void) {

    // g_dsp_ready = true;

    return SUCCESS;
}

static t_status _handle_system_port_state(uint8_t *payload, uint8_t length) {

    if (p_system_port_state_callback != NULL) {

        uint16_t port_f = (payload[1] << 8) | payload[0];
        uint16_t port_g = (payload[3] << 8) | payload[2];
        uint16_t port_h = (payload[5] << 8) | payload[4];

        p_system_port_state_callback(port_f, port_g, port_h);
    }

    return SUCCESS;
}

static t_status _handle_system_profile(uint8_t *payload, uint8_t length) {

    if (p_system_profile_callback != NULL) {

        uint32_t period = (payload[3] << 24 | payload[2] << 16 |
                           payload[1] << 8 | payload[0]);

        uint32_t cycles = (payload[7] << 24 | payload[6] << 16 |
                           payload[5] << 8 | payload[4]);

        p_system_profile_callback(period, cycles);
    }

    return SUCCESS;
}

static void _dsp_response_required(void) { g_pending_response++; }

static void _dsp_response_received(void) {

    if (g_pending_response > 0) {
        g_pending_response--;
    }
}
