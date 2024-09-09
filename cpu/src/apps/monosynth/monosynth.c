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
 * @file    monosynth.c
 *
 * @brief   Monophonic synth example.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

#include "freetribe.h"

#include "keyboard.h"

#include "param_scale.h"
#include "svc_panel.h"

#include "gui_task.h"

#include "leaf.h"

/*----- Macros and Definitions ---------------------------------------*/

#define CONTROL_RATE (1000)
#define MEMPOOL_SIZE (0x1000)
// #define MEMPOOL_SIZE (0x4000)

#define KNOB_LEVEL 0x00
#define KNOB_PITCH 0x02
#define KNOB_RES 0x03
#define KNOB_EG 0x04
#define KNOB_ATTACK 0x06
#define KNOB_DECAY 0x08
#define KNOB_MOD_DEPTH 0x05
#define KNOB_MOD_SPEED 0x0a

#define ENCODER_OSC 0x01
#define ENCODER_CUTOFF 0x02
#define ENCODER_MOD 0x03

#define BUTTON_MENU 0x09
#define BUTTON_SHIFT 0x0a
#define BUTTON_AMP_EG 0x20
#define BUTTON_LPF 0x12
#define BUTTON_HPF 0x14
#define BUTTON_BPF 0x16

#define DEFAULT_CUTOFF 0x7f // Index in pitch LUT.
#define DEFAULT_OSC_TYPE 2

#define DEFAULT_SCALE_NOTES NOTES_PHRYGIAN_DOMINANT
#define DEFAULT_SCALE_TONES 12
#define DEFAULT_SCALE_MODE 0

#define PARAM_MAX (PARAM_COUNT - 1)
#define FILTER_TYPE_MAX (FILTER_TYPE_COUNT - 1)
#define MOD_TYPE_MAX (MOD_TYPE_COUNT - 1)
#define OSC_TYPE_MAX (OSC_TYPE_COUNT - 1)

// #define DEFAULT_AMP_ENV_ATTACK SLEW_10MS
// #define DEFAULT_AMP_ENV_DECAY SLEW_1S
// #define DEFAULT_AMP_ENV_SUSTAIN FR32_MAX
//
// #define DEFAULT_FILTER_ENV_ATTACK SLEW_10MS
// #define DEFAULT_FILTER_ENV_DECAY SLEW_1S
// #define DEFAULT_FILTER_ENV_SUSTAIN FR32_MAX
//
// #define DEFAULT_AMP_LFO_DEPTH 0
// #define DEFAULT_AMP_LFO_SPEED FIX16_ONE
//
// #define DEFAULT_FILTER_LFO_DEPTH 0
// #define DEFAULT_FILTER_LFO_SPEED FIX16_ONE
//
// #define DEFAULT_PITCH_LFO_DEPTH 0
// #define DEFAULT_PITCH_LFO_SPEED FIX16_ONE

/// TODO: Move to common location.
typedef enum {
    PARAM_AMP,
    PARAM_FREQ,
    PARAM_PHASE,
    PARAM_GATE,
    PARAM_VEL,
    PARAM_AMP_LEVEL,
    PARAM_AMP_ENV_ATTACK,
    PARAM_AMP_ENV_DECAY,
    PARAM_AMP_ENV_SUSTAIN,
    PARAM_AMP_ENV_RELEASE,
    PARAM_AMP_ENV_DEPTH,
    PARAM_FILTER_ENV_DEPTH,
    PARAM_FILTER_ENV_ATTACK,
    PARAM_FILTER_ENV_DECAY,
    PARAM_FILTER_ENV_SUSTAIN,
    PARAM_FILTER_ENV_RELEASE,
    PARAM_PITCH_ENV_DEPTH,
    PARAM_PITCH_ENV_ATTACK,
    PARAM_PITCH_ENV_DECAY,
    PARAM_PITCH_ENV_SUSTAIN,
    PARAM_PITCH_ENV_RELEASE,
    PARAM_CUTOFF,
    PARAM_RES,
    PARAM_TUNE,
    PARAM_OSC_TYPE,
    PARAM_FILTER_TYPE,
    PARAM_AMP_LFO_DEPTH,
    PARAM_AMP_LFO_SPEED,
    PARAM_FILTER_LFO_DEPTH,
    PARAM_FILTER_LFO_SPEED,
    PARAM_PITCH_LFO_DEPTH,
    PARAM_PITCH_LFO_SPEED,

    PARAM_COUNT
} e_param;

typedef enum {
    OSC_TYPE_SINE,
    OSC_TYPE_TRI,
    OSC_TYPE_SAW,
    OSC_TYPE_SQUARE,

    OSC_TYPE_COUNT
} e_osc_type;

typedef enum {
    FILTER_TYPE_LPF,
    FILTER_TYPE_HPF,
    FILTER_TYPE_BPF,

    FILTER_TYPE_COUNT
} e_filter_type;

typedef enum {
    MOD_AMP_LFO,
    MOD_FILTER_LFO,
    MOD_PITCH_LFO,

    MOD_TYPE_COUNT
} e_mod_type;

typedef struct {

    int32_t next;
    int32_t last;
    bool changed;
} t_cv;

/*----- Static variable definitions ----------------------------------*/

static int32_t g_midi_hz_lut[128];
static int32_t g_octave_tune_lut[256];
static int32_t g_filter_res_lut[256];

static bool g_shift_held;
static bool g_menu_held;
static bool g_amp_eg;

static e_mod_type g_mod_type;

static t_keyboard g_kbd;
static t_scale g_scale;

static LEAF g_leaf;
static char g_mempool[MEMPOOL_SIZE];

static tADSRS g_amp_env;
static t_cv g_amp_cv;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _knob_callback(uint8_t index, uint8_t value);
static void _encoder_callback(uint8_t index, uint8_t value);
static void _button_callback(uint8_t index, bool state);
static void _trigger_callback(uint8_t pad, uint8_t vel, bool state);
static void _tick_callback(void);

static void _set_filter_type(uint8_t filter_type);
static void _set_mod_depth(uint32_t mod_depth);
static void _set_mod_speed(uint32_t mod_speed);

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

    int i;

    float hz;
    for (i = 0; i <= 127; i++) {
        // Initialise pitch to frequency lookup table.
        hz = freq_12tet_lut[i * 2];

        /// TODO: Oscillators in the Aleph DSP library
        ///       are producing the wrong frequencies.
        ///       It's probably user error, maybe
        ///       samplerate. As a workaround, we
        ///       scale the frequency here to compensate.
        ///       Should multiplication be fixed point?
        //
        hz *= 0.6827421407069484;

        // Convert to fix16,
        g_midi_hz_lut[i] = (int32_t)hz;
    }

    /// TODO: Should be log.
    //
    // Initialise pitch mod lookup table.
    float tune;
    for (i = 0; i <= 255; i++) {

        if (i <= 128) {
            // 0.5...1.
            tune = i / 256.0 + 0.5;

        } else {
            // >1...2.0
            tune = ((i - 128) / 127.0) + 1;
        }

        // Convert to fix16,
        tune *= (1 << 16);
        g_octave_tune_lut[i] = (int32_t)tune;
    }

    int32_t res;
    for (i = 0; i <= 255; i++) {

        res = 0x7fffffff - (i * (1 << 23));

        g_filter_res_lut[i] = res;
    }

    LEAF_init(&g_leaf, CONTROL_RATE, g_mempool, MEMPOOL_SIZE, NULL);

    tADSRS_init(&g_amp_env, 0, 1024, 8192, 1024, &g_leaf);

    scale_init(&g_scale, DEFAULT_SCALE_NOTES, DEFAULT_SCALE_TONES);
    keyboard_init(&g_kbd, &g_scale);

    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    ft_register_panel_callback(ENCODER_EVENT, _encoder_callback);
    ft_register_panel_callback(BUTTON_EVENT, _button_callback);
    ft_register_panel_callback(TRIGGER_EVENT, _trigger_callback);

    ft_register_tick_callback(0, _tick_callback);

    _set_filter_type(FILTER_TYPE_LPF);

    // Initialise GUI.
    gui_task();

    gui_print(4, 7, "MonoSynth Example");

    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 */
void app_run(void) { gui_task(); }

/*----- Static function implementations ------------------------------*/

static void _tick_callback(void) {

    float next = 0;

    /// TODO: Use a fixed point envelope generator, or convert properly.
    ///       We can probably use a much cheaper envelope generator,
    ///       as control rate is relatively low and parameters can be
    ///       interpolated at audio rate on the DSP side.
    //
    next = tADSRS_tick(&g_amp_env);
    g_amp_cv.next = ((int32_t)(next * 262143.0) >> 16) << 16;

    if (g_amp_cv.next != g_amp_cv.last) {
        g_amp_cv.last = g_amp_cv.next;
        g_amp_cv.changed = true;

    } else {
        g_amp_cv.changed = false;
    }

    // Only send parameters if they have changed.
    if (g_amp_cv.changed) {
        ft_set_module_param(0, PARAM_AMP, g_amp_cv.next);
    }
}

/**
 * @brief   Callback triggered by panel knob events.
 *
 * @param[in]   index   Index of knob.
 * @param[in]   value   Value of knob.
 */
static void _knob_callback(uint8_t index, uint8_t value) {

    uint16_t param;

    switch (index) {

    case KNOB_PITCH:
        ft_set_module_param(0, PARAM_TUNE, g_octave_tune_lut[value]);
        gui_post_param("Pitch: ", value);
        break;

    case KNOB_ATTACK:

        // if (g_shift_held) {
        //     if (g_amp_eg) {
        //         ft_set_module_param(0, PARAM_AMP_ENV_SUSTAIN,
        //         amp_lut[value]); gui_post_param("Amp Sus: ", value);
        //     } else {
        //         ft_set_module_param(0, PARAM_FILTER_ENV_SUSTAIN,
        //                             amp_lut[value]);
        //         gui_post_param("Fil Sus: ", value);
        //     }
        //
        // } else {
        //     if (g_amp_eg) {
        //         ft_set_module_param(0, PARAM_AMP_ENV_ATTACK,
        //                             integrator_lut[value]);
        //         gui_post_param("Amp Atk: ", value);
        //     } else {
        //         ft_set_module_param(0, PARAM_FILTER_ENV_ATTACK,
        //                             integrator_lut[value]);
        //         gui_post_param("Fil Atk: ", value);
        //     }
        // }

        break;

    case KNOB_DECAY:
        // if (g_shift_held) {
        //     if (g_amp_eg) {
        //         ft_set_module_param(0, PARAM_AMP_ENV_RELEASE,
        //                             integrator_lut[value]);
        //         gui_post_param("Amp Rel: ", value);
        //     } else {
        //         ft_set_module_param(0, PARAM_FILTER_ENV_RELEASE,
        //                             integrator_lut[value]);
        //         gui_post_param("Fil Rel: ", value);
        //     }
        //
        // } else {
        //     if (g_amp_eg) {
        //         ft_set_module_param(0, PARAM_AMP_ENV_DECAY,
        //                             integrator_lut[value]);
        //         gui_post_param("Amp Dec: ", value);
        //     } else {
        //         ft_set_module_param(0, PARAM_FILTER_ENV_DECAY,
        //                             integrator_lut[value]);
        //         gui_post_param("Fil Dec: ", value);
        //     }
        // }
        break;

    case KNOB_LEVEL:
        ft_set_module_param(0, PARAM_AMP_LEVEL, amp_lut[value]);
        gui_post_param("Amp Level: ", value);
        break;

    case KNOB_RES:
        ft_set_module_param(0, PARAM_RES, g_filter_res_lut[value]);
        gui_post_param("Resonance: ", value);
        break;

    case KNOB_EG:
        // ft_set_module_param(0, PARAM_FILTER_ENV_DEPTH, amp_lut[value]);
        // gui_post_param("EG Depth: ", value);
        break;

    case KNOB_MOD_DEPTH:
        _set_mod_depth(value << 23);
        break;

    case KNOB_MOD_SPEED:
        _set_mod_speed(value << 13);
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

    static uint8_t pitch = DEFAULT_CUTOFF;
    static int8_t osc_type = DEFAULT_OSC_TYPE;
    static int8_t mod_type;

    switch (index) {

    case ENCODER_CUTOFF:

        if (value == 0x01) {

            if (pitch < 0x7f) {
                pitch++;
                ft_set_module_param(0, PARAM_CUTOFF, g_midi_hz_lut[pitch]);
            }

        } else {
            if (pitch > 0) {
                pitch--;
                ft_set_module_param(0, PARAM_CUTOFF, g_midi_hz_lut[pitch]);
            }
        }
        gui_post_param("Cutoff: ", pitch);

        break;

    case ENCODER_OSC:

        if (value == 0x01) {
            osc_type++;
            if (osc_type > OSC_TYPE_MAX) {
                osc_type = 0;
            }
        } else {
            osc_type--;
            if (osc_type < 0) {
                osc_type = OSC_TYPE_MAX;
            }
        }
        ft_set_module_param(0, PARAM_OSC_TYPE, osc_type);
        gui_post_param("Osc Type: ", osc_type);

        break;

    case ENCODER_MOD:

        if (value == 0x01) {
            mod_type++;
            if (mod_type > MOD_TYPE_MAX) {
                mod_type = 0;
            }
        } else {
            mod_type--;
            if (mod_type < 0) {
                mod_type = MOD_TYPE_MAX;
            }
        }

        g_mod_type = mod_type;
        gui_post_param("Mod Type: ", mod_type);

        break;

    default:
        break;
    }
}

static void _trigger_callback(uint8_t pad, uint8_t vel, bool state) {

    int32_t freq;
    uint8_t note;

    note = keyboard_map_note(&g_kbd, pad);

    if (state) {

        freq = g_midi_hz_lut[note];

        ft_set_module_param(0, PARAM_PHASE, 0);
        ft_set_module_param(0, PARAM_FREQ, freq);
        // ft_set_module_param(0, PARAM_VEL, vel << 23);
        // ft_set_module_param(0, PARAM_GATE, state);
        tADSRS_on(&g_amp_env, vel / 128.0);

    } else {

        // ft_set_module_param(0, PARAM_VEL, vel << 23);
        // ft_set_module_param(0, PARAM_GATE, state);
        tADSRS_off(&g_amp_env);
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
        g_menu_held = state;
        break;

    case BUTTON_SHIFT:
        g_shift_held = state;
        break;

    case BUTTON_AMP_EG:
        if (state) {
            g_amp_eg = !g_amp_eg;
            ft_set_led(LED_AMP_EG, g_amp_eg);
        }
        break;

    case BUTTON_LPF:
        if (state) {
            _set_filter_type(FILTER_TYPE_LPF);
        }
        break;

    case BUTTON_BPF:
        if (state) {
            _set_filter_type(FILTER_TYPE_BPF);
        }
        break;

    case BUTTON_HPF:
        if (state) {
            _set_filter_type(FILTER_TYPE_HPF);
        }
        break;

    default:
        break;
    }
}

static void _set_filter_type(uint8_t filter_type) {

    switch (filter_type) {

    case FILTER_TYPE_LPF:
        ft_set_led(LED_LPF, 1);
        ft_set_led(LED_BPF, 0);
        ft_set_led(LED_HPF, 0);
        ft_set_module_param(0, PARAM_FILTER_TYPE, FILTER_TYPE_LPF);
        break;

    case FILTER_TYPE_BPF:
        ft_set_led(LED_LPF, 0);
        ft_set_led(LED_BPF, 1);
        ft_set_led(LED_HPF, 0);
        ft_set_module_param(0, PARAM_FILTER_TYPE, FILTER_TYPE_BPF);
        break;

    case FILTER_TYPE_HPF:
        ft_set_led(LED_LPF, 0);
        ft_set_led(LED_BPF, 0);
        ft_set_led(LED_HPF, 1);
        ft_set_module_param(0, PARAM_FILTER_TYPE, FILTER_TYPE_HPF);
        break;

    default:
        break;
    }
    gui_post_param("Fil Type: ", filter_type);
}

static void _set_mod_depth(uint32_t mod_depth) {

    switch (g_mod_type) {

    case MOD_AMP_LFO:
        // ft_set_module_param(0, PARAM_AMP_LFO_DEPTH, mod_depth);
        // gui_post_param("A.LFO Dpt: ", mod_depth >> 23);
        break;

    case MOD_FILTER_LFO:
        // ft_set_module_param(0, PARAM_FILTER_LFO_DEPTH, mod_depth);
        // gui_post_param("F.LFO Dpt: ", mod_depth >> 23);
        break;

    case MOD_PITCH_LFO:
        // ft_set_module_param(0, PARAM_PITCH_LFO_DEPTH, mod_depth);
        // gui_post_param("P.LFO Dpt: ", mod_depth >> 23);
        break;

    default:
        break;
    }
}

static void _set_mod_speed(uint32_t mod_speed) {

    switch (g_mod_type) {

    case MOD_AMP_LFO:
        // ft_set_module_param(0, PARAM_AMP_LFO_SPEED, mod_speed);
        // gui_post_param("A.LFO Spd: ", mod_speed >> 13);
        break;

    case MOD_FILTER_LFO:
        // ft_set_module_param(0, PARAM_FILTER_LFO_SPEED, mod_speed);
        // gui_post_param("F.LFO Spd: ", mod_speed >> 13);
        break;

    case MOD_PITCH_LFO:
        // ft_set_module_param(0, PARAM_PITCH_LFO_SPEED, mod_speed);
        // gui_post_param("P.LFO Spd: ", mod_speed >> 13);
        break;

    default:
        break;
    }
}

/*----- End of file --------------------------------------------------*/
