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
 * @brief   Freetribe kernel system calls.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "service/svc_panel.h"
#include "svc_display.h"
#include "svc_midi.h"
#include "svc_system.h"

#include "knl_syscalls.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static int _put_pixel(void *p);
static int _print(void *p);
static int _set_led(void *p);
static int _init_delay(void *p);
static int _test_delay(void *p);
static int _shutdown(void *p);

static t_syscall knl_syscall_table[] = {
    _put_pixel, _print, _set_led, _init_delay, _test_delay, _shutdown, NULL,
};

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

t_syscall *knl_get_syscalls(void) {

    // Return pointer to syscall jump table.
    return knl_syscall_table;
}

/*----- Static function implementations ------------------------------*/

static int _put_pixel(void *p) {

    t_pixel *pixel = p;

    svc_display_put_pixel(pixel->x, pixel->y, pixel->state);

    return 0;
}

static int _print(void *p) {

    char *text = p;

    svc_midi_send_string(text);

    return 0;
}

static int _set_led(void *p) {

    t_led *led = p;

    svc_panel_set_led(led->index, led->value);

    return 0;
}

static int _init_delay(void *p) {

    t_delay *delay = p;

    delay_start(p);

    return 0;
}

static int _test_delay(void *p) {

    t_delay *delay = p;

    delay_us(p);

    return 0;
}

static int _shutdown(void *p) {

    svc_system_shutdown();

    return 0;
}

/*----- End of file --------------------------------------------------*/
