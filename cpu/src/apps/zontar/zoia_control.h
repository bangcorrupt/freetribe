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
 * @file    zoia_control.h
 *
 * @brief   Publice API for ZOIA MIDI control change.
 *
 */

#ifndef ZOIA_CONTROL_H
#define ZOIA_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

/*----- Macros -------------------------------------------------------*/

#define ZOIA_CC_BYPASS 60
#define ZOIA_CC_PRESS 61
#define ZOIA_CC_RELEASE 62
#define ZOIA_CC_TURN 63

#define ZOIA_VALUE_TOGGLE 64

#define ZOIA_VALUE_BACK 42
#define ZOIA_VALUE_SHIFT 43
#define ZOIA_VALUE_STOMP_RIGHT 124
#define ZOIA_VALUE_ENCODER 127

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void zoia_bypass(uint8_t value);
void zoia_press(uint8_t value);
void zoia_release(uint8_t value);
void zoia_turn(uint8_t value);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
