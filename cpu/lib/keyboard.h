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
 * @file    keyboard.h
 *
 * @brief   Public API for keyboard module.
 *
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

/*----- Macros and Definitions ---------------------------------------*/

#define NOTES_IONIAN 2741
#define NOTES_AEOLIAN 1453
#define NOTES_PHRYGIAN_DOMINANT 1459
#define NOTES_MINOR_BLUES 1257

typedef struct {
    uint32_t notes;
    uint8_t tones;
    uint8_t mode;
} t_scale;

typedef struct {
    t_scale *scale;
    uint8_t octave;
    uint8_t split;
    uint8_t repeat;
    uint8_t map[32];
} t_keyboard;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void keyboard_init(t_keyboard *kbd, t_scale *scale);
void keyboard_set_scale(t_keyboard *kbd, t_scale *scale);
uint8_t keyboard_map_note(t_keyboard *kbd, uint8_t pad);
void scale_init(t_scale *scale, uint32_t notes, uint8_t tones);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
