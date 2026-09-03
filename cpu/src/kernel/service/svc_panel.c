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
 * @file    svc_panel.c
 *
 * @brief   Interface to panel controls and LED.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "dev_mcu.h"

#include "svc_panel.h"

#include "ft_error.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    STATE_INIT,
    STATE_RUN,
    STATE_ERROR,
} t_panel_task_state;

typedef enum {
    MSG_ID_CONTROL = 0x05,
    MSG_ID_ACK = 0x80,
    MSG_ID_BUTTONS_LSW = 0x91, // Low word of 64 bit held buttons mask.
    MSG_ID_BUTTONS_MSW = 0x92  // High word.
} t_panel_msg_id;

typedef void (*t_button_callback)(uint8_t button, bool state);
typedef void (*t_encoder_callback)(uint8_t enc, int8_t val);
typedef void (*t_knob_callback)(uint8_t knob, uint8_t val);
typedef void (*t_undefined_callback)(void);
typedef void (*t_trigger_callback)(uint8_t pad, uint8_t vel, bool state);
typedef void (*t_xy_pad_callback)(uint32_t x_val, uint32_t y_val);
typedef void (*t_panel_ack_callback)(uint32_t version);
typedef void (*t_held_buttons_callback)(uint32_t *held_buttons);

typedef struct __attribute__((packed)) {
    uint8_t id;
    uint8_t index;
    uint8_t reserved;
    uint8_t value;
    uint8_t state;
} t_panel_control_msg;

typedef struct __attribute__((packed)) {
    uint8_t id;
    uint8_t value_msb;
    uint8_t value_lsb;
    uint8_t reserved_msb;
    uint8_t reserved_lsb;
} t_panel_xy_msg;

typedef struct __attribute__((packed)) {
    uint8_t id;
    uint8_t value[4];
} t_panel_word_msg;

/*----- Static variable definitions ----------------------------------*/

static uint8_t g_led_current_brightness[LED_COUNT] = {0};

static t_button_callback p_button_callback = NULL;
static t_encoder_callback p_encoder_callback = NULL;
static t_knob_callback p_knob_callback = NULL;
static t_undefined_callback p_undefined_callback = NULL;
static t_trigger_callback p_trigger_callback = NULL;
static t_xy_pad_callback p_xy_pad_callback = NULL;
static t_panel_ack_callback p_panel_ack_callback = NULL;
static t_held_buttons_callback p_held_buttons_callback = NULL;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _panel_init(void);
static t_status _panel_parse(uint8_t *msg);
static uint32_t _read_u32_be(const uint8_t *value);

/*----- Extern function implementations ------------------------------*/

void svc_panel_task(void) {

    static t_panel_task_state state = STATE_INIT;
    static uint8_t panel_msg[5] = {0};

    switch (state) {

    case STATE_INIT:
        if (error_check(_panel_init()) == SUCCESS) {
            state = STATE_RUN;
        }
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:

        if (dev_mcu_rx_dequeue(panel_msg) == SUCCESS) {
            _panel_parse(panel_msg);
        }
        // No error if MCU message not available.
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

void svc_panel_register_callback(t_panel_event event, void *callback) {

    if (callback != NULL) {
        switch (event) {

        case BUTTON_EVENT:
            p_button_callback = (t_button_callback)callback;
            break;

        case ENCODER_EVENT:
            p_encoder_callback = (t_encoder_callback)callback;
            break;

        case KNOB_EVENT:
            p_knob_callback = (t_knob_callback)callback;
            break;

        case UNDEFINED_EVENT:
            p_undefined_callback = (t_undefined_callback)callback;
            break;

        case TRIGGER_EVENT:
            p_trigger_callback = (t_trigger_callback)callback;
            break;

        case XY_PAD_EVENT:
            p_xy_pad_callback = (t_xy_pad_callback)callback;
            break;

        case PANEL_ACK_EVENT:
            p_panel_ack_callback = (t_panel_ack_callback)callback;
            break;

        case HELD_BUTTONS_EVENT:
            p_held_buttons_callback = (t_held_buttons_callback)callback;
            break;

        default:
            break;
        }
    }
}

void svc_panel_request_buttons(void) {

    uint8_t msg[5] = {0};

    msg[0] = 0x91;

    dev_mcu_tx_enqueue(msg);
}

void svc_panel_calib_xy(uint32_t xcal, uint32_t ycal) {

    uint8_t msg[5];

    msg[0] = 0x85;
    msg[1] = (xcal >> 24) & 0xff;
    msg[2] = (xcal >> 16) & 0xff;
    msg[3] = (xcal >> 8) & 0xff;
    msg[4] = xcal & 0xff;

    dev_mcu_tx_enqueue(msg);

    msg[0] = 0x86;
    msg[1] = (ycal >> 24) & 0xff;
    msg[2] = (ycal >> 16) & 0xff;
    msg[3] = (ycal >> 8) & 0xff;
    msg[4] = ycal & 0xff;

    dev_mcu_tx_enqueue(msg);
}

void svc_panel_set_trigger_mode(uint8_t mode) {

    uint8_t msg[5] = {0};

    if (mode != 0) {
        mode = 1;
    }

    msg[0] = 0x84;
    msg[1] = mode;

    dev_mcu_tx_enqueue(msg);
}

/**
 * Set LED.
 */
void svc_panel_set_led(t_led_index led_index, uint8_t brightness) {

    uint8_t mcu_msg[5] = {0, 0, 0, 0, 0};

    mcu_msg[1] = led_index;
    mcu_msg[2] = brightness;

    dev_mcu_tx_enqueue(mcu_msg);
    g_led_current_brightness[led_index] = brightness;
}

/**
 * Toggle LED.
 */
/// TODO: Toggle to specific brightness.
void svc_panel_toggle_led(t_led_index led_index) {

    uint8_t mcu_msg[5] = {0, 0, 0, 0, 0};

    mcu_msg[1] = led_index;

    if (g_led_current_brightness[led_index] != 0) {
        mcu_msg[2] = 0x00;
    } else {
        mcu_msg[2] = 0xff;
    }

    dev_mcu_tx_enqueue(mcu_msg);
    g_led_current_brightness[led_index] = mcu_msg[2];
}

/*----- Static function implementations ------------------------------*/

static t_status _panel_init(void) {

    dev_mcu_init();

    t_status result = TASK_INIT_ERROR;

    uint8_t panel_msg[5] = {0};

    // Send initial message to MCU.
    panel_msg[0] = 0x80;

    dev_mcu_tx_enqueue(panel_msg);

    /// TODO: Non blocking / Timeout error?.
    //
    // Block until MCU acknowledges.
    // System is not useful without MCU running.
    while (dev_mcu_rx_dequeue(panel_msg) != SUCCESS)
        ;

    _panel_parse(panel_msg);

    result = SUCCESS;

    return result;
}

static t_status _panel_parse(uint8_t *msg) {

    t_status result = PANEL_PARSE_ERROR;
    static uint32_t held_buttons[2];
    t_panel_control_msg *control_msg = (t_panel_control_msg *)msg;
    t_panel_xy_msg *xy_msg = (t_panel_xy_msg *)msg;
    t_panel_word_msg *word_msg = (t_panel_word_msg *)msg;

    switch (msg[0]) {

    case BUTTON_EVENT:
        if (p_button_callback != NULL) {
            (p_button_callback)(control_msg->index, (bool)control_msg->reserved);
        }
        result = SUCCESS;
        break;

    case ENCODER_EVENT:
        if (p_encoder_callback != NULL) {
            (p_encoder_callback)(control_msg->index,
                                 (int8_t)(control_msg->value));
        }
        result = SUCCESS;
        break;

    case KNOB_EVENT:
        // In continuous mode, trigger pads use KNOB_EVENT message ID.
        if (control_msg->index >= 0x11) {
            if (p_trigger_callback != NULL) {
                (p_trigger_callback)(control_msg->index, control_msg->value,
                                     (bool)control_msg->state);
            }

        } else {
            if (p_knob_callback != NULL) {
                (p_knob_callback)(control_msg->index, control_msg->value);
            }
        }
        result = SUCCESS;
        break;

    case UNDEFINED_EVENT:
        if (p_undefined_callback != NULL) {
            (p_undefined_callback)();
        }
        result = SUCCESS;
        break;

    case TRIGGER_EVENT:
        if (p_trigger_callback != NULL) {
            (p_trigger_callback)(control_msg->index, control_msg->value,
                                 (bool)control_msg->state);
        }
        result = SUCCESS;
        break;

    case XY_PAD_EVENT:
        if (p_xy_pad_callback != NULL) {
            uint32_t x = (uint32_t)xy_msg->value_msb << 8 | xy_msg->value_lsb;
            uint32_t y = (uint32_t)xy_msg->reserved_msb << 8 |
                         xy_msg->reserved_lsb;
            (p_xy_pad_callback)(x, y);
        }
        result = SUCCESS;
        break;

    case MSG_ID_ACK:
        /// TODO: Is this actually version number?
        if (p_panel_ack_callback != NULL) {
            p_panel_ack_callback(_read_u32_be(word_msg->value));
        }
        result = SUCCESS;
        break;

    case MSG_ID_BUTTONS_LSW:
        held_buttons[0] = _read_u32_be(word_msg->value);
        // Callback not triggered until MSW received.
        result = SUCCESS;
        break;

    case MSG_ID_BUTTONS_MSW:
        held_buttons[1] = _read_u32_be(word_msg->value);

        if (p_held_buttons_callback != NULL) {
            p_held_buttons_callback(held_buttons);
        }

        /// TODO: Clear state?
        // held_buttons[0] = 0;
        // held_buttons[1] = 0;

        result = SUCCESS;
        break;

    default:
        /// TODO: Handle unknown msg_id.
        result = WARNING;
        break;
    }

    return result;
}

static uint32_t _read_u32_be(const uint8_t *value) {

    return ((uint32_t)value[0] << 24) | ((uint32_t)value[1] << 16) |
           ((uint32_t)value[2] << 8) | (uint32_t)value[3];
}

/*----- End of file --------------------------------------------------*/
