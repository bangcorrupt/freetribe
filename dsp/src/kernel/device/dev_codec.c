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
 * @file    dev_codec.c
 *
 * @brief   Audio codec device driver.
 */

/// TODO: Rework per_sport to use DMA linked descriptors
///       before building this on top of it.

/*----- Includes -----------------------------------------------------*/

// #include <stdbool.h>
//
// #include <blackfin.h>
// #include <builtins.h>

#include "types.h"
#include <stdint.h>

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void dev_codec_init(void) {
    //
}

void dev_codec_transmit(fract32 *output) {
    //
}

int32_t dev_codec_receive(fract32 *input) {
    //

    return 0;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
