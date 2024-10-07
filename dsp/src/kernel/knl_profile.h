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
 * @file    knl_profile.h
 *
 * @brief   Public API for Freetribe blackfin profiling.
 */

#ifndef KNL_PROFILE_H
#define KNL_PROFILE_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <blackfin.h>
#include <builtins.h>

/*----- Macros -------------------------------------------------------*/

#define CYCLE_LOG_LENGTH (16)

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

__inline__ __attribute__((always_inline)) static int cycles() {

    volatile long int ret;

    __asm__ __volatile__("%0 = CYCLES;\n\t" : "=&d"(ret) : : "R1");

    return ret;
}

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
