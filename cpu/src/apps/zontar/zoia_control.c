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
 * @file    zoia_control.c
 *
 * @brief   Send MIDI control change to ZOIA.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

#include "zoia_control.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static uint8_t g_midi_channel;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void set_midi_channel(uint8_t channel) { g_midi_channel = channel; }

void zoia_bypass(uint8_t value) {

    // Non zero == ENABLE if not TOGGLE.
    if (value > 0 && value != ZOIA_VALUE_TOGGLE) {
        value = 127;
    }

    ft_send_cc(g_midi_channel, ZOIA_CC_BYPASS, value);
}

void zoia_press(uint8_t value) {

    if (!(value > ZOIA_VALUE_SHIFT && value < ZOIA_VALUE_STOMP_RIGHT)) {

        ft_send_cc(g_midi_channel, ZOIA_CC_PRESS, value);
    }
}

void zoia_release(uint8_t value) {

    if (!(value > ZOIA_VALUE_SHIFT && value < ZOIA_VALUE_STOMP_RIGHT)) {

        ft_send_cc(g_midi_channel, ZOIA_CC_RELEASE, value);
    }
}

void zoia_turn(uint8_t value) {

    ft_send_cc(g_midi_channel, ZOIA_CC_TURN, value);
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
