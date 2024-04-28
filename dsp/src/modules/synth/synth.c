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
 * @file    synth.c
 *
 * @brief   A synth module for Freetribe.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "fix.h"
#include "fix16.h"
#include "fix16_fract.h"
#include "fract_math.h"
#include "types.h"

#include "aleph.h"

// #include "env_adsr.h"
// #include "filter_ladder.h"
#include "filter_svf.h"
#include "osc_polyblep.h"
#include "phasor.h"
#include "ricks_tricks.h"
#include "ugens/env_adsr.h"
#include "ugens/filter.h" // Prefix path to prevent name conflict.
#include "ugens/oscillator.h"
#include "ugens/waveform.h"

#include "module.h"

#include "utils.h"

#include "synth.h"

/*----- Macros and Definitions ---------------------------------------*/

#define SAMPLERATE 48000
#define MEMPOOL_SIZE 0x4000

/// TODO: Struct for parameter type.
///         scaler,
///         range,
///         default,
///         display,
///         etc...,

// #define DEFAULT_CUTOFF 0x7f // Index in pitch LUT.
#define DEFAULT_OSC_TYPE 2

#define DEFAULT_AMP_ENV_ATTACK SLEW_10MS
#define DEFAULT_AMP_ENV_DECAY SLEW_1S
#define DEFAULT_AMP_ENV_SUSTAIN FR32_MAX

#define DEFAULT_FILTER_ENV_ATTACK SLEW_10MS
#define DEFAULT_FILTER_ENV_DECAY SLEW_1S
#define DEFAULT_FILTER_ENV_SUSTAIN FR32_MAX

#define DEFAULT_PITCH_ENV_ATTACK SLEW_100MS
#define DEFAULT_PITCH_ENV_DECAY SLEW_100MS
#define DEFAULT_PITCH_ENV_SUSTAIN 0

#define DEFAULT_PITCH_ENV_DEPTH FR32_MAX >> 16

#define DEFAULT_AMP_LFO_DEPTH 0
#define DEFAULT_AMP_LFO_SPEED FIX16_ONE

#define DEFAULT_FILTER_LFO_DEPTH 0
#define DEFAULT_FILTER_LFO_SPEED FIX16_ONE

#define DEFAULT_PITCH_LFO_DEPTH 0
#define DEFAULT_PITCH_LFO_SPEED FIX16_ONE

/**
 * @brief   Enumeration of module parameters.
 *
 * Index of each external parameter of module.
 */
typedef enum {
    PARAM_FREQ,
    PARAM_GATE,
    PARAM_VEL,
    PARAM_AMP_LEVEL,
    PARAM_AMP_ENV_ATTACK,
    PARAM_AMP_ENV_DECAY,
    PARAM_AMP_ENV_SUSTAIN,
    PARAM_AMP_ENV_RELEASE,
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
    FILTER_TYPE_BPF,
    FILTER_TYPE_HPF,

    FILTER_TYPE_COUNT
} e_filter_type;

typedef struct {

    Waveform waveform_a;
    Waveform waveform_b;

    fract32 freq;
    fract32 tune;

    FilterSVF filter;
    // filter_ladder ladder;
    HPF dc_blocker;

    fract32 cutoff;
    fract32 res;

    fract32 vel;
    fract32 amp_level;

    EnvADSR amp_env;
    EnvADSR pitch_env;
    EnvADSR filter_env;

    fract32 amp_env_depth;
    fract32 filter_env_depth;
    fract32 pitch_env_depth;

    Oscillator amp_lfo;
    Oscillator filter_lfo;
    Oscillator pitch_lfo;

    fract32 amp_lfo_depth;
    fract32 filter_lfo_depth;
    fract32 pitch_lfo_depth;

    e_osc_type osc_type;
    e_filter_type filter_type;

} t_MonoSynth;

/*----- Static variable definitions ----------------------------------*/

static t_Aleph g_aleph;
static t_MonoSynth g_voice;

static char g_mempool[MEMPOOL_SIZE];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void MonoSynth_init(t_MonoSynth *synth) {

    Waveform_init(&synth->waveform_a, &g_aleph);
    Waveform_init(&synth->waveform_b, &g_aleph);

    Oscillator_init(&synth->amp_lfo, &g_aleph);
    Oscillator_init(&synth->filter_lfo, &g_aleph);
    Oscillator_init(&synth->pitch_lfo, &g_aleph);

    EnvADSR_init(&synth->amp_env, &g_aleph);
    EnvADSR_init(&synth->filter_env, &g_aleph);
    EnvADSR_init(&synth->pitch_env, &g_aleph);

    FilterSVF_init(&synth->filter, &g_aleph);
    HPF_init(&synth->dc_blocker, &g_aleph);

    // filter_ladder_init(&synth->ladder);
}

void MonoSynth_set_freq(t_MonoSynth *synth) {
    //
}

fract32 MonoSynth_tick(t_MonoSynth *synth) {

    fract32 waveform;

    fract32 waveform_a;
    fract32 waveform_b;
    fract32 freq;

    fract32 amp_lfo;
    fract32 filter_lfo;
    fract32 pitch_lfo;

    fract32 amp_env;
    fract32 filter_env;
    fract32 pitch_env;

    fract32 cutoff;

    // Calculate pitch LFO.
    pitch_lfo = Oscillator_next(&synth->pitch_lfo);

    // Scale pitch LFO depth.
    pitch_lfo = mult_fr1x32x32(pitch_lfo, synth->pitch_lfo_depth);

    // Calculate pitch envelope.
    pitch_env = EnvADSR_next(&synth->pitch_env);

    // Scale pitch envelope depth.
    pitch_env = mult_fr1x32x32(pitch_env, synth->pitch_env_depth);

    // Calculate amp LFO.
    amp_lfo = Oscillator_next(&synth->amp_lfo);

    // Scale amp LFO depth.
    amp_lfo = mult_fr1x32x32(amp_lfo, synth->amp_lfo_depth);

    // Calculate amplitude envelope.
    amp_env = EnvADSR_next(&synth->amp_env);

    // Scale amplitude envelope by velocity.
    amp_env = mult_fr1x32x32(amp_env, synth->vel);

    // Calculate filter LFO.
    filter_lfo = Oscillator_next(&synth->filter_lfo);

    // Scale filter LFO depth.
    filter_lfo = mult_fr1x32x32(filter_lfo, synth->filter_lfo_depth);

    // Calculate filter envelope.
    filter_env = EnvADSR_next(&synth->filter_env);

    // Scale filter envelope depth.
    filter_env = mult_fr1x32x32(filter_env, synth->filter_env_depth);

    // Apply pitch envelope.
    freq = add_fr1x32(pitch_env, synth->freq);

    // Apply pitch LFO.
    freq = add_fr1x32(freq, mult_fr1x32x32(freq, pitch_lfo));

    // Set oscillator frequency.
    Waveform_set_freq(&synth->waveform_a, freq);
    Waveform_set_freq(&synth->waveform_b, fix16_mul_fract(freq, synth->tune));

    // Generate waveforms.
    Waveform_set_shape(&synth->waveform_a, synth->osc_type);
    Waveform_set_shape(&synth->waveform_b, synth->osc_type);

    // Shift right to prevent clipping.
    waveform_a = shr_fr1x32(Waveform_next(&synth->waveform_a), 1);
    waveform_b = shr_fr1x32(Waveform_next(&synth->waveform_b), 1);

    // Sum waveforms.
    waveform = add_fr1x32(waveform_a, waveform_b);

    // Apply amp envelope.
    waveform = mult_fr1x32x32(waveform, amp_env);

    // Apply amp LFO.
    waveform = add_fr1x32(waveform, mult_fr1x32x32(waveform, amp_lfo));

    // Apply filter envelope.
    cutoff = add_fr1x32(filter_env, synth->cutoff);

    // Apply filter LFO.
    cutoff = add_fr1x32(cutoff, mult_fr1x32x32(cutoff, filter_lfo));

    // Set filter cutoff and resonance.
    FilterSVF_set_coeff(&synth->filter, cutoff);
    FilterSVF_set_rq(&synth->filter, synth->res);

    // filter_ladder_set_freq(&synth->ladder, cutoff);
    // filter_ladder_set_fb(&synth->ladder, FR32_MAX - synth->res);

    // Apply filter.
    switch (synth->filter_type) {

    case FILTER_TYPE_LPF:
        waveform = FilterSVF_softclip_asym_lpf_next(&synth->filter, waveform);
        break;

    case FILTER_TYPE_BPF:
        waveform = FilterSVF_softclip_asym_bpf_next(&synth->filter, waveform);
        break;

    case FILTER_TYPE_HPF:
        waveform = FilterSVF_softclip_asym_hpf_next(&synth->filter, waveform);
        // waveform = filter_ladder_lpf_asym_os_next(&synth->ladder,
        // waveform);
        // waveform = filter_ladder_lpf_next(&synth->ladder, waveform);
        break;

    default:
        // Default to LPF.
        waveform = FilterSVF_softclip_asym_lpf_next(&synth->filter, waveform);
        break;
    }

    // Scale amplitude by level.
    waveform = mult_fr1x32x32(waveform, synth->amp_level);

    // Block DC.
    waveform = HPF_dc_block2(&synth->dc_blocker, waveform);

    return waveform;
}

/**
 * @brief   Initialise module.
 */
void module_init(void) {

    Aleph_init(&g_aleph, SAMPLERATE, g_mempool, MEMPOOL_SIZE, NULL);

    MonoSynth_init(&g_voice);

    /// TODO: Define defaults.
    module_set_param(PARAM_AMP_LEVEL, FR32_MAX);
    module_set_param(PARAM_OSC_TYPE, 2);
    module_set_param(PARAM_FREQ, 220 << 16);
    module_set_param(PARAM_TUNE, FIX16_ONE);
    module_set_param(PARAM_CUTOFF, 0x326f6abb);
    module_set_param(PARAM_RES, FR32_MAX);

    g_voice.freq = 220 << 16;

    module_set_param(PARAM_AMP_ENV_ATTACK, DEFAULT_AMP_ENV_ATTACK);
    module_set_param(PARAM_AMP_ENV_DECAY, DEFAULT_AMP_ENV_DECAY);
    module_set_param(PARAM_AMP_ENV_SUSTAIN, DEFAULT_AMP_ENV_SUSTAIN);

    module_set_param(PARAM_FILTER_ENV_ATTACK, DEFAULT_FILTER_ENV_ATTACK);
    module_set_param(PARAM_FILTER_ENV_DECAY, DEFAULT_FILTER_ENV_DECAY);
    module_set_param(PARAM_FILTER_ENV_SUSTAIN, DEFAULT_FILTER_ENV_SUSTAIN);

    module_set_param(PARAM_PITCH_ENV_ATTACK, DEFAULT_PITCH_ENV_ATTACK);
    module_set_param(PARAM_PITCH_ENV_DECAY, DEFAULT_PITCH_ENV_DECAY);
    module_set_param(PARAM_PITCH_ENV_SUSTAIN, DEFAULT_PITCH_ENV_SUSTAIN);

    module_set_param(PARAM_PITCH_ENV_DEPTH, DEFAULT_PITCH_ENV_DEPTH);

    module_set_param(PARAM_AMP_LFO_DEPTH, DEFAULT_AMP_LFO_DEPTH);
    module_set_param(PARAM_FILTER_LFO_DEPTH, DEFAULT_FILTER_LFO_DEPTH);
    module_set_param(PARAM_PITCH_LFO_DEPTH, DEFAULT_PITCH_LFO_DEPTH);
}

/**
 * @brief   Process audio.
 *
 * @param[in]   in  Pointer to input buffer.
 * @param[out]  out Pointer to input buffer.
 */
void module_process(fract32 *in, fract32 *out) {

    fract32 output;

    output = MonoSynth_tick(&g_voice);

    // Set output.
    out[0] = output;
    out[1] = output;
}

/**
 * @brief   Set parameter.
 *
 * @param[in]   param_index Index of parameter to set.
 * @param[in]   value       Value of parameter.
 */
void module_set_param(uint16_t param_index, int32_t value) {

    switch (param_index) {

    case PARAM_FREQ:
        g_voice.freq = value;
        break;

    case PARAM_TUNE:
        g_voice.tune = value;
        break;

    case PARAM_AMP_LEVEL:
        g_voice.amp_level = value;
        break;

    case PARAM_VEL:
        if (value) {
            g_voice.vel = value;
        }
        break;

    case PARAM_GATE:
        if (value) {
            /// TODO: LFO phase reset parameter.
            // g_voice.amp_lfo.phase = 0;
            // g_voice.filter_lfo.phase = 0;
            // g_voice.pitch_lfo.phase = 0;

            EnvADSR_press(&g_voice.amp_env);
            EnvADSR_press(&g_voice.filter_env);
            EnvADSR_press(&g_voice.pitch_env);
        } else {
            EnvADSR_release(&g_voice.amp_env);
            EnvADSR_release(&g_voice.filter_env);
            EnvADSR_release(&g_voice.pitch_env);
        }
        break;

    case PARAM_AMP_ENV_ATTACK:
        EnvADSR_set_attack(&g_voice.amp_env, value);
        break;

    case PARAM_AMP_ENV_DECAY:
        EnvADSR_set_decay(&g_voice.amp_env, value);
        break;

    case PARAM_AMP_ENV_SUSTAIN:
        EnvADSR_set_sustain(&g_voice.amp_env, value);
        break;

    case PARAM_AMP_ENV_RELEASE:
        EnvADSR_set_release(&g_voice.amp_env, value);
        break;

    case PARAM_FILTER_ENV_ATTACK:
        EnvADSR_set_attack(&g_voice.filter_env, value);
        break;

    case PARAM_FILTER_ENV_DECAY:
        EnvADSR_set_decay(&g_voice.filter_env, value);
        break;

    case PARAM_FILTER_ENV_SUSTAIN:
        EnvADSR_set_sustain(&g_voice.filter_env, value);
        break;

    case PARAM_FILTER_ENV_RELEASE:
        EnvADSR_set_release(&g_voice.filter_env, value);
        break;

    case PARAM_FILTER_ENV_DEPTH:
        g_voice.filter_env_depth = value;
        break;

    case PARAM_PITCH_ENV_ATTACK:
        EnvADSR_set_attack(&g_voice.pitch_env, value);
        break;

    case PARAM_PITCH_ENV_DECAY:
        EnvADSR_set_decay(&g_voice.pitch_env, value);
        break;

    case PARAM_PITCH_ENV_SUSTAIN:
        EnvADSR_set_sustain(&g_voice.pitch_env, value);
        break;

    case PARAM_PITCH_ENV_RELEASE:
        EnvADSR_set_release(&g_voice.pitch_env, value);
        break;

    case PARAM_PITCH_ENV_DEPTH:
        g_voice.pitch_env_depth = value;
        break;

    case PARAM_CUTOFF:
        g_voice.cutoff = value;
        break;

    case PARAM_RES:
        g_voice.res = value;
        break;

    case PARAM_OSC_TYPE:
        g_voice.osc_type = value;
        break;

    case PARAM_FILTER_TYPE:
        g_voice.filter_type = value;
        break;

    case PARAM_AMP_LFO_DEPTH:
        g_voice.amp_lfo_depth = value;
        break;
    //
    case PARAM_AMP_LFO_SPEED:
        Oscillator_set_freq(&g_voice.amp_lfo, value);
        break;

    case PARAM_FILTER_LFO_DEPTH:
        g_voice.filter_lfo_depth = value;
        break;

    case PARAM_FILTER_LFO_SPEED:
        Oscillator_set_freq(&g_voice.filter_lfo, value);
        break;

    case PARAM_PITCH_LFO_DEPTH:
        g_voice.pitch_lfo_depth = value;
        break;

    case PARAM_PITCH_LFO_SPEED:
        Oscillator_set_freq(&g_voice.pitch_lfo, value);
        break;

    default:
        break;
    }
}

/**
 * @brief   Get parameter.
 *
 * @param[in]   param_index Index of parameter to get.
 *
 * @return      value       Value of parameter.
 */
int32_t module_get_param(uint16_t param_index) {

    int32_t value = 0;

    switch (param_index) {

    default:
        break;
    }

    return value;
}

/**
 * @brief   Get number of parameters.
 *
 * @return  Number of parameters
 */
uint32_t module_get_param_count(void) { return PARAM_COUNT; }

/**
 * @brief   Get name of parameter at index.
 *
 * @param[in]   param_index     Index pf parameter.
 * @param[out]  text            Buffer to store string.
 *                              Must provide 'MAX_PARAM_NAME_LENGTH'
 *                              bytes of storage.
 */
void module_get_param_name(uint16_t param_index, char *text) {

    switch (param_index) {

    default:
        copy_string(text, "Unknown", MAX_PARAM_NAME_LENGTH);
        break;
    }
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
