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
 * @file    app_ui.c
 *
 * @brief   Application user interface.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "freetribe.h"

#include "app_menu.h"
#include "app_ui.h"

#include "ugui.h"

/*----- Macros and Definitions ---------------------------------------*/

#define ENTRYPOINT 0xc0000000;
/* #define ENTRYPOINT 0x80000000; */
/* #define ENTRYPOINT 0x80005e40; */

/*----- Static variable definitions ----------------------------------*/

static void (*entry)();

/*----- Extern variable definitions ----------------------------------*/

UG_GUI gui;

/*----- Static function prototypes -----------------------------------*/

// TODO: static
void xy_handler(uint32_t x, uint32_t y);
void knob_handler(uint8_t index, uint8_t value);
void button_handler(uint8_t idx, bool state);
void encoder_handler(uint8_t index, int8_t value);

/*----- Extern function implementations ------------------------------*/

// TODO: UGUI uses 16 bits per pixel.
//          we are using 1 bpp.
void put_pixel(UG_S16 x, UG_S16 y, UG_COLOR c) {

    svc_display_put_pixel(x, y, !c);
}

void ui_init(void) {

    menu_init();

    // Configure uGUI
    UG_Init(&gui, put_pixel, 128, 64);

    /* Draw text with uGUI */
    UG_FontSelect(&FONT_6X8);
    UG_ConsoleSetArea(0, 0, 127, 63);
    UG_ConsoleSetBackcolor(C_BLACK);
    UG_ConsoleSetForecolor(C_WHITE);
}

void ui_print(char *text) { UG_ConsolePutString(text); }

void register_control_callbacks(void) {

    // Register control callbacks.
    svc_panel_register_callback(XY_PAD_EVENT, xy_handler);
    svc_panel_register_callback(KNOB_EVENT, knob_handler);
    svc_panel_register_callback(BUTTON_EVENT, button_handler);
    svc_panel_register_callback(ENCODER_EVENT, encoder_handler);
}

void xy_handler(uint32_t x, uint32_t y) {

    svc_midi_send_cc(0, 102, x >> 2 & 0x7f);
    svc_midi_send_cc(0, 103, y >> 2 & 0x7f);
}

void knob_handler(uint8_t index, uint8_t value) {

    svc_midi_send_cc(0, index, value >> 1);

    switch (index) {

    // Level knob.
    case 0:
        svc_dsp_set_module_param(0, 0, value << 23);
        svc_dsp_set_module_param(0, 1, value << 23);
        break;
    }
}

void button_handler(uint8_t idx, bool state) {

    // TODO: Static global with get method.
    //          Struct holding state of all controls.
    //              Allows shift functions, etc..
    static bool shift_held = false;

    switch (idx) {

    case 0x9:
        if (state) {
            // [MENU] pressed
            svc_panel_toggle_led(LED_MENU);
            svc_panel_toggle_led(LED_MENU);
            menu_enter();
        }
        break;

    case 0xa:
        shift_held = state;
        break;

    case 0xd:
        if (state) {
            if (shift_held) {
                // boot_app();
            } else {
                menu_exit();
            }
        }
        break;

    default:
        break;
    }
}

void encoder_handler(uint8_t index, int8_t value) {

    switch (index) {
    case 0:
        if (value == 1) {
            menu_down();
        } else {
            menu_up();
        }
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    }
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
