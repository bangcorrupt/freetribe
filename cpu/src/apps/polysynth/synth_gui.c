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
 * @file    synth_gui.c
 *
 * @brief   GUI for synth example.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freetribe.h"

#include "gui_task.h"
#include "gui_window.h"

#include "synth_gui.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static e_gui_menu_index g_gui_menu_current;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

bool gui_menu_select(e_gui_menu_index menu_index) {

    bool result = false;

    // if (gui_menu_current() != menu_index) {

    switch (menu_index) {

    case GUI_MENU_MODULATION:
        gui_post("Modulation\n");
        result = true;
        break;

    default:
        break;
    }
    // } else {
    //     //
    // }
    return result;
}

e_gui_menu_index gui_menu_current(void) { return g_gui_menu_current; }

void gui_menu_next(e_gui_menu_index menu_index) {
    //
}

void gui_menu_previous(e_gui_menu_index menu_index) {
    //
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
