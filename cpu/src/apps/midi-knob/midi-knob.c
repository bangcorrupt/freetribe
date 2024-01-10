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
 * @file    midi-knob.c
 *
 * @brief   Example application for Freetribe panel input and MIDI output.
 * 
 * Demonstrates how to receive input from the 
 * control panel and send MIDI CC messages.
 * Moving the knobs will send CC messages
 * and pressing the Play button will toggle it's LED.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application. 
 *
 * In this example, we register callbacks for panel 
 * knob and button input. Status is assumed successful.
 *
 * @return status   Status code indicating success:
 *                  - #SUCCESS
 *                  - #WARNING
 *                  - #ERROR
 */
t_status app_init(void) {

    t_status status = ERROR;

    /// TODO: Separate API function for each event type.
    //
    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    ft_register_panel_callback(BUTTON_EVENT, _button_callback);

    status = SUCCESS;

    return status;
}

/**
 * @brief   Run application.
 *
 * In this example, we test a flag set in the panel
 * button callback and conditionally toggle an LED.
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
 * For all knobs, Send a MIDI CC message on channel 0,
 * with CC number equal to knob index.
 * The panel MCU sends 8 bit unsigned values in 
 * the range 0...255 for knobs, so we right shift 
 * by 1 for MIDI CC values.
 *
 * @param[in]   index   Index of knob.
 * @param[in]   value   Values of knob.
 */
void _knob_callback(uint8_t index, uint8_t value) {

    ft_send_cc(0, index, value >> 1);
}

/**
 * @brief   Callback triggered by panel button events.
 *
 * Parse button index and act on value.
 * In this example, we set a flag each time
 * the Play button is depressed. We ignore
 * when the Play button is released. We  can 
 * test this flag in the main loop to toggle 
 * an LED each time the button is pressed.
 *
 * @param[in]   index   Index of button.
 * @param[in]   state   State of button.
 */
void _button_callback(uint8_t index, bool state) {

    switch (index) {

    /// TODO: Check index.
    ///       enum knob index.
    //
    // Play button.
    case 0xa:
	if (value == 1) {
	    g_toggle_led == true;
	}
        break;
    }
}
/*----- End of file --------------------------------------------------*/
