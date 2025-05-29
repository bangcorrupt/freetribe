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
 * @file    syscalls.c
 *
 * @brief   Implementations of stub Newlib stub functions.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void *_sbrk(int incr) {

    extern uint8_t _heap_end; /* Defined by the linker */

    static uint8_t *heap_end;

    uint8_t *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_heap_end;
    }
    prev_heap_end = heap_end;

    /// TODO: Check for collision.
    //
    // if (heap_end + incr > stack_ptr) {
    //     // write(1, "Heap and stack collision\n", 25);
    //     // abort();
    // }

    heap_end += incr;
    return (void *)prev_heap_end;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
