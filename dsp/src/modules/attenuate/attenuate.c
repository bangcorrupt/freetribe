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

/* Original work by monome, modified by bangcorrupt 2023 */

/**
 * @file    attenuate.c
 *
 * @brief   A basic example module.
 *          Applies attenuation with slew to each input signal.
 */

/*----- Includes -----------------------------------------------------*/

#include "fract_math.h"

#include "module.h"
#include "utils.h"

#include "aleph.h"
#include "aleph_lpf_one_pole.h"

/*----- Macros -------------------------------------------------------*/

#define MEMPOOL_SIZE (0x2000)

#define PARAM_AMP_MAX 0x7fffffff      ///< Maximum amplitude scale value.
#define PARAM_SLEW_DEFAULT 0x00010000 ///< Default parameter slew time.

/*----- Typedefs -----------------------------------------------------*/

/**
 * @brief   Enumeration of module paramters.
 *
 * Index of each external parameter of module.
 *
 */
typedef enum {

    PARAM_LEVEL0, ///< Attenuation multiplier channel 0.
    PARAM_LEVEL1, ///< Attenuation multiplier channel 1.

    PARAM_COUNT ///< Should remain last to return number of paramters.
} e_param;

typedef struct {

    /// Input parameters for amplitude scaling.
    fract32 level[2];

    /// Parameter slew filters.
    Aleph_LPFOnePole level_slew[2];

} t_module;

/*----- Static variable definitions ----------------------------------*/

static t_Aleph g_aleph;

__attribute__((section(".l1.data.a")))
__attribute__((aligned(32))) static char g_mempool[MEMPOOL_SIZE];

static t_module g_module;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise module.
 *
 * Initialise parameters and slew filters.
 */
void module_init(void) {

    Aleph_init(&g_aleph, SAMPLERATE, g_mempool, MEMPOOL_SIZE, NULL);

    // Initialise 1pole filters for input attenuation slew.
    Aleph_LPFOnePole_init(&g_module.level_slew[0], &g_aleph);
    Aleph_LPFOnePole_init(&g_module.level_slew[1], &g_aleph);

    // Set slew defaults.
    Aleph_LPFOnePole_set_coeff(&g_module.level_slew[0], PARAM_SLEW_DEFAULT);
    Aleph_LPFOnePole_set_coeff(&g_module.level_slew[1], PARAM_SLEW_DEFAULT);

    // Set amp to -12db.
    module_set_param(PARAM_LEVEL0, PARAM_AMP_MAX >> 2);
    module_set_param(PARAM_LEVEL1, PARAM_AMP_MAX >> 2);
}

/**
 * @brief   Process audio.
 *
 * Update slewed parameters and scale audio amplitude.
 *
 * @param[in]   in  Pointer to input buffer.
 * @param[out]  out Pointer to input buffer.
 */
void module_process(fract32 *in, fract32 *out) {

    // Process parameter slew.
    g_module.level[0] = Aleph_LPFOnePole_next(&g_module.level_slew[0]);
    g_module.level[1] = Aleph_LPFOnePole_next(&g_module.level_slew[1]);

    // Process audio samples.
    out[0] = mult_fr1x32x32(in[0], g_module.level[0]);
    out[1] = mult_fr1x32x32(in[1], g_module.level[1]);
}

/**
 * @brief   Set parameter.
 *
 * Set input to parameter slew filters.
 *
 * @param[in]   param_index Index of parameter to set.
 * @param[in]   vallue      Value of parameter.
 */
void module_set_param(uint16_t param_index, int32_t value) {

    switch (param_index) {

    // Input attenuation values
    case PARAM_LEVEL0:

        value = value < PARAM_AMP_MAX ? value : PARAM_AMP_MAX;
        // Add value to slew filter to avoid stepping.
        Aleph_LPFOnePole_set_target(&g_module.level_slew[0], value);
        break;

    case PARAM_LEVEL1:

        value = value < PARAM_AMP_MAX ? value : PARAM_AMP_MAX;
        Aleph_LPFOnePole_set_target(&g_module.level_slew[1], value);
        break;

    default:
        break;
    }
}

/**
 * @brief   Get parameter.
 *
 * Return current slew value of parameter.
 *
 * @param[in]   param_index Index of parameter to get.
 *
 * @return      value       Value of parameter.
 */
int32_t module_get_param(uint16_t param_index) {

    int32_t value = 0;

    switch (param_index) {

    // Input attenuation values
    case PARAM_LEVEL0:
        value = g_module.level[0];

    case PARAM_LEVEL1:
        value = g_module.level[1];

    default:
        break;
    }

    return value;
}

/// TODO: Return uint16_t?
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

    // Input attenuation values
    case PARAM_LEVEL0:
        copy_string(text, "Level 0", MAX_PARAM_NAME_LENGTH);

    case PARAM_LEVEL1:
        copy_string(text, "Level 1", MAX_PARAM_NAME_LENGTH);

    default:
        break;
    }
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
