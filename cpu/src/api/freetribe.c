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
 * @file    freetribe.c
 *
 * @brief   Public API for Freetribe applications.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

// Tick API
//
void ft_register_tick_callback(uint32_t divisor, void (*callback)(void)) {

    knl_register_user_tick_callback(divisor, callback);
}

// Print API
//
void ft_register_print_callback(void (*callback)(char *)) {

    svc_system_register_print_callback(callback);
}

void ft_print(char *text) { svc_midi_send_string(text); }

void ft_register_panel_callback(t_panel_event event, void (*callback)()) {

    svc_panel_register_callback(event, callback);
}

// MIDI API
//
void ft_register_midi_callback(event_type event,
                               midi_event_callback_t callback) {

    midi_register_event_handler(event, callback);
}

void ft_send_note_on(char chan, char note, char vel) {

    svc_midi_send_note_on(chan, note, vel);
}

void ft_send_note_off(char chan, char note, char vel) {

    svc_midi_send_note_off(chan, note, vel);
}

void ft_send_cc(char chan, char index, char val) {

    svc_midi_send_cc(chan, index, val);
}

// LED API
//
void ft_toggle_led(t_led_index led_index) { svc_panel_toggle_led(led_index); }

// DSP Command API
//
void ft_set_module_param(uint16_t module_id, uint16_t param_index,
                         int32_t param_value) {

    svc_dsp_set_module_param(module_id, param_index, param_value);
}

void ft_get_module_param(uint8_t module_id, uint16_t param_index) {

    svc_dsp_get_module_param(module_id, param_index);
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
