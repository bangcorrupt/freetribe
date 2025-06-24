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

#include <stdint.h>

#include "module.h"
#include "types.h"
#include "utils.h"

#include "aleph.h"

#include "common/config.h"
#include "common/params.h"
#include "custom_aleph_monovoice.h"

void module_set_param_voice(uint16_t voice_index, uint16_t param_index,
                            int32_t value);

/*----- Macros -------------------------------------------------------*/

#define MEMPOOL_SIZE (0x1000)

/// TODO: Struct for parameter type.
///         scaler,
///         range,
///         default,
///         display,
///         etc...,

// #define DEFAULT_CUTOFF 0x7f // Index in pitch LUT.

/// TODO: Move to common location.
/**
 * @brief   Enumeration of module parameters.
 *
 * Index of each external parameter of module.
 */

/*----- Typedefs -----------------------------------------------------*/

typedef struct {

    Custom_Aleph_MonoVoice voice[MAX_VOICES];
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

    int i;
    for (i = 0; i < MAX_VOICES; i++) {
        Custom_Aleph_MonoVoice_init(&g_module.voice[i], &g_aleph);

        module_set_param_voice(i, PARAM_AMP_LEVEL, FR32_MAX);
        module_set_param_voice(i, PARAM_OSC_TYPE, 2);
        module_set_param_voice(i, PARAM_OSC_2_TYPE, 2);
        module_set_param_voice(i, PARAM_FREQ, 220 << 16);
        module_set_param_voice(i, PARAM_TUNE, FIX16_ONE);
        module_set_param_voice(i, PARAM_CUTOFF, 0x326f6abb);
        module_set_param_voice(i, PARAM_RES, FR32_MAX);
    }
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

    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {

        // fract32 amp_level_scaled = ;///MAX_VOICES; // already scaled in _next

        outl[i] =
            mult_fr1x32x32(Custom_Aleph_MonoVoice_next(&g_module.voice[0]),
                           g_module.amp_level);
        int j;
        for (j = 1; j < MAX_VOICES; j++) {
            outl[i] = add_fr1x32(
                outl[i],
                mult_fr1x32x32(Custom_Aleph_MonoVoice_next(&g_module.voice[j]),
                               g_module.amp_level));
            // outl[i] +=
            // mult_fr1x32x32(Custom_Aleph_MonoVoice_next(&g_module.voice[j]),
            // g_module.amp_level);
        }

// if global filter, use the first voice's filter.
#ifdef VOICE_MODE_PARAPHONIC
        outl[i] =
            Custom_Aleph_MonoVoice_apply_filter(&g_module.voice[0], outl[i]);
#endif

        // Set output.
        outr[i] = outl[i];
    }
}

void module_set_param_voice(uint16_t voice_index, uint16_t param_index,
                            int32_t value) {
    uint16_t paramWithOffset =
        PARAM_VOICE_OFFSET(voice_index, param_index, PARAM_COUNT);
    module_set_param(paramWithOffset, value);
}

/**
 * @brief   Set parameter.
 *
 * @param[in]   param_index Index of parameter to set.
 * @param[in]   value       Value of parameter.
 */
void module_set_param(uint16_t param_index_with_offset, int32_t value) {

    uint16_t param_index =
        REMOVE_PARAM_OFFSET(param_index_with_offset, PARAM_COUNT);
    int voice_number = PARAM_VOICE_NUMBER(param_index_with_offset, PARAM_COUNT);
    // voice_number = 1;

    switch (param_index) {

    case PARAM_AMP:
        Custom_Aleph_MonoVoice_set_amp(&g_module.voice[voice_number], value);
        break;

    case PARAM_FREQ:
        Custom_Aleph_MonoVoice_set_freq(&g_module.voice[voice_number], value);
        break;

    case PARAM_OSC_PHASE:
        Custom_Aleph_MonoVoice_set_phase(&g_module.voice[voice_number], value);
        break;

    case PARAM_TUNE:
        Custom_Aleph_MonoVoice_set_freq_offset(&g_module.voice[voice_number],
                                               value);
        break;

    case PARAM_AMP_LEVEL:
        g_module.amp_level = value;
        break;

    case PARAM_CUTOFF:
        Custom_Aleph_MonoVoice_set_cutoff(&g_module.voice[voice_number], value);
        break;

    case PARAM_RES:
        Custom_Aleph_MonoVoice_set_res(&g_module.voice[voice_number], value);
        break;

    case PARAM_OSC_TYPE:
        Custom_Aleph_MonoVoice_set_shape_a(&g_module.voice[voice_number],
                                           value);
        break;

    case PARAM_OSC_2_TYPE:
        Custom_Aleph_MonoVoice_set_shape_b(&g_module.voice[voice_number],
                                           value);
        break;

    case PARAM_FILTER_TYPE:
        Custom_Aleph_MonoVoice_set_filter_type(&g_module.voice[voice_number],
                                               value);
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
