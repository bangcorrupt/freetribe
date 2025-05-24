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
 * @file    zontar.c
 *
 * @brief   Controller for ZOIA.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "freetribe.h"

#include "keyboard.h"

// #include "svc_panel.h"

#include "gui_task.h"

#include "zoia_interface.h"
#include "zoia_task.h"

/*----- Macros -------------------------------------------------------*/

#define KNOB_LEVEL 0x00
#define KNOB_PITCH 0x02
#define KNOB_RES 0x03
#define KNOB_EG 0x04
#define KNOB_ATTACK 0x06
#define KNOB_DECAY 0x08
#define KNOB_MOD_DEPTH 0x05
#define KNOB_MOD_SPEED 0x0a

#define ENCODER_MAIN 0x00
#define ENCODER_OSC 0x01
#define ENCODER_CUTOFF 0x02
#define ENCODER_MOD 0x03
#define ENCODER_IFX 0x04

#define BUTTON_MENU 0x09
#define BUTTON_SHIFT 0x0a
#define BUTTON_AMP_EG 0x20
#define BUTTON_LPF 0x12
#define BUTTON_HPF 0x14
#define BUTTON_BPF 0x16

#define DEFAULT_SCALE_NOTES NOTES_PHRYGIAN_DOMINANT
#define DEFAULT_SCALE_TONES 12
#define DEFAULT_SCALE_MODE 0

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static t_keyboard g_kbd;
static t_scale g_scale;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _tick_callback(void);
static void _knob_callback(uint8_t index, uint8_t value);
static void _encoder_callback(uint8_t index, uint8_t value);
static void _button_callback(uint8_t index, bool state);
static void _trigger_callback(uint8_t pad, uint8_t vel, bool state);
static void _xy_pad_callback(uint32_t x_val, uint32_t y_val);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application.
 *
 * @return status   Status code indicating success:
 *                  - SUCCESS
 *                  - WARNING
 *                  - ERROR
 */
t_status app_init(void) {

    t_status status = ERROR;

    // scale_init(&g_scale, DEFAULT_SCALE_NOTES, DEFAULT_SCALE_TONES);
    // keyboard_init(&g_kbd, &g_scale);

    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    ft_register_panel_callback(ENCODER_EVENT, _encoder_callback);
    ft_register_panel_callback(BUTTON_EVENT, _button_callback);
    ft_register_panel_callback(TRIGGER_EVENT, _trigger_callback);
    ft_register_panel_callback(XY_PAD_EVENT, _xy_pad_callback);

    ft_register_tick_callback(0, _tick_callback);

    // Initialise GUI.
    gui_task();

    ft_print("ZONTAR");
    gui_print(4, 7, "ZONTAR");

    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 */
void app_run(void) {

    zoia_task();
    gui_task();
}

/*----- Static function implementations ------------------------------*/

static void _tick_callback(void) {
    //
}

/**
 * @brief   Callback triggered by panel knob events.
 *
 * @param[in]   index   Index of knob.
 * @param[in]   value   Value of knob.
 */
static void _knob_callback(uint8_t index, uint8_t value) {

    switch (index) {

    case KNOB_PITCH:
        // module_set_param(PARAM_TUNE, g_octave_tune_lut[value] / 2.0);
        // gui_post_param("Pitch: ", value);
        break;

    case KNOB_ATTACK:
        break;

    case KNOB_DECAY:
        break;

    case KNOB_LEVEL:
        break;

    case KNOB_RES:
        break;

    case KNOB_EG:
        break;

    case KNOB_MOD_DEPTH:
        break;

    case KNOB_MOD_SPEED:
        break;

    default:
        break;
    }
}

/**
 * @brief   Callback triggered by panel encoder events.
 *
 * @param[in]   index   Index of encoder.
 * @param[in]   value   Value of encoder.
 */
static void _encoder_callback(uint8_t index, uint8_t value) {

    switch (index) {

    case ENCODER_MAIN:

        if (value == 0x01) {
            zoia_encoder(1);

        } else {
            zoia_encoder(-1);
        }
        break;

    case ENCODER_CUTOFF:

        if (value == 0x01) {
            //

        } else {
            //
        }
        // module_set_param(PARAM_FILTER_BASE_CUTOFF,
        // g_midi_pitch_cv_lut[cutoff]); gui_post_param("Cutoff: ", cutoff);

        break;

    case ENCODER_OSC:

        if (value == 0x01) {
            //
        } else {
            //
        }
        // module_set_param(PARAM_OSC_TYPE, (1.0 / OSC_TYPE_COUNT) * osc_type);
        // gui_post_param("Osc Type: ", osc_type);

        break;

    case ENCODER_MOD:

        if (value == 0x01) {
            //
        } else {
            //
        }
        break;

    default:
        break;
    }
}

static void _trigger_callback(uint8_t pad, uint8_t vel, bool state) {

    if (state) {
        //
    } else {
        //
    }
}

/**
 * @brief   Callback triggered by panel button events.
 *
 * @param[in]   index   Index of button.
 * @param[in]   state   State of button.
 */
static void _button_callback(uint8_t index, bool state) {

    switch (index) {

    case BUTTON_MENU:
        if (state) {
            //
        }
        break;

    case BUTTON_SHIFT:
        if (state) {
            //
        }
        break;

    case BUTTON_AMP_EG:
        if (state) {
            //
        }
        break;

    case BUTTON_LPF:
        if (state) {
            //
        }
        break;

    case BUTTON_BPF:
        if (state) {
            //
        }
        break;

    case BUTTON_HPF:
        if (state) {
            //
        }
        break;

    default:
        break;
    }
}

static void _xy_pad_callback(uint32_t x_val, uint32_t y_val) {

    //
}

/*----- End of file --------------------------------------------------*/
