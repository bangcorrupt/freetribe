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
 * @file    monosynth.c
 *
 * @brief   A monophonic synth module for Freetribe.
 */

/*----- Includes -----------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>

#include "module.h"
#include "per_sport.h"
#include "types.h"
#include "utils.h"

#include "aleph.h"

#include "aleph_monovoice.h"

#include "knl_profile.h"

/*----- Macros -------------------------------------------------------*/

#define MEMPOOL_SIZE (0x2000)

/// TODO: Struct for parameter type.
///         scaler,
///         range,
///         default,
///         display,
///         etc...,

// #define DEFAULT_CUTOFF 0x7f // Index in pitch LUT.
#define DEFAULT_OSC_TYPE 2

/// TODO: Move to common location.
/**
 * @brief   Enumeration of module parameters.
 *
 * Index of each external parameter of module.
 */

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    PARAM_AMP,
    PARAM_FREQ,
    PARAM_OSC_PHASE,
    PARAM_LFO_PHASE,
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
    PARAM_OSC_BASE_FREQ,
    PARAM_FILTER_BASE_CUTOFF,
    PARAM_PHASE_RESET,
    PARAM_RETRIGGER,

    PARAM_COUNT
} e_param;

typedef struct {

    Aleph_MonoVoice voice;
    fract32 amp_level;
    fract32 velocity;

} t_module;

/*----- Static variable definitions ----------------------------------*/

__attribute__((section(".l1.data.a")))
__attribute__((aligned(32))) static char g_mempool[MEMPOOL_SIZE];

static t_Aleph g_aleph;
static t_module g_module;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise module.
 */
void module_init(void) {

    Aleph_init(&g_aleph, SAMPLERATE, g_mempool, MEMPOOL_SIZE, NULL);

    Aleph_MonoVoice_init(&g_module.voice, &g_aleph);

    /// TODO: Define defaults.

    module_set_param(PARAM_AMP_LEVEL, FR32_MAX);
    module_set_param(PARAM_OSC_TYPE, 2);
    module_set_param(PARAM_FREQ, 220 << 16);
    module_set_param(PARAM_TUNE, FIX16_ONE);
    module_set_param(PARAM_CUTOFF, 0x326f6abb);
    module_set_param(PARAM_RES, FR32_MAX);
}

/**
 * @brief   Process audio.
 *
 * @param[in]   in  Pointer to input buffer.
 * @param[out]  out Pointer to input buffer.
 */
void module_process(fract32 *in, fract32 *out) {

    fract32 *outl = out;
    fract32 *outr = out + BLOCK_SIZE;

    Aleph_MonoVoice_next_block(&g_module.voice, outl, BLOCK_SIZE);

    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {

        // Scale amplitude by level.
        outl[i] = mult_fr1x32x32(outl[i], g_module.amp_level);

        // Set output.
        outr[i] = outl[i];
    }
}

/**
 * @brief   Set parameter.
 *
 * @param[in]   param_index Index of parameter to set.
 * @param[in]   value       Value of parameter.
 */
void module_set_param(uint16_t param_index, int32_t value) {

    switch (param_index) {

    case PARAM_AMP:
        Aleph_MonoVoice_set_amp(&g_module.voice, value);
        break;

    case PARAM_FREQ:
        Aleph_MonoVoice_set_freq(&g_module.voice, value);
        break;

    case PARAM_OSC_PHASE:
        Aleph_MonoVoice_set_phase(&g_module.voice, value);
        break;

    case PARAM_TUNE:
        Aleph_MonoVoice_set_freq_offset(&g_module.voice, value);
        break;

    case PARAM_AMP_LEVEL:
        g_module.amp_level = value;
        break;

    case PARAM_CUTOFF:
        Aleph_MonoVoice_set_cutoff(&g_module.voice, value);
        break;

    case PARAM_RES:
        Aleph_MonoVoice_set_res(&g_module.voice, value);
        break;

    case PARAM_OSC_TYPE:
        Aleph_MonoVoice_set_shape(&g_module.voice, value);
        break;

    case PARAM_FILTER_TYPE:
        Aleph_MonoVoice_set_filter_type(&g_module.voice, value);
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
