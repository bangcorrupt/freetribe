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

/// TODO: This should expose only those functions required by user applications.
///       For now we just include the service layer directly.
//
/**
 * @file    freetribe.h
 *
 * @brief   Public API for Freetribe application library.
 */

#ifndef FREETRIBE_H
#define FREETRIBE_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "ft_error.h"

#include "svc_clock.h"
#include "svc_delay.h"
#include "svc_display.h"
#include "svc_dsp.h"
#include "svc_midi.h"
#include "svc_panel.h"
#include "svc_sysex.h"
#include "svc_system.h"
#include "svc_systick.h"

#include "midi_fsm.h"

#include "svc_event.h"

#include "knl_main.h"
#include "usr_main.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void ft_init(void);

void ft_register_tick_callback(uint32_t divisor, void (*callback)(void));

bool ft_delay(t_delay *delay);
void ft_start_delay(t_delay *delay);

t_status ft_put_pixel(uint16_t pos_x, uint16_t pos_y, bool state);

int8_t ft_fill_frame(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                     uint16_t y_end, bool state);

void ft_register_print_callback(void (*callback)(char *));
void ft_print(char *text);

void ft_register_midi_callback(event_type event,
                               t_midi_event_callback callback);

void ft_event_subscribe(e_event_id id, t_listener listener);

void ft_send_note_on(char chan, char note, char vel);
void ft_send_note_off(char chan, char note, char vel);
void ft_send_cc(char chan, char index, char val);

/// TODO: Cast void pointers to typedef function pointers.
//
void ft_register_panel_callback(t_panel_event event, void *callback);
void ft_toggle_led(t_led_index led_index);
void ft_set_led(t_led_index led_index, uint8_t value);
void ft_set_trigger_mode(uint8_t mode);

void ft_set_module_param(uint16_t module_id, uint16_t param_index,
                         int32_t param_value);

void ft_get_module_param(uint8_t module_id, uint16_t param_index);

void ft_shutdown(void);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
