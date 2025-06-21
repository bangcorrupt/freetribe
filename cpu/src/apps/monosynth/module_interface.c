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
#include <stdint.h>

#include "module_interface.h"

/*----- Macros -------------------------------------------------------*/

#define EXP_BUFFER_SIZE (0x400)

#define GATE_OPEN (1)
#define GATE_CLOSED (0)
#define GATE_THRESHOLD (0.2)

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

static t_module g_module;

static Lfloat g_exp_buffer[EXP_BUFFER_SIZE];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static bool _cv_update(t_cv *cv, float value);

/*----- Extern function implementations ------------------------------*/

void module_init(LEAF *leaf) {

    LEAF_generate_exp(g_exp_buffer, 0.001f, 0.0f, 1.0f, -0.0008f,
                      EXP_BUFFER_SIZE);

    tADSRT_init(&g_module.amp_env, 0, 1024, 1, 1024, g_exp_buffer,
                EXP_BUFFER_SIZE, leaf);

    tADSRT_init(&g_module.filter_env, 0, 1024, 1, 1024, g_exp_buffer,
                EXP_BUFFER_SIZE, leaf);

    tADSRT_init(&g_module.pitch_env, 0, 1024, 1, 1024, g_exp_buffer,
                EXP_BUFFER_SIZE, leaf);

    tTriLFO_init(&g_module.amp_lfo, leaf);
    tTriLFO_init(&g_module.filter_lfo, leaf);
    tTriLFO_init(&g_module.pitch_lfo, leaf);

    tTriLFO_setFreq(&g_module.amp_lfo, 10);
    tTriLFO_setFreq(&g_module.filter_lfo, 10);
    tTriLFO_setFreq(&g_module.pitch_lfo, 10);
}

void module_process(void) {

    float amp_mod;
    float filter_mod;
    float pitch_mod;

    if (g_module.reset_phase) {

        module_set_param(PARAM_OSC_PHASE, 0);
        module_set_param(PARAM_LFO_PHASE, 0);

        g_module.reset_phase = false;
    }

    // Amplitude modulation.
    //
    amp_mod = tADSRT_tick(&g_module.amp_env);

    amp_mod +=
        amp_mod * tTriLFO_tick(&g_module.amp_lfo) * g_module.amp_lfo_depth;

    // Only send parameters to DSP if they have changed.
    if (_cv_update(&g_module.amp_cv, amp_mod)) {

        module_set_param(PARAM_AMP, clamp_value(amp_mod));
    }

    /// TODO: Should filter LFO follow the
    ///       envelope, like the amp LFO?

    // Filter cutoff modulation.
    //
    filter_mod = g_module.filter_cutoff;

    filter_mod += tADSRT_tick(&g_module.filter_env) * g_module.filter_env_depth;

    filter_mod +=
        tTriLFO_tick(&g_module.filter_lfo) * g_module.filter_lfo_depth;

    if (_cv_update(&g_module.filter_cv, filter_mod)) {

        module_set_param(PARAM_CUTOFF, clamp_value(filter_mod));
    }

    // Pitch modulation.
    //
    pitch_mod = g_module.osc_freq;

    /// TODO: Add control for pitch envelope.
    //
    // pitch_mod += tADSRT_tick(&g_module.pitch_env) * g_module.pitch_env_depth;

    pitch_mod += (tTriLFO_tick(&g_module.pitch_lfo) * g_module.pitch_lfo_depth);

    if (_cv_update(&g_module.pitch_cv, pitch_mod)) {

        module_set_param(PARAM_FREQ, clamp_value(pitch_mod));
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
void module_set_param(uint16_t param_index, float value) {

    switch (param_index) {

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
        tTriLFO_setPhase(&g_module.amp_lfo, float_to_fract32(value));
        tTriLFO_setPhase(&g_module.filter_lfo, float_to_fract32(value));
        tTriLFO_setPhase(&g_module.pitch_lfo, float_to_fract32(value));
        break;

    case PARAM_GATE:

        /// TODO:  This ignores velocity for repeated notes,
        ///        do we want this?
        //
        if (((g_module.retrigger == true) || (g_module.gate == GATE_CLOSED)) &&
            (value >= GATE_THRESHOLD)) {

            g_module.gate = GATE_OPEN;
            tADSRT_on(&g_module.amp_env, g_module.vel);
            tADSRT_on(&g_module.filter_env, 1);

        } else if (value < GATE_THRESHOLD) {
            g_module.gate = GATE_CLOSED;
            tADSRT_off(&g_module.amp_env);
            tADSRT_off(&g_module.filter_env);
        }
        break;

    case PARAM_VEL:
        g_module.vel = value;
        break;

    case PARAM_AMP_LEVEL:
        ft_set_module_param(0, param_index, float_to_fract32(value));
        break;

    case PARAM_AMP_ENV_ATTACK:
        tADSRT_setAttack(&g_module.amp_env, value * 8192.0);
        break;

    case PARAM_AMP_ENV_DECAY:
        tADSRT_setDecay(&g_module.amp_env, value * 8192.0);
        break;

    case PARAM_AMP_ENV_SUSTAIN:
        tADSRT_setSustain(&g_module.amp_env, value);
        break;

    case PARAM_AMP_ENV_RELEASE:
        tADSRT_setRelease(&g_module.amp_env, value * 8192.0);
        break;

    case PARAM_AMP_ENV_DEPTH:
        break;

    case PARAM_FILTER_ENV_DEPTH:
        g_module.filter_env_depth = value;
        break;

    case PARAM_FILTER_ENV_ATTACK:
        tADSRT_setAttack(&g_module.filter_env, value * 8192.0);
        break;

    case PARAM_FILTER_ENV_DECAY:
        tADSRT_setDecay(&g_module.filter_env, value * 8192.0);
        break;

    case PARAM_FILTER_ENV_SUSTAIN:
        tADSRT_setSustain(&g_module.filter_env, value);
        break;

    case PARAM_FILTER_ENV_RELEASE:
        tADSRT_setRelease(&g_module.filter_env, value * 8192.0);
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
        ft_set_module_param(0, param_index, float_to_fix16(value * 2.0));
        break;

    case PARAM_OSC_TYPE:
        ft_set_module_param(0, param_index, value * OSC_TYPE_COUNT);
        break;

    case PARAM_FILTER_TYPE:
        ft_set_module_param(0, PARAM_FILTER_TYPE, value * OSC_TYPE_COUNT);
        break;

    case PARAM_AMP_LFO_DEPTH:
        g_module.amp_lfo_depth = value;
        break;

    case PARAM_AMP_LFO_SPEED:
        tTriLFO_setFreq(&g_module.amp_lfo, value * 20);
        break;

    case PARAM_FILTER_LFO_DEPTH:
        g_module.filter_lfo_depth = value;
        break;

    case PARAM_FILTER_LFO_SPEED:
        tTriLFO_setFreq(&g_module.filter_lfo, value * 20);
        break;

    case PARAM_PITCH_LFO_DEPTH:
        g_module.pitch_lfo_depth = value;
        break;

    case PARAM_PITCH_LFO_SPEED:
        tTriLFO_setFreq(&g_module.pitch_lfo, value * 20);
        break;

    case PARAM_OSC_BASE_FREQ:
        g_module.osc_freq = value;
        break;

    case PARAM_FILTER_BASE_CUTOFF:
        g_module.filter_cutoff = value;
        break;

    case PARAM_PHASE_RESET:
        g_module.reset_phase = value;
        break;

    case PARAM_RETRIGGER:

        if (value > 0) {
            g_module.retrigger = true;
        } else {
            g_module.retrigger = false;
        }
        break;

    default:
        break;
    }
}

void module_get_param(uint16_t param_index) {

    switch (param_index) {

    default:
        break;
    }
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
