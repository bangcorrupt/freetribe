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

#include "gui_task.h"

#include "zoia_interface.h"
#include "zoia_task.h"

/*----- Macros -------------------------------------------------------*/

/// TODO: These should be in svc_panel.
//
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
#define BUTTON_EXIT 0x0d
#define BUTTON_AMP_EG 0x20
#define BUTTON_LPF 0x12
#define BUTTON_HPF 0x14
#define BUTTON_BPF 0x16

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static bool g_shift_held;

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

    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    ft_register_panel_callback(ENCODER_EVENT, _encoder_callback);
    ft_register_panel_callback(BUTTON_EVENT, _button_callback);
    ft_register_panel_callback(TRIGGER_EVENT, _trigger_callback);
    ft_register_panel_callback(XY_PAD_EVENT, _xy_pad_callback);

    ft_register_tick_callback(0, _tick_callback);

    // Initialise GUI.
    gui_task();

    // Initialise ZOIA control.
    zoia_task();

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
        break;

    case ENCODER_OSC:

        if (value == 0x01) {
            //
        } else {
            //
        }
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
            zoia_enter();
        }
        break;

    case BUTTON_EXIT:
        if (state) {

            if (g_shift_held) {
                zoia_home();

            } else {
                zoia_back();
            }
        }
        break;

    case BUTTON_SHIFT:
        g_shift_held = state;
        break;

    case BUTTON_AMP_EG:
        if (state) {
            //
        }
        break;

    case BUTTON_LPF:
        if (state) {
            ft_shutdown();
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
