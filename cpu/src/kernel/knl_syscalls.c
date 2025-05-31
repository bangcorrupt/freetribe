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
 * @file    knl_syscalls.c
 *
 * @brief   Freetribe kernel system calls.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "knl_events.h"
#include "svc_delay.h"
#include "svc_display.h"
#include "svc_dsp.h"
#include "svc_midi.h"
#include "svc_panel.h"
#include "svc_system.h"

#include "midi_fsm.h"

#include "knl_main.h"
#include "knl_syscalls.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static int _print(void *p);
static int _put_pixel(void *p);
static int _fill_frame(void *p);
static int _set_led(void *p);
static int _init_delay(void *p);
static int _test_delay(void *p);
static int _register_callback(void *p);
static int _event_subscribe(void *p);
static int _send_midi_msg(void *p);
static int _set_module_param(void *p);
static int _get_module_param(void *p);
static int _set_trigger_mode(void *p);
static int _shutdown(void *p);

static t_syscall knl_syscall_table[] = {
    _print,
    _put_pixel,
    _fill_frame,
    _set_led,
    _init_delay,
    _test_delay,
    _register_callback,
    _event_subscribe,
    _send_midi_msg,
    _set_module_param,
    _get_module_param,
    _set_trigger_mode,
    _shutdown,
    NULL,
};

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

t_syscall *knl_get_syscalls(void) {

    // Return pointer to syscall jump table.
    return knl_syscall_table;
}

/*----- Static function implementations ------------------------------*/

static int _print(void *p) {

    char *text = p;

    svc_midi_send_string(text);

    return 0;
}

static int _put_pixel(void *p) {

    t_pixel *pixel = p;

    t_event event = {.id = KNL_EVENT_PUT_PIXEL,
                     .len = sizeof(t_pixel),
                     .data = (uint8_t *)pixel};

    return knl_event_publish(&event);
}

static int _fill_frame(void *p) {

    t_frame *frame = p;

    t_event event = {.id = KNL_EVENT_FILL_FRAME,
                     .len = sizeof(t_frame),
                     .data = (uint8_t *)frame};

    return knl_event_publish(&event);

    return 0;
}

static int _set_led(void *p) {

    t_led *led = p;

    svc_panel_set_led(led->index, led->value);

    return 0;
}

static int _init_delay(void *p) {

    t_delay *delay = p;

    delay_start(delay);

    return 0;
}

static int _test_delay(void *p) {

    t_delay *delay = p;

    delay_us(delay);

    return 0;
}

static int _register_callback(void *p) {

    t_callback *callback = p;

    switch (callback->id) {

    case CALLBACK_TICK:
        knl_register_user_tick_callback(callback->arg, callback->handler);
        break;

    case CALLBACK_PANEL:
        svc_panel_register_callback(callback->arg, callback->handler);
        break;

    case CALLBACK_MIDI:
        midi_register_event_handler(callback->arg, callback->handler);
        break;

    default:
        break;
    }

    return 0;
}

static int _event_subscribe(void *p) {

    struct params {
        e_event_id id;
        t_listener listener;
    };

    knl_event_subscribe(((struct params *)p)->id,
                        ((struct params *)p)->listener);

    return 0;
}

static int _send_midi_msg(void *p) {

    t_midi_msg *msg = p;

    uint8_t stat = msg->status & 0xf0;
    uint8_t chan = msg->status & 0x0f;

    switch (stat) {

    case MIDI_STATUS_NOTE_OFF:
        svc_midi_send_note_off(chan, msg->byte_1, msg->byte_2);
        break;

    case MIDI_STATUS_NOTE_ON:
        svc_midi_send_note_on(chan, msg->byte_1, msg->byte_2);
        break;

    case MIDI_STATUS_CC:
        svc_midi_send_cc(chan, msg->byte_1, msg->byte_2);
        break;

    default:
        break;
    }

    return 0;
}

static int _set_module_param(void *p) {

    t_param *param = p;

    svc_dsp_set_module_param(param->module_id, param->index, param->value);

    return 0;
}

static int _get_module_param(void *p) {

    t_param *param = p;

    svc_dsp_get_module_param(param->module_id, param->index);

    return 0;
}

static int _set_trigger_mode(void *p) {

    uint8_t *mode = p;

    svc_panel_set_trigger_mode(*mode);

    return 0;
}

static int _shutdown(void *p) {

    svc_system_shutdown();

    return 0;
}

/*----- End of file --------------------------------------------------*/
