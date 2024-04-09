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
 * @file    keyboard.c
 *
 * @brief   Keyboard module.
 *
 * Map trigger pads to note numbers.
 *
 */

/// TODO: Add support for rotation, transposition, etc...

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "keyboard.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _gen_keymap(t_keyboard *kbd, t_scale *scale);

/*----- Extern function implementations ------------------------------*/

void scale_init(t_scale *scale, uint32_t notes, uint8_t tones) {

    scale->notes = notes;

    scale->tones = tones;

    /// TODO: Support mode rotation.
    scale->mode = 0;
}

void keyboard_init(t_keyboard *kbd, t_scale *scale) {

    kbd->octave = 3;
    kbd->split = 12;
    kbd->scale = scale;
    _gen_keymap(kbd, kbd->scale);
}

void keyboard_set_scale(t_keyboard *kbd, t_scale *scale) {

    kbd->scale = scale;
    _gen_keymap(kbd, kbd->scale);
}

void keyboard_set_octave(t_keyboard *kbd, uint8_t octave) {

    kbd->octave = octave;
}

void keyboard_set_split(t_keyboard *kbd, uint8_t split) { kbd->split = split; }

/// TODO: Pre-compute lookup tables.
uint8_t keyboard_map_note(t_keyboard *kbd, uint8_t pad) {

    uint8_t octave;
    uint8_t note;
    uint8_t degree;
    bool row;

    row = pad >= 8;

    if (row) {
        pad -= 8;
    }

    if (pad >= kbd->repeat) {
        degree = (pad % kbd->repeat);
    } else {
        degree = pad;
    }

    octave = kbd->octave + (pad / kbd->repeat);

    note = kbd->map[degree] + (kbd->scale->tones * octave);

    if (!row) {
        note += kbd->split;
    }

    return note;
}

/*----- Static function implementations ------------------------------*/

static void _gen_keymap(t_keyboard *kbd, t_scale *scale) {

    uint8_t i;
    uint8_t j = 0;

    for (i = 0; i < scale->tones; i++) {

        if (scale->notes & (1 << i)) {
            kbd->map[j] = i;
            j++;
        }
    }
    kbd->repeat = j;
}

/*----- End of file --------------------------------------------------*/
