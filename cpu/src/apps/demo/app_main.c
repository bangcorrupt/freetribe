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

/*
 * @file    app_main.c
 *
 * @brief   Freetribe demo application.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdio.h>

#include "freetribe.h"

#include "app_main.h"
#include "app_menu.h"
#include "app_midi.h"
#include "app_ui.h"

/*----- Macros and Definitions ---------------------------------------*/

#define USER_TICK_DIV 0     // User tick for every systick (~1ms).
#define DEBUG_TICK_DIV 1000 // Debug tick per 1000 user ticks (~1s).

/*----- Static variable definitions ----------------------------------*/

volatile static bool g_print_debug_msg = false;

static char g_debug_msg[] = "FREETRIBE\n";

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _user_tick_callback(void);
static void _register_callbacks(void);

/*----- Extern function implementations ------------------------------*/

t_status app_init(void) {

    _register_callbacks();
    ui_init();

    return 0;
}

void app_run(void) {

    static uint8_t i;

    if (g_print_debug_msg) {
        print(&g_debug_msg[i++]);

        if (i >= sizeof(g_debug_msg) - 1) {
            i = 0;
        }
        g_print_debug_msg = false;
    }
}

void print(char *text) {

    ft_print(text);
    ui_print(text);
}

/*----- Static function implementations ------------------------------*/

static void _register_callbacks() {

    svc_system_register_print_callback(print);
    knl_register_user_tick_callback(USER_TICK_DIV, _user_tick_callback);
    register_midi_callbacks();
    register_control_callbacks();
}

static void _user_tick_callback(void) {

    static uint16_t debug;

    if (debug >= DEBUG_TICK_DIV) {
        g_print_debug_msg = true;
        debug = 0;
    } else {
        debug++;
    }
}

/*----- End of file --------------------------------------------------*/
