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

#include "leaf-config.h"
#include "param_scale.h"
#include "svc_panel.h"

#include "gui_task.h"

#include "leaf.h"

#include "module_interface.h"


/*----- Macros -------------------------------------------------------*/

#define CONTROL_RATE (1000)
#define MEMPOOL_SIZE (0x1000)

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
#define BUTTON_EXIT 0x0d

#define DEFAULT_SCALE_NOTES NOTES_PHRYGIAN_DOMINANT
#define DEFAULT_SCALE_TONES 12
#define DEFAULT_SCALE_MODE 0

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static LEAF g_leaf;
static char g_mempool[MEMPOOL_SIZE];

static t_keyboard g_kbd;
static t_scale g_scale;

static e_mod_type g_mod_type;

static bool g_shift_held;
static bool g_menu_held;
static bool g_amp_eg;
static bool g_retrigger;

static float g_midi_pitch_cv_lut[128];
static float g_amp_cv_lut[256];
static float g_knob_cv_lut[256];

static float g_midi_hz_lut[128];
static float g_octave_tune_lut[256];
static float g_filter_res_lut[256];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _tick_callback(void);
static void _knob_callback(uint8_t index, uint8_t value);
static void _encoder_callback(uint8_t index, uint8_t value);
static void _button_callback(uint8_t index, bool state);
static void _trigger_callback_2(uint8_t pad, uint8_t vel, bool state);
static void _note_on_callback(char chan, char note, char vel);
static void _note_off_callback(char chan, char note, char vel);
static void process_note_event(uint8_t note, uint8_t vel, bool state) ;

static void _set_filter_type(uint8_t filter_type);
static void _set_mod_depth(uint32_t mod_depth);
static void _set_mod_speed(uint32_t mod_speed);

static void _lut_init(void);

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
voice_manager_init();
    t_status status = ERROR;

    LEAF_init(&g_leaf, CONTROL_RATE, g_mempool, MEMPOOL_SIZE, NULL);

    module_init(&g_leaf);
    _set_filter_type(FILTER_TYPE_LPF);

    _lut_init();

    scale_init(&g_scale, DEFAULT_SCALE_NOTES, DEFAULT_SCALE_TONES);
    keyboard_init(&g_kbd, &g_scale);
    g_kbd.octave = 4;

    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    ft_register_panel_callback(ENCODER_EVENT, _encoder_callback);
    ft_register_panel_callback(BUTTON_EVENT, _button_callback);
    ft_register_panel_callback(TRIGGER_EVENT, _trigger_callback_2);
    ft_register_midi_callback(EVT_CHAN_NOTE_ON, _note_on_callback);
    ft_register_midi_callback(EVT_CHAN_NOTE_OFF, _note_off_callback);

    ft_register_tick_callback(0, _tick_callback);

    // Initialise GUI.
    gui_task();

    ft_print("PolySynth Example");
    gui_print(4, 7, "PolySynth Example");

    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 */
void app_run(void) { gui_task(); }

/*----- Static function implementations ------------------------------*/

static void _tick_callback(void) {
    //
    module_process();
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
        module_set_param_all_voices(PARAM_TUNE, g_octave_tune_lut[value] / 2.0);
        gui_post_param("Pitch: ", value);
        break;

    case KNOB_ATTACK:
        if (g_shift_held) {

            if (g_amp_eg) {
                module_set_param_all_voices(PARAM_AMP_ENV_SUSTAIN, g_knob_cv_lut[value]);
                gui_post_param("Amp Sus: ", value);

            } else {
                module_set_param_all_voices(PARAM_FILTER_ENV_SUSTAIN,
                                 g_knob_cv_lut[value]);

                gui_post_param("Fil Sus: ", value);
            }

        } else {
            if (g_amp_eg) {
                module_set_param_all_voices(PARAM_AMP_ENV_ATTACK, g_knob_cv_lut[value]);
                gui_post_param("Amp Atk: ", value);

            } else {
                module_set_param_all_voices(PARAM_FILTER_ENV_ATTACK, g_knob_cv_lut[value]);
                gui_post_param("Fil Atk: ", value);
            }
        }
        break;

    case KNOB_DECAY:
        if (g_shift_held) {

            if (g_amp_eg) {
                module_set_param_all_voices(PARAM_AMP_ENV_RELEASE, g_knob_cv_lut[value]);
                gui_post_param("Amp Rel: ", value);

            } else {
                module_set_param_all_voices(PARAM_FILTER_ENV_RELEASE,
                                 g_knob_cv_lut[value]);

                gui_post_param("Fil Rel: ", value);
            }

        } else {
            if (g_amp_eg) {
                module_set_param_all_voices(PARAM_AMP_ENV_DECAY, g_knob_cv_lut[value]);
                gui_post_param("Amp Dec: ", value);

            } else {
                module_set_param_all_voices(PARAM_FILTER_ENV_DECAY, g_knob_cv_lut[value]);
                gui_post_param("Fil Dec: ", value);
            }
        }
        break;

    case KNOB_LEVEL:
        module_set_param_all_voices(PARAM_AMP_LEVEL, g_amp_cv_lut[value]);
        gui_post_param("Amp Level: ", value);
        break;

    case KNOB_RES:
        module_set_param_all_voices(PARAM_RES, g_knob_cv_lut[value]);
        gui_post_param("Resonance: ", value);
        break;

    case KNOB_EG:
        module_set_param_all_voices(PARAM_FILTER_ENV_DEPTH, g_knob_cv_lut[value]);
        gui_post_param("EG Depth: ", value);
        break;

    case KNOB_MOD_DEPTH:
        _set_mod_depth(value);
        break;

    case KNOB_MOD_SPEED:
        _set_mod_speed(value);
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

    static uint8_t cutoff = DEFAULT_CUTOFF;
    static int8_t osc_type = DEFAULT_OSC_TYPE;
    static int8_t mod_type;

    switch (index) {

    case ENCODER_CUTOFF:

        if (value == 0x01) {

            if (cutoff < 0x7f) {
                cutoff++;
            }

        } else {
            if (cutoff > 0) {
                cutoff--;
            }
        }
        module_set_param_all_voices(PARAM_FILTER_BASE_CUTOFF, g_midi_pitch_cv_lut[cutoff]);
        gui_post_param("Cutoff: ", cutoff);

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
        if (g_shift_held) {
            module_set_param_all_voices(PARAM_OSC_2_TYPE, (1.0 / OSC_TYPE_COUNT) * osc_type);
            gui_post_param("Osc 2 Type: ", osc_type);
        } else {
            module_set_param_all_voices(PARAM_OSC_TYPE, (1.0 / OSC_TYPE_COUNT) * osc_type);
            gui_post_param("Osc 1 Type: ", osc_type);
        }
        

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
        //gui_post_param("Mod Type: ", mod_type);
        gui_show_mod_type(mod_type);

        break;

    default:
        break;
    }
}

static void process_note_event(uint8_t note, uint8_t vel, bool state) {
    uint8_t voice_idx;
    if (state) {  
        /* Note ON event */
        
        /* Check if note is already active (prevent retriggering) */
        voice_idx = voice_manager_get_voice_by_note(note);
        if (voice_idx != INVALID_VOICE) {
               //return;
            // Note already active, just retrigger envelope 
            module_set_param_voice(voice_idx, PARAM_VEL, 1.0);
            module_set_param_voice(voice_idx, PARAM_GATE, true);
            module_set_param_voice(voice_idx, PARAM_PHASE_RESET, true);
            voice_manager_set_voice_in_release_stage(voice_idx,false);
            return;
        }
        
        /* Find available voice */
        voice_idx = voice_manager_get_free_voice();
        
        /* If no free voices, use voice stealing */
        if (voice_idx == INVALID_VOICE) {
            //ft_print("no free vices");
            return;
            voice_idx = voice_manager_get_oldest_voice();
            
            /* Turn off gate for stolen voice */
            //module_set_param_voice(voice_idx, PARAM_GATE, false);
            voice_manager_set_voice_in_release_stage(voice_idx,false);

        }
        
        /* Assign new note to selected voice */
        voice_manager_assign_note(voice_idx, note);
        
        /* Configure voice parameters */
        module_set_param_voice(voice_idx, PARAM_VEL, 1.0);
        module_set_param_voice(voice_idx, PARAM_GATE, true);
        module_set_param_voice(voice_idx, PARAM_OSC_BASE_FREQ, g_midi_pitch_cv_lut[note]);
        module_set_param_voice(voice_idx, PARAM_FREQ, g_midi_pitch_cv_lut[note]);
        module_set_param_voice(voice_idx, PARAM_PHASE_RESET, true);
        
        
    } else {  
        /* Note OFF event */
        
        /* Find and release the voice assigned to this note */
        voice_idx = voice_manager_get_voice_by_note(note);
        if (voice_idx != INVALID_VOICE) {

            bool in_release_stage = voice_manager_is_voice_in_release_stage(voice_idx);
            if (in_release_stage) {
                /* Voice is already in release stage, do nothing */
                ft_print("Voice already in release stage");
                return;
            }
            /* Turn off gate (start release phase) */
            module_set_param_voice(voice_idx, PARAM_GATE, false);
            voice_manager_set_voice_in_release_stage(voice_idx,true);
            
            /* Release note allocation */
           // voice_manager_release_voice(voice_idx);
            //voice_manager_release_note(note);
            
        }
    }
}


/**
 * @brief Polyphonic trigger callback function
 * 
 * Handles note on/off events with polyphonic voice allocation.
 * Supports up to MAX_VOICES simultaneous notes with voice stealing
 * when all voices are in use.
 * 
 * @param pad Input pad/key identifier
 * @param vel Velocity value (0-127)
 * @param state Note state (true = note on, false = note off)
 */
static void _trigger_callback_2(uint8_t pad, uint8_t vel, bool state) {
    uint8_t note = keyboard_map_note(&g_kbd, pad);

process_note_event(note, vel, state);


}



/**
 * @brief   Callback triggered by panel button events.
 *
 * @param[in]   index   Index of button.
 * @param[in]   state   State of button.
 */
static void _button_callback(uint8_t index, bool state) {

    switch (index) {
case BUTTON_EXIT:
        if (state == 1) {
            ft_shutdown();
        }
        break;
    case BUTTON_MENU:
        g_menu_held = state;
        break;

    case BUTTON_SHIFT:
        g_shift_held = state;
        break;

    case BUTTON_AMP_EG:
        if (state) {

            if (g_shift_held) {

                g_retrigger = !g_retrigger;
                module_set_param_all_voices(PARAM_RETRIGGER, g_retrigger);
                gui_post_param("Env Retrig: ", g_retrigger);

            } else {
                g_amp_eg = !g_amp_eg;
                ft_set_led(LED_AMP_EG, g_amp_eg);
            }
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

        module_set_param_all_voices(PARAM_FILTER_TYPE,
                         (1.0 / OSC_TYPE_COUNT) * FILTER_TYPE_LPF);
        break;

    case FILTER_TYPE_BPF:
        ft_set_led(LED_LPF, 0);
        ft_set_led(LED_BPF, 1);
        ft_set_led(LED_HPF, 0);

        module_set_param_all_voices(PARAM_FILTER_TYPE,
                         (1.0 / OSC_TYPE_COUNT) * FILTER_TYPE_BPF);
        break;

    case FILTER_TYPE_HPF:
        ft_set_led(LED_LPF, 0);
        ft_set_led(LED_BPF, 0);
        ft_set_led(LED_HPF, 1);

        module_set_param_all_voices(PARAM_FILTER_TYPE,
                         (1.0 / OSC_TYPE_COUNT) * FILTER_TYPE_HPF);
        break;

    default:
        break;
    }
    gui_post_param("Fil Type: ", filter_type);
}

static void _set_mod_depth(uint32_t mod_depth) {

    switch (g_mod_type) {

    case MOD_AMP_LFO:
        module_set_param_all_voices(PARAM_AMP_LFO_DEPTH, g_knob_cv_lut[mod_depth]);
        gui_post_param("A.LFO Dpt: ", mod_depth);
        break;

    case MOD_FILTER_LFO:
        module_set_param_all_voices(PARAM_FILTER_LFO_DEPTH, g_knob_cv_lut[mod_depth]);
        gui_post_param("F.LFO Dpt: ", mod_depth);
        break;

    case MOD_PITCH_LFO:
        module_set_param_all_voices(PARAM_PITCH_LFO_DEPTH, g_knob_cv_lut[mod_depth]);
        gui_post_param("P.LFO Dpt: ", mod_depth);
        break;

    default:
        break;
    }
}

static void _set_mod_speed(uint32_t mod_speed) {

    switch (g_mod_type) {

    case MOD_AMP_LFO:
        module_set_param_all_voices(PARAM_AMP_LFO_SPEED, g_knob_cv_lut[mod_speed]);
        gui_post_param("A.LFO Spd: ", mod_speed);
        break;

    case MOD_FILTER_LFO:
        module_set_param_all_voices(PARAM_FILTER_LFO_SPEED, g_knob_cv_lut[mod_speed]);
        gui_post_param("F.LFO Spd: ", mod_speed);
        break;

    case MOD_PITCH_LFO:
        module_set_param_all_voices(PARAM_PITCH_LFO_SPEED, g_knob_cv_lut[mod_speed]);
        gui_post_param("P.LFO Spd: ", mod_speed);
        break;

    default:
        break;
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
    uint8_t note_int = (uint8_t)note;
    uint8_t vel_int = (uint8_t)vel;
    process_note_event(note_int, vel_int, true);
}
/**
 * @brief   Callback triggered by MIDI note off events.
 *
 * Echo received note on messages to MIDI output.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   note    MIDI note number.
 * @param[in]   vel     MIDI note velocity.
 */
static void _note_off_callback(char chan, char note, char vel) {
    uint8_t note_int = (uint8_t)note;
    uint8_t vel_int = (uint8_t)vel;
    process_note_event(note_int, vel_int, false);

}


static void _lut_init(void) {

    int i;
    float scaled;

    for (i = 0; i <= 127; i++) {

        g_midi_pitch_cv_lut[i] = note_to_cv(i);
    }

    for (i = 0; i <= 255; i++) {

        scaled = i / 255.0;
        g_amp_cv_lut[i] = powf(scaled, 2);
    }

    for (i = 0; i <= 255; i++) {
        g_knob_cv_lut[i] = i / 255.0;
    }

    /// TODO: Should be log.
    //
    // Initialise pitch mod lookup table.
    float tune;
    for (i = 0; i <= 255; i++) {

        if (i <= 128) {
            // 0.5...1.
            tune = (i / 256.0) + 0.5;

        } else {
            // >1...2.0
            tune = ((i - 128) / 127.0) + 1;
        }

        // // Convert to fix16,
        // tune *= (1 << 16);
        // g_octave_tune_lut[i] = (int32_t)tune;

        g_octave_tune_lut[i] = tune;
    }

    int32_t res;
    for (i = 0; i <= 255; i++) {

        res = 0x7fffffff - (i * (1 << 23));

        g_filter_res_lut[i] = res;
    }
}

/*----- End of file --------------------------------------------------*/
