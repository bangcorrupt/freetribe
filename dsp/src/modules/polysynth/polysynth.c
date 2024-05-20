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
 * @file    polysynth.c
 *
 * @brief   A polyphonic synth module for Freetribe.
 */

/*----- Includes -----------------------------------------------------*/

#include "aleph.h"

#include "aleph_polysynth.h"

#include "module.h"

#include "utils.h"

// #include "polysynth.h"

/*----- Macros and Definitions ---------------------------------------*/

#define SAMPLERATE 48000
#define MEMPOOL_SIZE 0x4000

#define POLYSYNTH_NUM_VOICES 16

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
    PARAM_VOICE_INDEX,
    PARAM_FREQ,
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

typedef struct {

    Aleph_PolySynth synth;
    uint8_t voice_index;
    fract32 amp_level;
    fract32 velocity;

} t_module;

/*----- Static variable definitions ----------------------------------*/

static t_Aleph g_aleph;
static char g_mempool[MEMPOOL_SIZE];

static t_module g_module;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise module.
 */
void module_init(void) {

    Aleph_init(&g_aleph, SAMPLERATE, g_mempool, MEMPOOL_SIZE, NULL);

    Aleph_PolySynth_init(&g_module.synth, &g_aleph, POLYSYNTH_NUM_VOICES);

    /// TODO: Define defaults.
    //

    module_set_param(PARAM_AMP_LEVEL, FR32_MAX);
    module_set_param(PARAM_OSC_TYPE, 2);
    module_set_param(PARAM_FREQ, 220 << 16);
    module_set_param(PARAM_TUNE, FIX16_ONE);
    module_set_param(PARAM_CUTOFF, 0x326f6abb);
    module_set_param(PARAM_RES, FR32_MAX);

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

    output = Aleph_PolySynth_next(&g_module.synth);

    // Scale amplitude by level.
    output = mult_fr1x32x32(output, g_module.amp_level);

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

    int i;

    switch (param_index) {

    case PARAM_VOICE_INDEX:
        g_module.voice_index = value;

    case PARAM_FREQ:
        Aleph_PolySynth_set_voice_freq(&g_module.synth, g_module.voice_index,
                                       value);
        break;

    case PARAM_TUNE:
        Aleph_PolySynth_set_freq_offset(&g_module.synth, value);
        break;

    case PARAM_AMP_LEVEL:
        g_module.amp_level = value;
        break;

        /// TODO: Envelope depth modulation input.
        //
        // case PARAM_VEL:
        //     if (value) {
        //         g_velocity = value;
        //         Aleph_PolySynth_set_amp_env_depth(
        //             &g_synth[i], mult_fr1x32x32(g_synth[i]->amp_env_depth,
        //             g_velocity));
        //     }
        //     break;

    case PARAM_GATE:
        Aleph_PolySynth_set_voice_gate(&g_module.synth, g_module.voice_index,
                                       value);
        break;

    case PARAM_AMP_ENV_ATTACK:
        for (i = 0; i < POLYSYNTH_NUM_VOICES; i++) {
            Aleph_PolySynth_set_amp_env_attack(&g_module.synth, value);
        }
        break;

    case PARAM_AMP_ENV_DECAY:
        Aleph_PolySynth_set_amp_env_decay(&g_module.synth, value);
        break;

    case PARAM_AMP_ENV_SUSTAIN:
        Aleph_PolySynth_set_amp_env_sustain(&g_module.synth, value);
        break;

    case PARAM_AMP_ENV_RELEASE:
        Aleph_PolySynth_set_amp_env_release(&g_module.synth, value);
        break;

    case PARAM_AMP_ENV_DEPTH:
        Aleph_PolySynth_set_amp_env_depth(&g_module.synth, value);
        // Aleph_PolySynth_set_amp_env_depth(
        //     &g_module.synth, mult_fr1x32x32(g_module.synth->amp_env_depth,
        //     g_velocity));
        break;

    case PARAM_FILTER_ENV_ATTACK:
        Aleph_PolySynth_set_filter_env_attack(&g_module.synth, value);
        break;

    case PARAM_FILTER_ENV_DECAY:
        Aleph_PolySynth_set_filter_env_decay(&g_module.synth, value);
        break;

    case PARAM_FILTER_ENV_SUSTAIN:
        Aleph_PolySynth_set_filter_env_sustain(&g_module.synth, value);
        break;

    case PARAM_FILTER_ENV_RELEASE:
        Aleph_PolySynth_set_filter_env_release(&g_module.synth, value);
        break;

    case PARAM_FILTER_ENV_DEPTH:
        Aleph_PolySynth_set_filter_env_depth(&g_module.synth, value);
        break;

    case PARAM_PITCH_ENV_ATTACK:
        Aleph_PolySynth_set_pitch_env_attack(&g_module.synth, value);
        break;

    case PARAM_PITCH_ENV_DECAY:
        Aleph_PolySynth_set_pitch_env_decay(&g_module.synth, value);
        break;

    case PARAM_PITCH_ENV_SUSTAIN:
        Aleph_PolySynth_set_pitch_env_sustain(&g_module.synth, value);
        break;

    case PARAM_PITCH_ENV_RELEASE:
        Aleph_PolySynth_set_pitch_env_release(&g_module.synth, value);
        break;

    case PARAM_PITCH_ENV_DEPTH:
        Aleph_PolySynth_set_pitch_env_depth(&g_module.synth, value);
        break;

    case PARAM_CUTOFF:
        Aleph_PolySynth_set_cutoff(&g_module.synth, value);
        break;

    case PARAM_RES:
        Aleph_PolySynth_set_res(&g_module.synth, value);
        break;

    case PARAM_OSC_TYPE:
        Aleph_PolySynth_set_shape(&g_module.synth, value);
        break;

    case PARAM_FILTER_TYPE:
        Aleph_PolySynth_set_filter_type(&g_module.synth, value);
        break;

    case PARAM_AMP_LFO_DEPTH:
        Aleph_PolySynth_set_amp_lfo_depth(&g_module.synth, value);
        break;
    //
    case PARAM_AMP_LFO_SPEED:
        Aleph_PolySynth_set_amp_lfo_freq(&g_module.synth, value);
        break;

    case PARAM_FILTER_LFO_DEPTH:
        Aleph_PolySynth_set_filter_lfo_depth(&g_module.synth, value);
        break;

    case PARAM_FILTER_LFO_SPEED:
        Aleph_PolySynth_set_filter_lfo_freq(&g_module.synth, value);
        break;

    case PARAM_PITCH_LFO_DEPTH:
        Aleph_PolySynth_set_pitch_lfo_depth(&g_module.synth, value);
        break;

    case PARAM_PITCH_LFO_SPEED:
        Aleph_PolySynth_set_pitch_lfo_freq(&g_module.synth, value);
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