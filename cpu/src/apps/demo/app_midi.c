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

/*
 * @file    app_midi.c
 *
 * @brief   Application MIDI handling.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "freetribe.h"

#include "midi_fsm.h"

#include "app_midi.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _note_on_callback(char, char, char);
static void _note_off_callback(char, char, char);
static void _prog_chg_callback(char, char, char);

/*----- Extern function implementations ------------------------------*/

void register_midi_callbacks(void) {
    midi_register_event_handler(EVT_CHAN_NOTE_ON, _note_on_callback);
    midi_register_event_handler(EVT_CHAN_NOTE_OFF, _note_off_callback);
    midi_register_event_handler(EVT_CHAN_PROGRAM_CHANGE, _prog_chg_callback);
}

/*----- Static function implementations ------------------------------*/

static void _note_on_callback(char chan, char note, char vel) {
    svc_midi_send_note_on(chan, note, vel);
}

static void _note_off_callback(char chan, char note, char vel) {
    svc_midi_send_note_off(chan, note, vel);
}

static void _prog_chg_callback(char chan, char note, char vel) {
    //
}

/*----- End of file --------------------------------------------------*/
