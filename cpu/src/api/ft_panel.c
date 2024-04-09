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
 * @file    ft_panel.c
 *
 * @brief   Control panel input module.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "ft_panel.h"

/*----- Macros and Definitions ---------------------------------------*/

typedef enum {
    PANEL_BUTTON,
    PANEL_ENCODER,
    PANEL_KNOB,
    PANEL_TRIGGER_KNOB,
    PANEL_XY_PAD,

    PANEL_CONTROL_COUNT
} e_panel_control;

typedef enum {
    KNOB_LEVEL,
    KNOB_PAN,
    KNOB_PITCH,
    KNOB_RES,
    KNOB_EG,
    KNOB_DEPTH,
    KNOB_ATTACK,
    KNOB_DECAY,
    KNOB_IFX,
    KNOB_EDIT,
    KNOB_SPEED,

    KNOB_COUNT
} e_knob;

// typedef struct {
//     uint8_t index;
//     uint8_t value;
// } t_knob;
//
// typedef struct {
//     uint8_t index;
//     int8_t value;
//     int32_t count;
// } t_encoder;
//
// typedef struct {
//     uint8_t index;
//     bool state;
// } t_button;
//
// typedef struct {
//     uint8_t index;
//     uint8_t value;
// } t_trigger;
//
// typedef struct {
//     uint16_t x;
//     uint16_t y;
//     bool z;
// } t_xy_pad;
//
// typedef struct {
//     t_knob knob[11];
//     t_encoder encoder[5];
//     t_button button[34];
//     t_trigger trigger[16];
//     t_xy_pad xy_pad;
// } t_panel;
//
/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
