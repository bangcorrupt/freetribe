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
 * @file    svc_midi.c
 *
 * @brief   Functions to transmit and receive MIDI messages.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "dev_trs.h"

#include "ft_error.h"
#include "svc_midi.h"

#include "midi_fsm.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef enum { STATE_INIT, STATE_RUN, STATE_ERROR } t_midi_task_state;

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _midi_init(void);
static void _midi_out(uint8_t midi_byte);

/*----- Extern function implementations ------------------------------*/

void svc_midi_task(void) {

    static t_midi_task_state state = STATE_INIT;

    uint8_t midi_byte = 0;

    switch (state) {

    // Initialise MIDI task.
    case STATE_INIT:
        if (error_check(_midi_init()) == SUCCESS) {
            state = STATE_RUN;
        }
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:
        // Fetch and parse single MIDI byte.
        if (dev_trs_rx_dequeue(&midi_byte) == SUCCESS) {
            midi_receive_byte(midi_byte);
        }
        // No error if MIDI message not available.
        break;

    case STATE_ERROR:
        error_check(UNRECOVERABLE_ERROR);
        break;

    default:
        // TODO: Record unhandled state.
        if (error_check(UNHANDLED_STATE_ERROR) != SUCCESS) {
            state = STATE_ERROR;
        }
        break;
    }
}

/// TODO: Abstract this to separate library.
///          Functions returning messages.
/// TODO: Running status, only send changes.
void svc_midi_send_note_on(char chan, char note, char vel) {
    _midi_out(0x90 | chan);
    _midi_out(note);
    _midi_out(vel);
}

void svc_midi_send_note_off(char chan, char note, char vel) {
    _midi_out(0x80 | chan);
    _midi_out(note);
    _midi_out(vel);
}

void svc_midi_send_cc(uint8_t chan, uint8_t idx, uint8_t val) {

    _midi_out(0xb0 | chan);
    _midi_out(idx);
    _midi_out(val);
}

void svc_midi_send_string(char *text) {

    _midi_out(0xf0);

    while (*text) {
        _midi_out(*text++);
    }

    _midi_out(0xf7);
}

/// TODO: Refactor svc_midi_send_sysex.
//
void sysex_response(uint8_t msg_id) {
    _midi_out(0xf0);

    _midi_out(msg_id);

    _midi_out(0xf7);
}

/// TODO: malloc.
//
// void svc_midi_send_sysex(t_sysex_msg *msg) {
//
//     _midi_out(0xf0);
//
//     _midi_tx_buffer((uint8_t *)msg->header, sizeof(*msg->header));
//     _midi_tx_buffer((uint8_t *)msg->args, sizeof(*msg->args));
//     _midi_tx_buffer((uint8_t *)msg->data, sizeof(*msg->data));
//
//     _midi_out(0xf7);
// }

void svc_midi_send_byte(uint8_t byte) { _midi_out(byte); }

/*----- Static function implementations ------------------------------*/

static t_status _midi_init(void) {

    t_status result = TASK_INIT_ERROR;

    if (error_check(midi_init_fsm()) == SUCCESS) {

        dev_trs_init();

        midi_register_sysex_handler((t_midi_sysex_callback)sysex_parse);

        // Do any other initialisation.
        result = SUCCESS;
    }

    return result;
}

static void _midi_tx_buffer(uint8_t *data, uint32_t length) {

    while (length--) {

        dev_trs_tx_enqueue(data++);
    }
}

static void _midi_out(uint8_t midi_byte) {

    /// TODO: Refactor to use returned error code.
    //
    dev_trs_tx_enqueue(&midi_byte);
}

/*----- End of file --------------------------------------------------*/
