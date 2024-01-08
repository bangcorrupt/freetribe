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
 * @file    app_main.c
 *
 * @brief   Freetribe demo application.
 *
 * Demontsrates using the Freetribe API to handle events,
 * control the DSP and provide feedback.  Level knob controls
 * amplitude scaling of audio-passthrough while sending MIDI CC
 * and printing to the display.  Received MIDI note messages
 * are echoed to the output and a blinking LED is driven by the
 * user tick.  An external library is integrated for GUI management.
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

/**
 * @brief   Initialise application. Called once when app starts.
 *
 * Defined as a weak reference in the Freetribe API,
 * must be implemented by app developer.  This is a
 * good place to register callbacks and do any setup
 * required by external libraries.
 * In this example, we register callbacks for
 * MIDI and panel input, then initialise the uGUI
 * library.  Status is assumed successful.
 *
 * @return status   Status code indicating success:
 *                  - #SUCCESS
 *                  - #WARNING
 *                  - #ERROR
 * @include app_main.c
 */
t_status app_init(void) {

    t_status status = ERROR;

    _register_callbacks();
    _ui_init();

    status = SUCCESS;

    return status;
}

/**
 * @brief   Run application.  Called continuously in the main loop.
 *
 * Defined as a weak reference in the Freetribe API,
 * must be implemented by the app developer.
 * Must not block, as this would block the kernel.
 * Set flags in event handlers and test them here.
 * In this example, we test a flag set in the tick
 * callback and conditionally toggle an LED.
 */
void app_run(void) {

    if (g_toggle_led) {
        ft_toggle_led(LED_TAP);

        g_toggle_led = false;
    }
}

/*----- Static function implementations ------------------------------*/

/**
 * @brief   Register callbacks for MIDI and panel events.
 *
 * When the events occur, the callback functions will be executed.
 * In this example, we register callbacks for the user tick,
 * MIDI note on/off and panel knob input.
 */
static void _register_callbacks() {

    ft_register_tick_callback(USER_TICK_DIV, _user_tick_callback);

    ft_register_midi_callback(EVT_CHAN_NOTE_ON, _note_on_callback);
    ft_register_midi_callback(EVT_CHAN_NOTE_OFF, _note_off_callback);
    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
}

/**
 * @brief   Initialise the uGUI library.
 *
 * uGUI requires a function to print a pixel and
 * the dimensions of the display.  We also configure
 * the font and console before filling the screen black.
 */
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

/**
 * @brief   Callback triggered by user tick events.
 *
 * The Freetribe kernel systick ISR sets a flag to
 * trigger the kernel tick callback in the main loop.
 * The kernel tick callback triggers the user tick
 * callback at specified subdivisions.
 * In this example, we count the number of ticks
 * and when it reaches DEBUG_TICK_DIV we set a
 * flag to be tested in the app_run function.
 */
static void _user_tick_callback(void) {

    static uint16_t debug;

    if (debug >= DEBUG_TICK_DIV) {

        g_toggle_led = true;
        debug = 0;

    } else {
        debug++;
    }
}

/**
 * @brief   Callback triggered by MIDI note on events.
 *
 * Echo received note on messages to MIDI output.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   note    MIDI note number.
 * @param[in]   vel     MIDI note velocity.
 */
static void _note_on_callback(char chan, char note, char vel) {
    ft_send_note_on(chan, note, vel);
}

/**
 * @brief   Callback triggered by MIDI note off events.
 *
 * Echo received note off messages to MIDI output.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   note    MIDI note number.
 * @param[in]   vel     MIDI note velocity.
 */
static void _note_off_callback(char chan, char note, char vel) {
    ft_send_note_off(chan, note, vel);
}

/**
 * @brief   Callback triggered by panel knob events.
 *
 * Parse knob index and act on value.
 * Here we send a MIDI CC message on channel 0,
 * with CC number equal to knob index, for all knobs.
 * The panel MCU sends 8 bit unsigned values for
 * knobs, so we right shift by 1 for MIDI CC values.
 * For the Level knob only, we set parameters 0 and 1
 * in the DSP audio module.  The DSP expects 32 bit
 * signed values for module parameters, so we left
 * shift by 23.  This gives us most of the positive
 * range (0-0x7f800000) with an efficient operation.
 *
 * @param[in]   index   Index of knob.
 * @param[in]   value   Values of knob.
 */
/// TODO: We are doing too much in the callbacks.
/// Just copy values to local data structure,
/// then act in the main loop.
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

/**
 * @brief   Put pixel functon for uGUI library.
 *
 * User defined function required by uGUI.
 * Sets or clears a pixel at the specified coordinates.
 * uGUI uses 16 bits per pixel, Freetribe expects 1 bit per pixel.
 *
 * @param[in]   x   Horizontal position of pixel.
 * @param[in]   y   Vertical position of pixel.
 * @param[in]   c   Colour value, in this case 0 or 1.
 */
static void _put_pixel(UG_S16 x, UG_S16 y, UG_COLOR c) {

    svc_display_put_pixel(x, y, !c);
}

/**
 * @brief   Print a string to the uGUI console.
 *
 * @param[in]   text    String to print.
 */
static void _ui_print(char *text) {
    //
    UG_ConsolePutString(text);
}

/**
 * @brief   Utility function to build parameter string.
 *
 * Converts integer value of knob to string and
 * concatenates with parameter name.
 *
 * @note    Currently only supports 'Volume' parameter.
 *
 * @param[in]   value   Value of parameter.
 */
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
