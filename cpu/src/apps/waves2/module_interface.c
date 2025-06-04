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
 * @file    module_interface.c
 *
 * @brief   CPU interface interface to DSP module.
 */

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

#include "leaf.h"

#include "leaf-envelopes.h"
#include "leaf-oscillators.h"

#include "param_scale.h"

#include "module_interface.h"

/*----- Macros -------------------------------------------------------*/

#define EXP_BUFFER_SIZE (0x200)

#define GATE_OPEN (1)
#define GATE_CLOSED (0)
#define GATE_THRESHOLD (0.2)

#define PARAM_VOICE_OFFSET(voice, param, paramCount) (param + (voice * paramCount))
#define REMOVE_PARAM_OFFSET(param_index_with_offset,paramCount) ((param_index_with_offset) % paramCount)
#define PARAM_VOICE_NUMBER(param_index_with_offset,paramCount) ((param_index_with_offset) / paramCount)

/*----- Typedefs -----------------------------------------------------*/

typedef struct {

    float next;
    float last;
    bool changed;
} t_cv;

typedef struct {
    tADSRT amp_env;
    tADSRT filter_env;
    tADSRT pitch_env;

    float vel;

    tTriLFO amp_lfo;
    float amp_lfo_depth;

    tTriLFO filter_lfo;
    float filter_lfo_depth;

    float filter_cutoff;
    float filter_env_depth;

    float osc_freq;
    float pitch_env_depth;

    tTriLFO pitch_lfo;
    float pitch_lfo_depth;

    t_cv amp_cv;
    t_cv filter_cv;
    t_cv pitch_cv;

    e_mod_type mod_type;

    bool gate;
    bool retrigger;
    bool reset_phase;

} t_module;

/*----- Static variable definitions ----------------------------------*/

static t_module g_module[MAX_VOICES];

static Lfloat g_exp_buffer[EXP_BUFFER_SIZE];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static bool _cv_update(t_cv *cv, float value);

/*----- Extern function implementations ------------------------------*/
void module_init(LEAF *leaf) {

    int voice_index;
    for (voice_index = 0; voice_index < MAX_VOICES; voice_index++) {

        LEAF_generate_exp(g_exp_buffer, 0.001f, 0.0f, 1.0f, -0.0008f,
                        EXP_BUFFER_SIZE);

        tADSRT_init(&g_module[voice_index].amp_env, 0, 1024, 1, 1024, g_exp_buffer,
                    EXP_BUFFER_SIZE, leaf);

        tADSRT_init(&g_module[voice_index].filter_env, 0, 1024, 1, 1024, g_exp_buffer,
                    EXP_BUFFER_SIZE, leaf);

        tADSRT_init(&g_module[voice_index].pitch_env, 200, 0, 1, 0, g_exp_buffer,EXP_BUFFER_SIZE, leaf);

        tTriLFO_init(&g_module[voice_index].amp_lfo, leaf);
        tTriLFO_init(&g_module[voice_index].filter_lfo, leaf);
        tTriLFO_init(&g_module[voice_index].pitch_lfo, leaf);

        tTriLFO_setFreq(&g_module[voice_index].amp_lfo, 10);
        tTriLFO_setFreq(&g_module[voice_index].filter_lfo, 10);
        tTriLFO_setFreq(&g_module[voice_index].pitch_lfo, 0);
    }
}

void module_process(void) {


    int voice_index;
    for (voice_index = 0; voice_index < MAX_VOICES; voice_index++) {

        float amp_mod;
        float filter_mod;
        float pitch_mod;

        if (g_module[voice_index].reset_phase) {

            module_set_param_voice(voice_index,PARAM_OSC_PHASE, 0);
            module_set_param_voice(voice_index,PARAM_LFO_PHASE, 0);

            g_module[voice_index].reset_phase = false;
        }

        // Amplitude modulation.
        //
        amp_mod = tADSRT_tick(&g_module[voice_index].amp_env);

        //amp_mod += amp_mod * tTriLFO_tick(&g_module[voice_index].amp_lfo) * g_module[voice_index].amp_lfo_depth;




        // Only send parameters to DSP if they have changed.
        if (_cv_update(&g_module[voice_index].amp_cv, amp_mod)) {
            float clampled_amp = clamp_value(amp_mod);
            
            if (clampled_amp <= 0.3f && voice_manager_is_voice_in_release_stage(voice_index)) {
                voice_manager_release_voice(voice_index);
            
            }

            module_set_param_voice(voice_index,PARAM_AMP,clampled_amp);
        }

        /// TODO: Should filter LFO follow the
        ///       envelope, like the amp LFO?

        // Filter cutoff modulation.
        //
        filter_mod = g_module[voice_index].filter_cutoff;

        filter_mod += tADSRT_tick(&g_module[voice_index].filter_env) * g_module[voice_index].filter_env_depth;

        filter_mod += tTriLFO_tick(&g_module[voice_index].filter_lfo) * g_module[voice_index].filter_lfo_depth;

        if (_cv_update(&g_module[voice_index].filter_cv, filter_mod)) {

            module_set_param_voice(voice_index,PARAM_CUTOFF, clamp_value(filter_mod));
        }

        // Pitch modulation.
        //
        pitch_mod = g_module[voice_index].osc_freq;

        /// TODO: Add control for pitch envelope.
        //
        pitch_mod += tADSRT_tick(&g_module[voice_index].pitch_env) * g_module[voice_index].pitch_env_depth;

        pitch_mod += (tTriLFO_tick(&g_module[voice_index].pitch_lfo) * g_module[voice_index].pitch_lfo_depth);

        if (_cv_update(&g_module[voice_index].pitch_cv, pitch_mod)) {

            module_set_param_voice(voice_index,PARAM_FREQ, clamp_value(pitch_mod));
        }
    }
}


void module_set_param_all_voices(uint16_t param_index_without_offset, float value) {
    int i;
    for (i = 0; i < MAX_VOICES; i++) {
        module_set_param_voice(i, param_index_without_offset, value);
    }


}
/**
 * @brief       Handle module parameter changes.
 *              Receives float, -1.0 <= value < 1.0,
 *              converts to format required by destination.
 *
 * @param[in]   param_index     Index of parameter.
 * @param[in]   value           Value of parameter.
 *
 */
void module_set_param_voice(uint16_t voice_index,uint16_t param_index_without_offset, float value) {

    uint16_t param_index = PARAM_VOICE_OFFSET(voice_index, param_index_without_offset, PARAM_COUNT);

    switch (param_index_without_offset) {

    case PARAM_AMP:
        ft_set_module_param(0, param_index, float_to_fract32(value));
        break;

    case PARAM_FREQ:
        ft_set_module_param(0, param_index,
                            float_to_fix16(cv_to_osc_freq(value)));
        break;

    case PARAM_OSC_PHASE:
        ft_set_module_param(0, param_index, float_to_fract32(value));
        break;

    case PARAM_LFO_PHASE:
        tTriLFO_setPhase(&g_module[voice_index].amp_lfo, float_to_fract32(value));
        tTriLFO_setPhase(&g_module[voice_index].filter_lfo, float_to_fract32(value));
        tTriLFO_setPhase(&g_module[voice_index].pitch_lfo, float_to_fract32(value));
        break;

    case PARAM_GATE:

        /// TODO:  This ignores velocity for repeated notes,
        ///        do we want this?
        //
        g_module[voice_index].retrigger = true;
        if (((g_module[voice_index].retrigger == true) || (g_module[voice_index].gate == GATE_CLOSED)) &&
            (value >= GATE_THRESHOLD)) {

            g_module[voice_index].gate = GATE_OPEN;
            tADSRT_on(&g_module[voice_index].amp_env, g_module[voice_index].vel);
            tADSRT_on(&g_module[voice_index].filter_env, 1);

        } else if (value < GATE_THRESHOLD) {
            g_module[voice_index].gate = GATE_CLOSED;
            tADSRT_off(&g_module[voice_index].amp_env);
            tADSRT_off(&g_module[voice_index].filter_env);
        }
        break;

    case PARAM_VEL:
        g_module[voice_index].vel = value;
        break;

    case PARAM_AMP_LEVEL:
        ft_set_module_param(0, param_index, float_to_fract32(value));
        break;

    case PARAM_AMP_ENV_ATTACK:
        tADSRT_setAttack(&g_module[voice_index].amp_env, value * 8192.0);
        break;

    case PARAM_AMP_ENV_DECAY:
        tADSRT_setDecay(&g_module[voice_index].amp_env, value * 8192.0);
        break;

    case PARAM_AMP_ENV_SUSTAIN:
        tADSRT_setSustain(&g_module[voice_index].amp_env, value);
        break;

    case PARAM_AMP_ENV_RELEASE:
        tADSRT_setRelease(&g_module[voice_index].amp_env, value * 8192.0);
        break;

    case PARAM_AMP_ENV_DEPTH:
        break;

    case PARAM_FILTER_ENV_DEPTH:
        g_module[voice_index].filter_env_depth = value;
        break;

    case PARAM_FILTER_ENV_ATTACK:
        tADSRT_setAttack(&g_module[voice_index].filter_env, value * 8192.0);
        break;

    case PARAM_FILTER_ENV_DECAY:
        tADSRT_setDecay(&g_module[voice_index].filter_env, value * 8192.0);
        break;

    case PARAM_FILTER_ENV_SUSTAIN:
        tADSRT_setSustain(&g_module[voice_index].filter_env, value);
        break;

    case PARAM_FILTER_ENV_RELEASE:
        tADSRT_setRelease(&g_module[voice_index].filter_env, value * 8192.0);
        break;

    case PARAM_PITCH_ENV_DEPTH:
        break;

    case PARAM_PITCH_ENV_ATTACK:
        break;

    case PARAM_PITCH_ENV_DECAY:
        break;

    case PARAM_PITCH_ENV_SUSTAIN:
        break;

    case PARAM_PITCH_ENV_RELEASE:
        break;

    case PARAM_CUTOFF:
        ft_set_module_param(
            0, param_index,
            float_to_fix16(cv_to_filter_freq_oversample(value)));
        break;

    case PARAM_RES:
        ft_set_module_param(0, param_index, float_to_fract32(1.0 - value));
        break;

    case PARAM_TUNE:
        //ft_set_module_param(0, param_index, float_to_fix16(value * 2.0));
        ft_set_module_param(0, param_index, value );
        break;

    case PARAM_OSC_TYPE:
    case PARAM_OSC_2_TYPE:
        ft_set_module_param(0, param_index, value * OSC_TYPE_COUNT);
        break;

    case PARAM_FILTER_TYPE:
        ft_set_module_param(0, PARAM_FILTER_TYPE, value * OSC_TYPE_COUNT);
        break;

    case PARAM_AMP_LFO_DEPTH:
        g_module[voice_index].amp_lfo_depth = value;
        break;

    case PARAM_AMP_LFO_SPEED:
        tTriLFO_setFreq(&g_module[voice_index].amp_lfo, value * 20);
        break;

    case PARAM_FILTER_LFO_DEPTH:
        g_module[voice_index].filter_lfo_depth = value;
        break;

    case PARAM_FILTER_LFO_SPEED:
        tTriLFO_setFreq(&g_module[voice_index].filter_lfo, value * 20);
        break;

    case PARAM_PITCH_LFO_DEPTH:
        g_module[voice_index].pitch_lfo_depth = value;
        break;

    case PARAM_PITCH_LFO_SPEED:
        tTriLFO_setFreq(&g_module[voice_index].pitch_lfo, value * 20);
        break;

    case PARAM_OSC_BASE_FREQ:
        g_module[voice_index].osc_freq = value;
        break;

    case PARAM_FILTER_BASE_CUTOFF:
        g_module[voice_index].filter_cutoff = value;
        break;

    case PARAM_PHASE_RESET:
        g_module[voice_index].reset_phase = value;
        break;

    case PARAM_RETRIGGER:

        if (value > 0) {
            g_module[voice_index].retrigger = true;
        } else {
            g_module[voice_index].retrigger = false;
        }
        break;

    default:
        break;
    }
}

float module_get_param(uint16_t param_index) {

    float value = 0;

    switch (param_index) {

    default:
        break;
    }

    return value;
}

/*----- Static function implementations ------------------------------*/

static bool _cv_update(t_cv *cv, float value) {

    cv->next = value;

    if (cv->next != cv->last) {
        cv->last = cv->next;
        cv->changed = true;

    } else {
        cv->changed = false;
    }

    return cv->changed;
}

/*----- End of file --------------------------------------------------*/
