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

#include "svc_event.h"

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

typedef void (*t_undefined_callback)(void);
typedef void (*t_xy_pad_callback)(uint32_t x_val, uint32_t y_val);

/*----- Static variable definitions ----------------------------------*/

static uint8_t g_led_current_brightness[LED_COUNT] = {0};

static t_undefined_callback p_undefined_callback = NULL;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _panel_init(void);
static t_status _panel_parse(uint8_t *msg);

static void _publish_data_rx_event(void);
static void _data_rx_listener(const t_event *event);

static t_status _publish_button_event(uint8_t index, bool state);
static t_status _publish_encoder_event(uint8_t index, int8_t value);
static t_status _publish_knob_event(uint8_t index, uint8_t value);
static t_status _publish_trigger_event(uint8_t index, uint8_t vel, bool state);
static t_status _publish_touchpad_event(uint16_t x, uint16_t y);
static t_status _publish_held_buttons_event(uint32_t *buttons);
static t_status _publish_ack_event(uint32_t version);

/*----- Extern function implementations ------------------------------*/

void svc_panel_task(void) {

    static t_panel_task_state state = STATE_INIT;

    switch (state) {

    case STATE_INIT:
        while (error_check(_panel_init()) != SUCCESS)
            ;

        state = STATE_RUN;
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:

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
/// FIX: DEPRECATED
///         Kernel should not keep track of LED state.
//
/// TODO: Move this to application library.
//
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

    /// TODO: Use ACK event listener to tell kernel MCU is ready.
    //
    // Block until MCU acknowledges.
    // System is not useful without MCU running.
    while (dev_mcu_rx_dequeue(panel_msg) != SUCCESS)
        ;

    _panel_parse(panel_msg);

    dev_mcu_register_callback(0, _publish_data_rx_event);
    svc_event_subscribe(SVC_EVENT_MCU_DATA_RX, _data_rx_listener);

    result = SUCCESS;

    return result;
}

static void _publish_data_rx_event(void) {

    t_event event = {
        .id = SVC_EVENT_MCU_DATA_RX,
        .len = 0,
        .data = NULL,
    };

    svc_event_publish(&event);
}

static void _data_rx_listener(const t_event *event) {

    uint8_t panel_msg[5] = {0};

    if (dev_mcu_rx_dequeue(panel_msg) == SUCCESS) {
        _panel_parse(panel_msg);
    }
}

static t_status _panel_parse(uint8_t *msg) {

    t_status result = PANEL_PARSE_ERROR;

    static uint32_t held_buttons[2];

    uint32_t version;

    switch (msg[0]) {

    case BUTTON_EVENT:
        _publish_button_event(msg[1], msg[2]);
        break;

    case ENCODER_EVENT:
        result = _publish_encoder_event(msg[1], msg[3]);
        break;

    case KNOB_EVENT:
        // In continuous mode, trigger pads use KNOB_EVENT message ID.
        if (msg[1] >= 0x11) {

            /// TODO: Modify pad index to match trigger mode?
            //
            result = _publish_trigger_event(msg[1], msg[3], (bool)msg[4]);

        } else {
            result = _publish_knob_event(msg[1], msg[3]);
        }
        break;

        /// TODO: Undefined message ID event.
    case UNDEFINED_EVENT:
        if (p_undefined_callback != NULL) {
            (p_undefined_callback)();
        }
        result = SUCCESS;
        break;

    case TRIGGER_EVENT:
        result = _publish_trigger_event(msg[1], msg[3], (bool)msg[4]);
        break;

    case XY_PAD_EVENT:
        result = _publish_touchpad_event((msg[1] << 8 | msg[2]),
                                         (msg[3] << 8 | msg[4]));
        break;

    case MSG_ID_ACK:

        /// TODO: Is this actually version number?
        ///         Think this is a checksum.
        //
        version = msg[1] << 0x18 | msg[2] << 0x10 | msg[3] << 0x8 | msg[4];

        result = _publish_ack_event(version);
        break;

    case MSG_ID_BUTTONS_LSW:
        held_buttons[0] =
            msg[1] << 0x18 | msg[2] << 0x10 | msg[3] << 0x8 | msg[4];
        // Event not published until MSW received.
        result = SUCCESS;
        break;

    case MSG_ID_BUTTONS_MSW:
        held_buttons[1] =
            msg[1] << 0x18 | msg[2] << 0x10 | msg[3] << 0x8 | msg[4];

        /// TODO: Clear state?
        // held_buttons[0] = 0;
        // held_buttons[1] = 0;

        result = _publish_held_buttons_event(held_buttons);
        break;

    default:
        /// TODO: Unknown message ID event.
        result = WARNING;
        break;
    }

    return result;
}

static t_status _publish_button_event(uint8_t index, bool state) {

    t_event event;
    t_button button;

    button.index = index;
    button.state = state;

    event.id = SVC_EVENT_PANEL_BUTTON;
    event.len = sizeof(button);
    event.data = (uint8_t *)&button;

    return svc_event_publish(&event);
}

static t_status _publish_encoder_event(uint8_t index, int8_t value) {

    t_event event;
    t_encoder encoder;

    encoder.index = index;
    encoder.value = value;

    event.id = SVC_EVENT_PANEL_ENCODER;
    event.len = sizeof(encoder);
    event.data = (uint8_t *)&encoder;

    return svc_event_publish(&event);
}

static t_status _publish_knob_event(uint8_t index, uint8_t value) {

    t_event event;
    t_knob knob;

    knob.index = index;
    knob.value = value;

    event.id = SVC_EVENT_PANEL_KNOB;
    event.len = sizeof(knob);
    event.data = (uint8_t *)&knob;

    return svc_event_publish(&event);
}

static t_status _publish_trigger_event(uint8_t index, uint8_t vel, bool state) {

    t_event event;
    t_trigger trigger;

    trigger.index = index;
    trigger.value = index;
    trigger.state = state;

    event.id = SVC_EVENT_PANEL_TRIGGER;
    event.len = sizeof(trigger);
    event.data = (uint8_t *)&trigger;

    return svc_event_publish(&event);
}

static t_status _publish_ack_event(uint32_t version) {

    t_event event;

    event.id = SVC_EVENT_PANEL_ACK;
    event.len = sizeof(version);
    event.data = (uint8_t *)&version;

    return svc_event_publish(&event);
}

static t_status _publish_held_buttons_event(uint32_t *buttons) {

    t_event event;

    event.id = SVC_EVENT_HELD_BUTTONS;
    event.len = 8;
    event.data = (uint8_t *)&buttons;

    return svc_event_publish(&event);
}

static t_status _publish_touchpad_event(uint16_t x, uint16_t y) {

    t_event event;

    t_touchpad touchpad = {.x = x, .y = y};

    event.id = SVC_EVENT_TOUCHPAD;
    event.len = sizeof(touchpad);
    event.data = (uint8_t *)&touchpad;

    return svc_event_publish(&event);
}

/*----- End of file --------------------------------------------------*/
