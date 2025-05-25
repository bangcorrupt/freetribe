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
 * @file    knl_syscalls.c
 *
 * @brief   Freetribe kernal system calls.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "svc_display.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef void (*t_syscall_put_pixel)(uint16_t pos_x, uint16_t pos_y, bool state);

/*----- Static variable definitions ----------------------------------*/

static void *knl_syscall_table[] = {&svc_display_put_pixel, NULL};

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void *knl_get_syscalls(void) {

    // Return pointer to syscall jump table.
    return &knl_syscall_table;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
