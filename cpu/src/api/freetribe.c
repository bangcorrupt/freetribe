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
 * @file    freetribe.c
 *
 * @brief   Freetribe application library.
 *
 * A set of wrappers for user applications to access kernel functions.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"
#include "svc_delay.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

// Tick API
//
/**
 * @brief   Register a callback for userspace tick events.
 *
 * The kernel systick triggers once per millisecond.
 * Set the divisor greater than 0 to trigger the user tick callback less often.
 *
 * @param[in]   divisor     Ratio of kernel ticks to user ticks.
 * @param[in]   callback    Function to call.
 */
void ft_register_tick_callback(uint32_t divisor, void (*callback)(void)) {

    knl_register_user_tick_callback(divisor, callback);
}

// Delay API
//
/**
 * @brief   Get the current cycle count from the delay timer.
 *
 * The delay timer runs continuously.
 * Use this API to get the current value to use as
 * start time before calling `ft_delay()`.
 *
 * @return  Current value of delay timer.
 */
uint32_t ft_get_delay_current(void) {

    /// TODO: Refactor delay functions.
    //
    return delay_get_current_count();
}

/**
 * @brief   Non-blocking delay from `start_time` for `delay_time`.
 *
 * Use `ft_get_delay_current()` to get the start time, then
 * use this API to test if `delay_time` microseconds have passed.
 *
 * @return  True if at least `delay_time` microseconds have passed since
 * `start_time`.
 */
bool ft_delay(t_delay_state *state) { return delay_us(state); }

// Display API
//
//
void ft_put_pixel(uint16_t pos_x, uint16_t pos_y, bool state) {

    svc_display_put_pixel(pos_x, pos_y, state);
}

int8_t ft_fill_frame(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                     uint16_t y_end, bool state) {
    return svc_display_fill_frame(x_start, y_start, x_end, y_end, state);
}

// Print API
//
/// TODO: What is going on with print?
//
void ft_register_print_callback(void (*callback)(char *)) {

    svc_system_register_print_callback(callback);
}

/**
 * @brief   Print ASCII string as MIDI system exclusive message.
 *
 * String will be encapsulated with 0xf7...0xf0
 * to ensure it passes through receiving MIDI drivers.
 *
 * @param[in]   text    String to be printed.
 *
 */
void ft_print(char *text) { svc_midi_send_string(text); }

// Panel API
//
/**
 * @brief   Register a callback for panel control input events.
 *
 * @param[in]   event       Type of event to catch.
 * @param[in]   callback    Function to call.
 *
 */
/// TODO: Separate function for each event type.
//
void ft_register_panel_callback(t_panel_event event, void *callback) {

    svc_panel_register_callback(event, callback);
}

void ft_set_trigger_mode(uint8_t mode) { svc_panel_set_trigger_mode(mode); }

// MIDI API
//
/**
 * @brief   Register a callback MIDI input events.
 *
 * @param[in]   event       Type of event to catch.
 * @param[in]   callback    Function to call.
 *
 */
/// TODO: Separate function for each event type.
//
void ft_register_midi_callback(event_type event,
                               t_midi_event_callback callback) {

    midi_register_event_handler(event, callback);
}

/**
 * @brief   Send MIDI note on message.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   note    MIDI note number.
 * @param[in]   vel     MIDI note velocity.
 *
 */
void ft_send_note_on(char chan, char note, char vel) {

    svc_midi_send_note_on(chan, note, vel);
}

/**
 * @brief   Send MIDI note off message.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   note    MIDI note number.
 * @param[in]   vel     MIDI note velocity.
 *
 */
void ft_send_note_off(char chan, char note, char vel) {

    svc_midi_send_note_off(chan, note, vel);
}

/**
 * @brief   Send MIDI control change message.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   index   MIDI CC number.
 * @param[in]   val     MIDI CC value.
 *
 */
void ft_send_cc(char chan, char index, char val) {

    svc_midi_send_cc(chan, index, val);
}

// LED API
//
/**
 * @brief   Toggle LED into the opposite state.
 *
 * @param[in]   led_index   Index of LED to toggle.
 *
 */
void ft_toggle_led(t_led_index led_index) { svc_panel_toggle_led(led_index); }

/**
 * @brief   Set LED.
 *
 * @param[in]   led_index   Index of LED to toggle.
 *
 */
void ft_set_led(t_led_index led_index, bool state) {
    svc_panel_set_led(led_index, state);
}

// DSP Command API
//
/**
 * @brief   Set parameter value in DSP audio module.
 *
 * @param[in]   module_id   Index of module to address.
 * @param[in]   param_index Index of parameter to set.
 * @param[in]   param_value Value of parameter.
 *
 */
void ft_set_module_param(uint16_t module_id, uint16_t param_index,
                         int32_t param_value) {

    svc_dsp_set_module_param(module_id, param_index, param_value);
}

/**
 * @brief   Request parameter value from DSP audio module.
 *
 * First register a callback for MODULE_PARAM_VALUE event,
 * then call this to request the data from the DSP.
 *
 * @param[in]   module_id   Index of module to address.
 * @param[in]   param_index Index of parameter to get.
 *
 */
void ft_get_module_param(uint8_t module_id, uint16_t param_index) {

    svc_dsp_get_module_param(module_id, param_index);
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
