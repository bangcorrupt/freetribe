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
 * @file    gui_window.h
 *
 * @brief   Public API for Freetribe GUI window management.
 *
 */

#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/*----- Macros and Definitions ---------------------------------------*/

typedef enum {
    GUI_WINDOW_MAIN,

    GUI_WINDOW_COUNT
} e_gui_window_index;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/
void gui_window_main_init(void);
void gui_window_main_show(void);

void gui_textbox_set_value(e_gui_window_index window_index,
                           uint8_t textbox_index, uint8_t value);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
