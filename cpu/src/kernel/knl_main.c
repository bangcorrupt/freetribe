/*----------------------------------------------------------------------

                     This file is part of Freetribe

                https://github.com/bangcorrupt/freetribe

                                License

                   GNU AFFERO GENERAL PUBLIC LICENSE
                      Version 3, 19 November 2007

                           AGPL-3.0-or-later

 Freetribe is free software: you can redistribute it and/or modify it
under the terus of the GNU Affero General Public License as published by
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
 * @file    knl_main.c
 *
 * @brief   Main task for Freetribe CPU kernel.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "midi_fsm.h"

#include "dev_mcu.h"
#include "dev_trs.h"

#include "svc_clock.h"
#include "svc_delay.h"
#include "svc_display.h"
#include "svc_dsp.h"
#include "svc_midi.h"
#include "svc_panel.h"
#include "svc_system.h"
#include "svc_systick.h"

#include "knl_events.h"
#include "knl_syscalls.h"

/*----- Macros -------------------------------------------------------*/

// #define DISPLAY_TICK_DIV 0

/*----- Typedefs -----------------------------------------------------*/

typedef enum { STATE_INIT, STATE_RUN, STATE_ERROR } t_kernel_task_state;

/*----- Static variable definitions ----------------------------------*/

volatile static bool g_user_tick = false;

static uint32_t g_user_tick_div;

static void (*p_user_tick_callback)(void) = NULL;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _kernel_init(void);
static void _kernel_run(void);

static void _systick_callback(uint32_t systick);

static void _trs_data_rx_callback(void);
static void _trs_data_rx_listener(const t_event *event);

static void _mcu_data_rx_callback(void);
static void _mcu_data_rx_listener(const t_event *event);

static void _midi_cc_rx_callback(char chan, char index, char value);
static void _midi_cc_rx_listener(const t_event *event);

static void _panel_ack_callback(uint32_t version);
static void _panel_ack_listener(const t_event *event);

static void _held_buttons_callback(uint32_t *held_buttons);
static void _held_buttons_listener(const t_event *event);

static void _button_callback(uint8_t button, bool state);

static void _put_pixel_listener(const t_event *event);
static void _fill_frame_listener(const t_event *event);

/*----- Extern function implementations ------------------------------*/

void knl_main_task(void) {

    static t_kernel_task_state state = STATE_INIT;

    switch (state) {

    // Initialise kernel task.
    case STATE_INIT:

        if (error_check(_kernel_init()) == SUCCESS) {
            state = STATE_RUN;

            /// TODO: Implement sysex_printf
            //
            svc_midi_send_string("Kernel task initialised.\n");
        }
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:

        _kernel_run();
        // Remain in RUN state.
        break;

    case STATE_ERROR:
        error_check(UNRECOVERABLE_ERROR);
        break;

    default:
        /// TODO: Record unhandled state.
        //
        if (error_check(UNHANDLED_STATE_ERROR) != SUCCESS) {
            state = STATE_ERROR;
        }
        break;
    }
}

void knl_register_user_tick_callback(uint32_t divisor, void (*callback)(void)) {

    if (callback != NULL) {
        p_user_tick_callback = callback;
        g_user_tick_div = divisor;
    }
}

/*----- Static function implementations ------------------------------*/

static t_status _kernel_init(void) {

    (*(t_get_syscalls *)PTR_GET_SYSCALLS) = knl_get_syscalls;

    // System task only runs initialisation stage.
    svc_system_task();

    // Initialise MIDI early to catch kernel print.
    svc_midi_task();

    svc_midi_send_string("Welcome to Freetribe!\n");

    svc_midi_send_string("Hardware initialised.\n");

    // Initialise timers.
    systick_init();
    systick_register_callback(_systick_callback);

    // Initialise control panel.
    svc_panel_task();

    // Initialise DSP.
    svc_dsp_task();

    // Initialise display.
    svc_display_task();

    knl_event_task();

    dev_trs_register_callback(0, _trs_data_rx_callback);
    knl_event_subscribe(KNL_EVENT_TRS_DATA_RX, _trs_data_rx_listener);

    midi_register_event_handler(EVT_CHAN_CONTROL_CHANGE, _midi_cc_rx_callback);
    knl_event_subscribe(KNL_EVENT_MIDI_CC_RX, _midi_cc_rx_listener);

    dev_mcu_register_callback(0, _mcu_data_rx_callback);
    knl_event_subscribe(KNL_EVENT_MCU_DATA_RX, _mcu_data_rx_listener);

    svc_panel_register_callback(PANEL_ACK_EVENT, _panel_ack_callback);
    knl_event_subscribe(KNL_EVENT_PANEL_ACK, _panel_ack_listener);

    svc_panel_register_callback(HELD_BUTTONS_EVENT, _held_buttons_callback);
    knl_event_subscribe(KNL_EVENT_HELD_BUTTONS, _held_buttons_listener);

    svc_panel_register_callback(BUTTON_EVENT, _button_callback);

    knl_event_subscribe(KNL_EVENT_PUT_PIXEL, _put_pixel_listener);
    knl_event_subscribe(KNL_EVENT_FILL_FRAME, _fill_frame_listener);

    return SUCCESS;
}

static void _kernel_run(void) {

    knl_event_task();

    // svc_panel_task();
    svc_dsp_task();
    // svc_midi_task();
    svc_display_task();

    if (g_user_tick && p_user_tick_callback != NULL) {
        (*p_user_tick_callback)();
        g_user_tick = false;
    }
}

static void _systick_callback(uint32_t systick) {

    static uint8_t user_tick;

    // Set flag to run user tick callback.
    if (user_tick >= g_user_tick_div) {
        g_user_tick = true;
        user_tick = 0;
    } else {
        user_tick++;
    }
}

static void _panel_ack_callback(uint32_t version) {

    t_event event = {
        .id = KNL_EVENT_PANEL_ACK,
        .len = sizeof(version),
        .data = (uint8_t *)version,
    };

    knl_event_publish(&event);
}

static void _panel_ack_listener(const t_event *event) {

    /// TODO: Store version with get method exposed to user.

    // uint32_t version = (uint32_t)event->data;

    // Clear callback registration.
    svc_panel_register_callback(PANEL_ACK_EVENT, NULL);
}

static void _held_buttons_callback(uint32_t *held_buttons) {

    t_event event = {
        .id = KNL_EVENT_HELD_BUTTONS,
        .len = 8,
        .data = (uint8_t *)held_buttons,
    };

    knl_event_publish(&event);
}

static void _held_buttons_listener(const t_event *event) {

    /// TODO: Store buttons with get method exposed to user.

    // uint32_t *buttons = (uint32_t *)event->data;

    // Clear callback registration.
    svc_panel_register_callback(HELD_BUTTONS_EVENT, NULL);
}

static void _trs_data_rx_callback(void) {

    t_event event = {
        .id = KNL_EVENT_TRS_DATA_RX,
        .len = 0,
        .data = NULL,
    };

    knl_event_publish(&event);
}

static void _trs_data_rx_listener(const t_event *event) {
    //
    svc_midi_task();
}

static void _mcu_data_rx_callback(void) {

    t_event event = {
        .id = KNL_EVENT_MCU_DATA_RX,
        .len = 0,
        .data = NULL,
    };

    knl_event_publish(&event);
}

static void _mcu_data_rx_listener(const t_event *event) {
    //
    svc_panel_task();
}

static void _midi_cc_rx_callback(char chan, char index, char value) {

    uint8_t msg[] = {chan, index, value};

    t_event event = {
        .id = KNL_EVENT_MIDI_CC_RX,
        .len = sizeof(msg),
        .data = msg,
    };

    knl_event_publish(&event);
}

static void _midi_cc_rx_listener(const t_event *event) {

    char snum[5];

    const uint8_t *msg = event->data;

    itoa(msg[1], snum, 16);
    svc_midi_send_string(snum);

    itoa(msg[2], snum, 16);
    svc_midi_send_string(snum);
}

static void _button_callback(uint8_t button, bool state) {

    uint8_t data[2] = {button, state};

    t_event event = {
        .id = KNL_EVENT_PANEL_BUTTON,
        .len = sizeof(data),
        .data = data,
    };

    knl_event_publish(&event);
}

static void _put_pixel_listener(const t_event *event) {

    t_pixel *pixel = (t_pixel *)event->data;

    svc_display_put_pixel(pixel->x, pixel->y, pixel->state);
}

static void _fill_frame_listener(const t_event *event) {

    t_frame *frame = (t_frame *)event->data;

    svc_display_fill_frame(frame->x_start, frame->y_start, frame->x_end,
                           frame->y_end, frame->state);
}

/*----- End of file --------------------------------------------------*/
