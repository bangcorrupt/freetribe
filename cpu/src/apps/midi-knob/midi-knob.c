
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

                       Copyright bangcorrupt 2024

----------------------------------------------------------------------*/

/**
 * @file    midi-knob.c
 *
 * @brief   Example application for Freetribe user input.
 *
 * This example uses knob and button callbacks
 * to send MIDI CC messages and toggle an LED.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

/*----- Macros and Definitions ---------------------------------------*/

#define BUTTON_PLAY 0x2

/*----- Static variable definitions ----------------------------------*/

static bool g_toggle_led = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

void _knob_callback(uint8_t index, uint8_t value);
void _button_callback(uint8_t index, bool state);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application.
 *
 * Register callbacks for knob and button input.
 */
t_status app_init(void) {

    t_status status = ERROR;

    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    ft_register_panel_callback(BUTTON_EVENT, _button_callback);

    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 *
 * Test flag to conditionally toggle LED.
 */
void app_run(void) {

    if (g_toggle_led) {
        ft_toggle_led(LED_PLAY);

        g_toggle_led = false;
    }
}

/*----- Static function implementations ------------------------------*/

/**
 * @brief   Callback triggered by panel knob events.
 *
 * Send a MIDI CC message on channel 0,
 * with CC number equal to knob index, for all knobs.
 * The panel MCU sends 8 bit unsigned values for
 * knobs, so we right shift by 1 for MIDI CC values.
 */
void _knob_callback(uint8_t index, uint8_t value) {

    ft_send_cc(0, index, value >> 1);
}

/**
 * @brief   Callback triggered by panel button events.
 *
 *  parse the button index and test the button state.
 *  When the [Play] button is depressed we set a flag  to toggle it's LED.
 *  We ignore when the [Play] button is released.
 */
void _button_callback(uint8_t index, bool state) {

    switch (index) {

    case BUTTON_PLAY:
        if (state == 1) {
            g_toggle_led = true;
        }
        break;
    }
}

/*----- End of file --------------------------------------------------*/
