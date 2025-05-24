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
 * @file    zoia-interface.c
 *
 * @brief   MIDI interface to ZOIA.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

#include "freetribe.h"

#include "zoia-interface.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _encoder_inc(uint32_t clicks);
static void _encoder_dec(uint32_t clicks);

/*----- Extern function implementations ------------------------------*/

void zoia_encoder(int32_t clicks) {

    if (clicks > 0) {
        _encoder_inc(clicks);

    } else if (clicks < 0) {
        _encoder_dec(abs(clicks));
    }
}

/// TODO: This triggers a bug in ZOIA.
//
void zoia_home(void) {

    int i;
    for (i = 0; i < 5; i++) {

        ft_send_cc(0, 61, 42);
        ft_send_cc(0, 62, 42);
    }
}

void zoia_patch_set(uint8_t patch_index) {

    patch_index = patch_index > 63 ? 63 : patch_index;

    zoia_home();

    zoia_encoder(-64);

    zoia_encoder(patch_index);
}

/*----- Static function implementations ------------------------------*/

static void _encoder_inc(uint32_t clicks) {

    while (clicks > 63) {

        ft_send_cc(0, 63, 127);

        clicks -= 63;
    }

    ft_send_cc(0, 63, 64 + clicks);
}

static void _encoder_dec(uint32_t clicks) {

    while (clicks > 63) {

        ft_send_cc(0, 63, 0);

        clicks -= 63;
    }

    ft_send_cc(0, 63, 64 - clicks);
}

/*----- End of file --------------------------------------------------*/
