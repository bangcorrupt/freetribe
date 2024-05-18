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
 * @file    gui_window.c
 *
 * @brief   Window management for Freetribe GUI.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gui_task.h"
#include "gui_window.h"

#include "ugui.h"
#include "ugui_button.h"
#include "ugui_colors.h"
#include "ugui_textbox.h"

/*----- Macros and Definitions ---------------------------------------*/

#define GUI_MAX_WINDOW_OBJECTS 32

#define WINDOW_MAIN_NUM_BUTTONS 4
#define WINDOW_MAIN_BUTTON_HEIGHT 10
#define WINDOW_MAIN_BUTTON_TEXTBOX_HEIGHT 9

/*----- Static variable definitions ----------------------------------*/

static UG_WINDOW g_gui_window[GUI_WINDOW_COUNT];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _window_main_callback(UG_MESSAGE *msg);

/*----- Extern function implementations ------------------------------*/

void gui_textbox_set_value(e_gui_window_index window_index,
                           uint8_t textbox_index, uint8_t value) {

    static char text[4] = {0};
    itoa(value, text, 10);

    UG_TextboxSetText(&g_gui_window[window_index], textbox_index, text);
}

void gui_window_main_init(void) {

    UG_WINDOW *main_window = &g_gui_window[GUI_WINDOW_MAIN];

    static UG_BUTTON button[WINDOW_MAIN_NUM_BUTTONS];
    static UG_TEXTBOX button_textbox[WINDOW_MAIN_NUM_BUTTONS];

    static UG_OBJECT main_window_obj_buf[GUI_MAX_WINDOW_OBJECTS];

    char *button_string[] = {"Atk", "Dec", "Sus", "Rel"};

    uint8_t screen_base;
    uint8_t button_width;
    uint8_t button_ys;
    uint8_t button_xs;
    uint8_t button_xe;

    uint8_t textbox_xs;
    uint8_t textbox_xe;
    uint8_t textbox_ys;
    uint8_t textbox_ye;

    uint8_t button_id;

    UG_WindowCreate(main_window, main_window_obj_buf, GUI_MAX_WINDOW_OBJECTS,
                    _window_main_callback);

    UG_WindowSetStyle(main_window, WND_STYLE_2D | WND_STYLE_HIDE_TITLE);
    // UG_WindowSetTitleHeight(main_window, 0);
    UG_WindowSetBackColor(main_window, C_BLACK);
    UG_WindowSetForeColor(main_window, C_WHITE);

    screen_base = main_window->ye;
    button_width = main_window->xe / WINDOW_MAIN_NUM_BUTTONS;
    button_ys = screen_base - WINDOW_MAIN_BUTTON_HEIGHT;

    textbox_ys = button_ys - WINDOW_MAIN_BUTTON_TEXTBOX_HEIGHT;
    textbox_ye = button_ys - 1;

    for (button_id = 0; button_id < WINDOW_MAIN_NUM_BUTTONS; button_id++) {

        button_xs = button_id * button_width + button_id;
        button_xe = button_width * (button_id + 1) + button_id;

        textbox_xs = button_xs + 3;
        textbox_xe = button_xe - 3;

        UG_ButtonCreate(main_window, &button[button_id], button_id, button_xs,
                        button_ys, button_xe, screen_base);

        UG_ButtonSetStyle(main_window, button_id,
                          BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);

        UG_ButtonSetText(main_window, button_id, button_string[button_id]);

        UG_TextboxCreate(main_window, &button_textbox[button_id], button_id,
                         textbox_xs, textbox_ys, textbox_xe, textbox_ye);

        UG_TextboxSetText(main_window, button_id, "000");
    }
}

void gui_window_main_show(void) {

    UG_WindowShow(&g_gui_window[GUI_WINDOW_MAIN]);

    gui_draw_line(0, 16, 127, 16, 1);
    gui_draw_line(0, 41, 127, 41, 1);
    gui_draw_line(31, 41, 31, 63, 1);
    gui_draw_line(63, 41, 63, 63, 1);
    gui_draw_line(95, 41, 95, 63, 1);
}

/*----- Static function implementations ------------------------------*/

static void _window_main_callback(UG_MESSAGE *msg) {
    //
}

/*----- End of file --------------------------------------------------*/
