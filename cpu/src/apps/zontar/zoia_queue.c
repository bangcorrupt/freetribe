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
 * @file    zoia_queue.c
 *
 * @brief   Event queue for ZOIA controller.
 */

/*----- Includes -----------------------------------------------------*/

#include "ring_buffer.h"

#include "zoia_queue.h"

/*----- Macros -------------------------------------------------------*/

#define ZOIA_EVENT_QUEUE_LEN 0x20

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

// Event queue ring buffer.
static rbd_t g_zoia_rbd;
static char g_zoia_rbmem[ZOIA_EVENT_QUEUE_LEN][sizeof(t_zoia_event)];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

t_status zoia_queue_init(void) {

    t_status result = ERROR;

    // Event queue buffer attributes.
    rb_attr_t rb_attr = {sizeof(g_zoia_rbmem[0]), ARRAY_SIZE(g_zoia_rbmem),
                         g_zoia_rbmem};

    if (ring_buffer_init(&g_zoia_rbd, &rb_attr) == SUCCESS) {

        // Do any other initialisation...

        result = SUCCESS;
    }

    return result;
}

t_status zoia_enqueue(t_zoia_event *p_event) {

    // Returns 0 if SUCCESS.
    return ring_buffer_put(g_zoia_rbd, p_event);
}

t_status zoia_dequeue(t_zoia_event *p_event) {

    // Returns 0 if SUCCESS.
    return ring_buffer_get(g_zoia_rbd, p_event);
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
