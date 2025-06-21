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
 * @file    knl_profile.c
 *
 * @brief   Performance profiling.
 */

/*----- Includes -----------------------------------------------------*/

#include "cdefBF52x_base.h"
#include "gcc.h"

#include <stdint.h>

#include "per_sport.h"

#include "knl_profile.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

uint64_t g_module_cycles = 0;

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

t_profile knl_profile_stats(void) {

    static t_profile stats;

    stats.period = (uint32_t)sport0_period();
    stats.cycles = (uint32_t)g_module_cycles;

    return stats;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
