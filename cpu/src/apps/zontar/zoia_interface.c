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
 * @file    zoia_interface.c
 *
 * @brief   MIDI interface to ZOIA.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

#include "freetribe.h"

#include "zoia_control.h"
#include "zoia_interface.h"
#include "zoia_queue.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static t_zoia_event g_event;

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

void zoia_enter(void) {

    g_event.value = ZOIA_VALUE_ENCODER;

    g_event.type = ZOIA_EVENT_PRESS;

    zoia_enqueue(&g_event);

    g_event.type = ZOIA_EVENT_RELEASE;

    zoia_enqueue(&g_event);
}

void zoia_home(void) {

    int i;
    for (i = 0; i < 5; i++) {
        zoia_back();
    }
}

void zoia_back(void) {

    g_event.value = ZOIA_VALUE_BACK;

    g_event.type = ZOIA_EVENT_PRESS;

    zoia_enqueue(&g_event);

    g_event.type = ZOIA_EVENT_RELEASE;

    zoia_enqueue(&g_event);
}

/// TODO: Use Program Change instead.
//
void zoia_patch_set(uint8_t patch_index) {

    patch_index = patch_index > 63 ? 63 : patch_index;

    zoia_home();

    zoia_encoder(-64);

    zoia_encoder(patch_index);
}

/*----- Static function implementations ------------------------------*/

static void _encoder_inc(uint32_t clicks) {

    g_event.type = ZOIA_EVENT_TURN;
    g_event.value = 127;

    while (clicks > 63) {

        zoia_enqueue(&g_event);

        clicks -= 63;
    }

    g_event.value = 64 + clicks;

    zoia_enqueue(&g_event);
}

static void _encoder_dec(uint32_t clicks) {

    g_event.type = ZOIA_EVENT_TURN;
    g_event.value = 0;

    while (clicks > 63) {

        zoia_enqueue(&g_event);

        clicks -= 63;
    }

    g_event.value = 64 - clicks;

    zoia_enqueue(&g_event);
}

/*----- End of file --------------------------------------------------*/
