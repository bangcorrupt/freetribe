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
 * @file    gui_task.h
 *
 * @brief   Public API for GUI task.
 */

#ifndef GUI_TASK_H
#define GUI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void gui_task(void);
void gui_print(uint8_t pos_x, uint8_t pos_y, char *text);
void gui_post(char *text);
void gui_post_param(char *label, uint8_t value);
void gui_print_int(uint8_t x_start, uint8_t y_start, uint8_t value);
void gui_draw_line(uint8_t x_start, uint8_t y_start, uint8_t x_end,
                   uint8_t y_end, bool colour);


void gui_show_mod_type(uint8_t mod_type) ;

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
