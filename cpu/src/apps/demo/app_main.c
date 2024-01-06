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

#include <stdlib.h>
#include <string.h>

#include "freetribe.h"

#include "app_main.h"

#include "ugui.h"

/*----- Macros and Definitions ---------------------------------------*/

#define USER_TICK_DIV 0     // User tick for every systick (~1ms).
#define DEBUG_TICK_DIV 1000 // Debug tick per 1000 user ticks (~1s).

/*----- Static variable definitions ----------------------------------*/

volatile static bool g_toggle_led = false;

/*----- Extern variable definitions ----------------------------------*/

UG_GUI gui;

/*----- Static function prototypes -----------------------------------*/

static void _register_callbacks(void);

static void _user_tick_callback(void);
static void _knob_callback(uint8_t index, uint8_t value);
static void _note_on_callback(char, char, char);
static void _note_off_callback(char, char, char);

static void _ui_init(void);
static void _put_pixel(UG_S16 x, UG_S16 y, UG_COLOR c);
static void _ui_print(char *text);

static char *_build_string(uint8_t value);

/*----- Extern function implementations ------------------------------*/

t_status app_init(void) {

    _register_callbacks();
    _ui_init();

    return 0;
}

void app_run(void) {

    if (g_toggle_led) {
        ft_toggle_led(LED_TAP);

        g_toggle_led = false;
    }
}

/*----- Static function implementations ------------------------------*/

static void _register_callbacks() {

    ft_register_tick_callback(USER_TICK_DIV, _user_tick_callback);

    ft_register_midi_callback(EVT_CHAN_NOTE_ON, _note_on_callback);
    ft_register_midi_callback(EVT_CHAN_NOTE_OFF, _note_off_callback);
    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
}

static void _ui_init(void) {

    // Initialise uGUI
    UG_Init(&gui, _put_pixel, 128, 64);

    // Configure uGUI
    UG_FontSelect(&FONT_6X8);
    UG_ConsoleSetArea(0, 0, 127, 63);
    UG_ConsoleSetBackcolor(C_BLACK);
    UG_ConsoleSetForecolor(C_WHITE);
    UG_FillScreen(C_BLACK);
}

static void _user_tick_callback(void) {

    static uint16_t debug;

    if (debug >= DEBUG_TICK_DIV) {

        g_toggle_led = true;
        debug = 0;

    } else {
        debug++;
    }
}

static void _note_on_callback(char chan, char note, char vel) {
    ft_send_note_on(chan, note, vel);
}

static void _note_off_callback(char chan, char note, char vel) {
    ft_send_note_off(chan, note, vel);
}

void _knob_callback(uint8_t index, uint8_t value) {

    ft_send_cc(0, index, value >> 1);

    switch (index) {

    // Level knob.
    case 0:
        ft_set_module_param(0, 0, value << 23);
        ft_set_module_param(0, 1, value << 23);
        _ui_print(_build_string(value >> 1));
        break;
    }
}

// UGUI uses 16 bits per pixel, we are using 1 bpp.
static void _put_pixel(UG_S16 x, UG_S16 y, UG_COLOR c) {

    svc_display_put_pixel(x, y, !c);
}

static void _ui_print(char *text) {
    //
    UG_ConsolePutString(text);
}

static char *_build_string(uint8_t value) {

    char value_string[5];

    static char display_string[20];

    itoa(value, value_string, 10);

    strcpy(display_string, "Level ");

    strncat(display_string, value_string, 3);
    strncat(display_string, "\n", 1);

    return display_string;
}

/*----- End of file --------------------------------------------------*/
