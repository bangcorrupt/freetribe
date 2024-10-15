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
 * @file    svc_cpu.c
 *
 * @brief   Service for communicating with host CPU.
 */

// TODO: CPU device driver, command service.

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <string.h>

#include "ft_error.h"

#include "per_gpio.h"
#include "per_spi.h"

#include "module.h"

/*----- Macros -------------------------------------------------------*/

#define MSG_START 0xf0

/*----- Typedefs -----------------------------------------------------*/

typedef enum { STATE_INIT, STATE_RUN, STATE_ERROR } t_cpu_task_state;

typedef enum {
    PARSE_START,
    PARSE_MSG_TYPE,
    PARSE_MSG_ID,
    PARSE_PAYLOAD_LENGTH,
    PARSE_PAYLOAD
} t_msg_parse_state;

// TODO: Move protocol definition to common module.
//       Union struct for message.
enum e_message_type { MSG_TYPE_MODULE, MSG_TYPE_SYSTEM };

enum e_module_msg_id {
    MODULE_GET_PARAM_VALUE,
    MODULE_SET_PARAM_VALUE,
    MODULE_PARAM_VALUE,
    MODULE_GET_PARAM_NAME,
    MODULE_PARAM_NAME
};

enum e_system_msg_id {
    SYSTEM_CHECK_READY,
    SYSTEM_READY,
    SYSTEM_GET_PORT_STATE,
    SYSTEM_SET_PORT_STATE,
    SYSTEM_PORT_STATE
};

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _cpu_init(void);

static void _cpu_receive(uint8_t byte);

static void _transmit_message(uint8_t msg_type, uint8_t msg_id,
                              uint8_t *payload, uint8_t length);

static void _handle_message(uint8_t msg_type, uint8_t msg_id, uint8_t *payload,
                            uint8_t length);

static t_status _handle_module_message(uint8_t msg_id, uint8_t *payload,
                                       uint8_t length);

static t_status _handle_system_message(uint8_t msg_id, uint8_t *payload,
                                       uint8_t length);

static t_status _handle_module_get_param_value(uint8_t *payload,
                                               uint8_t length);

static t_status _handle_module_set_param_value(uint8_t *payload,
                                               uint8_t length);

static t_status _handle_module_get_param_name(uint8_t *payload, uint8_t length);

static t_status _handle_system_check_ready(void);

static t_status _handle_system_get_port_state(void);
static t_status _handle_system_set_port_state(uint8_t *payload, uint8_t length);

static t_status _respond_module_param_value(uint16_t module_id,
                                            uint16_t param_index,
                                            int32_t param_value);

static t_status _respond_module_param_name(uint16_t module_id,
                                           uint16_t param_index,
                                           char *param_name);

static t_status _respond_system_check_ready(void);
static t_status _respond_system_port_state(uint16_t port_f, uint16_t port_g,
                                           uint16_t port_h);

/*----- Extern function implementations ------------------------------*/

void svc_cpu_task(void) {

    static t_cpu_task_state state = STATE_INIT;

    static uint8_t cpu_byte;

    switch (state) {

    case STATE_INIT:
        if (_cpu_init() == SUCCESS) {
            state = STATE_RUN;
        }
        // Remain in INIT state until initialisation
        // successful.
        break;

        /// TODO: case STATE_HANDSHAKE:

    case STATE_RUN:

        // Handle received bytes.
        if (dev_cpu_spi_rx_dequeue(&cpu_byte) == SUCCESS) {
            _cpu_receive(cpu_byte);
        }
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

/*----- Static function implementations ------------------------------*/

static t_status _cpu_init(void) {

    t_status result = TASK_INIT_ERROR;

    // Initialise CPU SPI device driver.
    dev_cpu_spi_init();

    // _transmit_message(MSG_TYPE_SYSTEM, SYSTEM_READY, NULL, 0);

    /// TODO: Handhsake.

    result = SUCCESS;

    return result;
}

/// TODO: Return status.
static void _transmit_message(uint8_t msg_type, uint8_t msg_id,
                              uint8_t *payload, uint8_t length) {
    //
    uint8_t msg_start = MSG_START;

    dev_cpu_spi_tx_enqueue(&msg_start);
    dev_cpu_spi_tx_enqueue(&msg_type);
    dev_cpu_spi_tx_enqueue(&msg_id);
    dev_cpu_spi_tx_enqueue(&length);

    while (length--) {
        dev_cpu_spi_tx_enqueue(payload++);
    }
}

/// TODO: Move to separate module.
static void _cpu_receive(uint8_t byte) {

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
        // TODO: Error handling and reset protocol.
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
        }
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
    // Switch message type.
    switch (msg_type) {

    case MSG_TYPE_MODULE:
        _handle_module_message(msg_id, payload, length);
        break;

    case MSG_TYPE_SYSTEM:
        _handle_system_message(msg_id, payload, length);
        break;
    }
}

static t_status _handle_module_message(uint8_t msg_id, uint8_t *payload,
                                       uint8_t length) {

    uint8_t result = ERROR;

    switch (msg_id) {

    case MODULE_GET_PARAM_VALUE:
        result = _handle_module_get_param_value(payload, length);
        break;

    case MODULE_SET_PARAM_VALUE:
        result = _handle_module_set_param_value(payload, length);
        break;

    case MODULE_GET_PARAM_NAME:
        result = _handle_module_get_param_name(payload, length);
        break;

    default:
        result = ERROR;
        break;
    }

    return result;
}

static t_status _handle_system_message(uint8_t msg_id, uint8_t *payload,
                                       uint8_t length) {

    uint8_t result = ERROR;
    switch (msg_id) {

    case SYSTEM_CHECK_READY:
        result = _handle_system_check_ready();

    case SYSTEM_GET_PORT_STATE:
        result = _handle_system_get_port_state();
        break;

    case SYSTEM_SET_PORT_STATE:
        // result = _handle_system_set_port_state(payload, length);
        break;

    default:
        result = ERROR;
        break;
    }

    return result;
}

static t_status _handle_module_get_param_value(uint8_t *payload,
                                               uint8_t length) {

    /// TODO: Union struct for message parsing.
    uint16_t module_id = (payload[1] << 8) | payload[0];

    uint16_t param_index = (payload[3] << 8) | payload[2];

    /// TODO: Register callbacks for message handling?
    //
    // module_id not supported yet.
    int32_t param_value = module_get_param(param_index);

    _respond_module_param_value(module_id, param_index, param_value);

    /// TODO: Error handling and protocol reset.
    return SUCCESS;
}

static t_status _handle_module_set_param_value(uint8_t *payload,
                                               uint8_t length) {

    /// TODO: Union struct for message parsing.
    int16_t module_id = (payload[1] << 8) | payload[0];

    uint16_t param_index = (payload[3] << 8) | payload[2];

    /// TODO: Register callbacks for message handling?
    int32_t param_value = (payload[7] << 24) | (payload[6] << 16) |
                          (payload[5] << 8) | payload[4];

    // module_id not supported yet.
    module_set_param(param_index, param_value);

    /// TODO: Error handling and protocol reset.
    return SUCCESS;
}

static t_status _handle_module_get_param_name(uint8_t *payload,
                                              uint8_t length) {

    uint16_t module_id;
    uint16_t param_index;

    char param_name[MAX_PARAM_NAME_LENGTH];

    /// TODO: Union struct for message parsing.
    module_id = (payload[1] << 8) | payload[0];

    param_index = (payload[3] << 8) | payload[2];

    // module_id not supported yet.
    module_get_param_name(param_index, param_name);

    _respond_module_param_name(module_id, param_index, param_name);

    /// TODO: Error handling and protocol reset.
    return SUCCESS;
}

static t_status _handle_system_check_ready(void) {

    /// TODO: Should initiate from CPU, then reply.
    //
    /// _respond_system_check_ready();

    return SUCCESS;
}

static t_status _respond_system_check_ready(void) {

    _transmit_message(MSG_TYPE_SYSTEM, SYSTEM_READY, NULL, 0);

    return SUCCESS;
}

static t_status _handle_system_get_port_state(void) {

    uint16_t port_f = per_gpio_get_port(PORT_F);
    uint16_t port_g = per_gpio_get_port(PORT_G);
    uint16_t port_h = per_gpio_get_port(PORT_H);

    _respond_system_port_state(port_f, port_g, port_h);

    /// TODO: Error handling and protocol reset.
    return SUCCESS;
}

static t_status _respond_module_param_value(uint16_t module_id,
                                            uint16_t param_index,
                                            int32_t param_value) {

    uint8_t payload[] = {module_id & 0xff,   (module_id >> 8) & 0xff,
                         param_index & 0xff, (param_index >> 8) & 0xff,
                         param_value & 0xff, (param_value >> 8 & 0xff)};

    _transmit_message(MSG_TYPE_MODULE, MODULE_PARAM_VALUE, payload,
                      sizeof(payload));

    return SUCCESS;
}

static t_status _respond_module_param_name(uint16_t module_id,
                                           uint16_t param_index,
                                           char *param_name) {

    /// TODO: This is jank.

    uint32_t string_length = strlen(param_name);
    uint32_t payload_length = string_length + 5;

    uint8_t payload[payload_length];

    payload[0] = module_id & 0xff;
    payload[1] = (module_id >> 8) & 0xff;
    payload[2] = param_index & 0xff;
    payload[3] = (param_index >> 8) & 0xff;

    memcpy(payload + 4, param_name, string_length);

    // Ensure null termination.
    payload[payload_length - 1] = '\0';

    _transmit_message(MSG_TYPE_MODULE, MODULE_PARAM_NAME, payload,
                      sizeof(payload));

    return SUCCESS;
}

static t_status _respond_system_port_state(uint16_t port_f, uint16_t port_g,
                                           uint16_t port_h) {

    uint8_t payload[] = {port_f & 0xff, (port_f >> 8) & 0xff,
                         port_g & 0xff, (port_g >> 8) & 0xff,
                         port_h & 0xff, (port_h >> 8) & 0xff};

    _transmit_message(MSG_TYPE_SYSTEM, SYSTEM_PORT_STATE, payload,
                      sizeof(payload));

    return SUCCESS;
}

/*----- End of file --------------------------------------------------*/
