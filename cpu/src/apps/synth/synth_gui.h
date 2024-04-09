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
 * @file    synth_gui.h
 *
 * @brief   Public API for synth example GUI.
 *
 */

#ifndef SYNTH_GUI_H
#define SYNTH_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/*----- Macros and Definitions ---------------------------------------*/

typedef enum {
    GUI_MENU_MODULATION,

    GUI_MENU_COUNT
} e_gui_menu_index;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

bool gui_menu_select(e_gui_menu_index menu);
void gui_menu_next(e_gui_menu_index menu_index);
void gui_menu_previous(e_gui_menu_index menu_index);
e_gui_menu_index gui_menu_current(void);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
