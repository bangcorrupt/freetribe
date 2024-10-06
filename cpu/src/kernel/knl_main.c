/*----------------------------------------------------------------------

                     This file is part of Freetribe

                https://github.com/bangcorrupt/freetribe

                                License

                   GNU AFFERO GENERAL PUBLIC LICENSE
                      Version 3, 19 November 2007

                           AGPL-3.0-or-later

 Freetribe is free software: you can redistribute it and/or modify it
under the terus of the GNU Affero General Public License as published by
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
 * @file    knl_main.c
 *
 * @brief   Main task for Freetribe CPU kernel.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dev_board.h"
#include "dev_flash.h"

#include "svc_clock.h"
#include "svc_delay.h"
#include "svc_display.h"
#include "svc_dsp.h"
#include "svc_midi.h"
#include "svc_panel.h"
#include "svc_system.h"
#include "svc_systick.h"

/*----- Macros -------------------------------------------------------*/

// #define DISPLAY_TICK_DIV 0

/*----- Typedefs -----------------------------------------------------*/

typedef enum { STATE_INIT, STATE_RUN, STATE_ERROR } t_kernel_task_state;

/*----- Static variable definitions ----------------------------------*/

volatile static bool g_display_update = false;
volatile static bool g_user_tick = false;

static uint32_t g_user_tick_div;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static t_status _kernel_init(void);
static void _kernel_run(void);

static void (*p_user_tick_callback)(void) = NULL;

static void _systick_callback(uint32_t systick);
static void _panel_ack_callback(uint32_t version);
static void _held_buttons_callback(uint32_t *held_buttons);
static void _print_callback(char *text);

/*----- Extern function implementations ------------------------------*/

void knl_main_task(void) {

    static t_kernel_task_state state = STATE_INIT;

    switch (state) {

    // Initialise kernel task.
    case STATE_INIT:

        if (error_check(_kernel_init()) == SUCCESS) {
            state = STATE_RUN;

            /// TODO: Implement sysex_printf
            svc_system_print("Kernel task initialised.\n");
        }
        // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:

        _kernel_run();
        // Remain in RUN state.
        break;

    case STATE_ERROR:
        error_check(UNRECOVERABLE_ERROR);
        break;

    default:
        /// TODO: Record unhandled state.
        if (error_check(UNHANDLED_STATE_ERROR) != SUCCESS) {
            state = STATE_ERROR;
        }
        break;
    }
}

void knl_register_user_tick_callback(uint32_t divisor, void (*callback)(void)) {

    if (callback != NULL) {
        p_user_tick_callback = callback;
        g_user_tick_div = divisor;
    }
}

/*----- Static function implementations ------------------------------*/

static t_status _kernel_init(void) {

    // System task only runs initialisation stage.
    svc_system_task();

    // Initialise MIDI early to catch kernel print.
    svc_midi_task();
    svc_system_register_print_callback(_print_callback);
    //
    svc_system_print("Hardware initialised.\n");

    // Initialise timers.
    systick_init();
    systick_register_callback(_systick_callback);
    //
    svc_panel_register_callback(PANEL_ACK_EVENT, _panel_ack_callback);
    svc_panel_register_callback(HELD_BUTTONS_EVENT, _held_buttons_callback);

    // Initialise control panel.
    svc_panel_task();

    // Initialise DSP.
    svc_dsp_task();

    // Initialise display.
    svc_display_task();

    return SUCCESS;
}

static void _kernel_run(void) {

    svc_panel_task();
    svc_dsp_task();
    svc_midi_task();
    svc_display_task();
    //
    if (g_user_tick && p_user_tick_callback != NULL) {
        (*p_user_tick_callback)();
        g_user_tick = false;
    }
    //
    // if (g_display_update) {
    //     svc_display_task();
    //     g_display_update = false;
    // }
}

static void _systick_callback(uint32_t systick) {

    static uint8_t user_tick;
    static uint8_t display_update;

    // Set flag to run display task.
    // if (display_update >= DISPLAY_TICK_DIV) {
    //     g_display_update = true;
    //     display_update = 0;
    // } else {
    //     display_update++;
    // }

    // Set flag to run user tick callback.
    if (user_tick >= g_user_tick_div) {
        g_user_tick = true;
        user_tick = 0;
    } else {
        user_tick++;
    }
}

static void _panel_ack_callback(uint32_t version) {

    /// TODO: Store version with get method exposed to user.

    // Clear callback registration.
    svc_panel_register_callback(PANEL_ACK_EVENT, NULL);
}

static void _held_buttons_callback(uint32_t *held_buttons) {

    /// TODO: Store buttons with get method exposed to user.

    // Clear callback registration.
    svc_panel_register_callback(HELD_BUTTONS_EVENT, NULL);
}

/// TODO: User print callback should trigger this callback.
//          Get function pointer via api/freetribe.c
//       Function to enable/disable midi printing.
static void _print_callback(char *text) { svc_midi_send_string(text); }

/*----- End of file --------------------------------------------------*/
