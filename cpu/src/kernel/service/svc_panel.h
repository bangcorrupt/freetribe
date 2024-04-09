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
 * @file    svc_panel.h
 *
 * @brief   Public API for communicating with panel.
 *
 */

#ifndef SVC_PANEL_H
#define SVC_PANEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/*----- Macros and Definitions ---------------------------------------*/

#define LED_COUNT 0x58

typedef enum {
    BUTTON_EVENT = 0,
    ENCODER_EVENT = 1,
    KNOB_EVENT = 2,
    UNDEFINED_EVENT = 3,
    TRIGGER_EVENT = 4,
    XY_PAD_EVENT = 5,
    PANEL_ACK_EVENT,
    HELD_BUTTONS_EVENT // Only fires once HELD_BUTTONS_MSW received.
} t_panel_event;

typedef enum {
    LED_MAIN_ENC = 0x00,
    LED_OSC_ENC = 0x01,
    LED_FILTER_ENC = 0x02,
    LED_MOD_ENC = 0x03,
    LED_INSERT_ENC = 0x04,

    LED_BACK = 0x05,
    LED_FORWARD = 0x06,
    LED_MENU = 0x07,
    LED_EXIT = 0x08,
    LED_SHIFT = 0x0a,
    LED_WRITE = 0x0b,

    LED_LPF = 0x0c,
    LED_HPF = 0x0d,
    LED_BPF = 0x0e,
    LED_MFX_SEND = 0x0f,
    LED_AMP_EG = 0x10,
    LED_IFX = 0x11,

    LED_REC = 0x12,
    LED_STOP = 0x13,
    LED_PLAY = 0x09,  // Both the same blue LED?
    LED_PAUSE = 0x15, //
    LED_TAP = 0x14,

    LED_GATE_ARP = 0x1a,
    LED_TOUCH_SCALE = 0x1b,
    LED_MASTER_FX = 0x1c,
    LED_MFX_HOLD = 0x1d,

    LED_LEFT = 0x1e,
    LED_RIGHT = 0x1f,
    LED_PART_MUTE = 0x20,
    LED_PART_ERASE = 0x21,
    LED_TRIGGER = 0x22,
    LED_SEQUENCER = 0x23,
    LED_KEYBOARD = 0x24,
    LED_CHORD = 0x25,
    LED_STEP_JUMP = 0x26,
    LED_PATTERN_SET = 0x27,

    LED_BAR_0_RED = 0x28,
    LED_BAR_0_BLUE = 0x16,
    LED_BAR_1_RED = 0x29,
    LED_BAR_1_BLUE = 0x17,
    LED_BAR_2_RED = 0x2a,
    LED_BAR_2_BLUE = 0x18,
    LED_BAR_3_RED = 0x2b,
    LED_BAR_3_BLUE = 0x19,

    LED_PAD_0_RED = 0x2c,
    LED_PAD_0_BLUE = 0x2d,
    LED_PAD_1_RED = 0x2e,
    LED_PAD_1_BLUE = 0x2f,
    LED_PAD_2_RED = 0x30,
    LED_PAD_2_BLUE = 0x31,
    LED_PAD_3_RED = 0x32,
    LED_PAD_3_BLUE = 0x33,
    LED_PAD_4_RED = 0x34,
    LED_PAD_4_BLUE = 0x35,
    LED_PAD_5_RED = 0x36,
    LED_PAD_5_BLUE = 0x37,
    LED_PAD_6_RED = 0x38,
    LED_PAD_6_BLUE = 0x39,
    LED_PAD_7_RED = 0x3a,
    LED_PAD_7_BLUE = 0x3b,

    LED_PAD_8_RED = 0x3c,
    LED_PAD_8_BLUE = 0x3d,
    LED_PAD_9_RED = 0x3e,
    LED_PAD_9_BLUE = 0x3f,
    LED_PAD_10_RED = 0x40,
    LED_PAD_10_BLUE = 0x41,
    LED_PAD_11_RED = 0x42,
    LED_PAD_11_BLUE = 0x43,
    LED_PAD_12_RED = 0x44,
    LED_PAD_12_BLUE = 0x45,
    LED_PAD_13_RED = 0x46,
    LED_PAD_13_BLUE = 0x47,
    LED_PAD_14_RED = 0x48,
    LED_PAD_14_BLUE = 0x49,
    LED_PAD_15_RED = 0x4a,
    LED_PAD_15_BLUE = 0x4b,

    LED_RGB_0_RED = 0x52,
    LED_RGB_0_GREEN = 0x53,
    LED_RGB_0_BLUE = 0x54,
    LED_RGB_1_RED = 0x55,
    LED_RGB_1_GREEN = 0x56,
    LED_RGB_1_BLUE = 0x57,
    LED_RGB_2_RED = 0x4c,
    LED_RGB_2_GREEN = 0x4d,
    LED_RGB_2_BLUE = 0x4e,
    LED_RGB_3_RED = 0x4f,
    LED_RGB_3_GREEN = 0x50,
    LED_RGB_3_BLUE = 0x51

} t_led_index;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void svc_panel_task(void);
void svc_panel_enqueue(uint8_t *msg);
void svc_panel_register_callback(t_panel_event event, void *callback);
void svc_panel_set_trigger_mode(uint8_t mode);
void svc_panel_set_led(t_led_index led_index, uint8_t brightness);
void svc_panel_toggle_led(t_led_index led_index);
void svc_panel_request_buttons(void);
void svc_panel_calib_xy(uint32_t xcal, uint32_t ycal);

#ifdef __cplusplus
}
#endif
#endif /* SVC_PANEL_H */

/*----- End of file --------------------------------------------------*/
