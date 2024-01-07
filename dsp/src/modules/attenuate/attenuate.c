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

/*
 * @file    attenuate.c
 *
 * @brief   A very simple template module.
 *          Applies attenuation with slew to each input signal.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include "fract_math.h"
#include "types.h"

#include "module.h"

#include "filter_1p.h"

#include "utils.h"

/*----- Macros and Definitions ---------------------------------------*/

// Maximum positive fract32 value
#define PARAM_AMP_MAX 0x7fffffff
#define PARAM_SLEW_DEFAULT 0x00010000

// Enumerate parameters.
enum params {
    // Attenuation multiplier values.
    PARAM_LEVEL0,
    PARAM_LEVEL1,
    // Number of parameters.
    PARAM_COUNT
};

/*----- Static variable definitions ----------------------------------*/

// Input parameters.
static fract32 g_level[2];

// Parameter slew filters.
static filter_1p_lo g_level_slew[2];

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void module_init(void) {

    // Initialise 1pole filters for input attenuation slew.
    filter_1p_lo_init(&(g_level_slew[0]), 0);
    filter_1p_lo_init(&(g_level_slew[1]), 0);

    // Set amp to -12db.
    module_set_param(PARAM_LEVEL0, PARAM_AMP_MAX >> 2);
    module_set_param(PARAM_LEVEL1, PARAM_AMP_MAX >> 2);

    // Set slew defaults.
    filter_1p_lo_set_slew(&(g_level_slew[0]), PARAM_SLEW_DEFAULT);
    filter_1p_lo_set_slew(&(g_level_slew[1]), PARAM_SLEW_DEFAULT);
}

// Frame process function.
// void module_process(fract32 **inputs, fract32 **outputs, uint32_t frames) {
void module_process(fract32 *in, fract32 *out) {

    // Process parameter slew.
    g_level[0] = filter_1p_lo_next(&(g_level_slew[0]));
    g_level[1] = filter_1p_lo_next(&(g_level_slew[1]));

    // Process audio samples.
    out[0] = mult_fr1x32x32(in[0], g_level[0]);
    out[1] = mult_fr1x32x32(in[1], g_level[1]);
}

// Sets slew filter input if parameter smoothed.
void module_set_param(uint16_t param_index, int32_t value) {

    switch (param_index) {

    // Input attenuation values
    case PARAM_LEVEL0:
        value = value < PARAM_AMP_MAX ? value : PARAM_AMP_MAX;
        // Add value to slew filter to avoid stepping.
        filter_1p_lo_in(&(g_level_slew[0]), value);
        break;

    case PARAM_LEVEL1:
        value = value < PARAM_AMP_MAX ? value : PARAM_AMP_MAX;
        filter_1p_lo_in(&(g_level_slew[1]), value);
        break;

    default:
        break;
    }
}

// Returns current slew value if parameter smoothed.
int32_t module_get_param(uint16_t param_index) {

    switch (param_index) {

    // Input attenuation values
    case PARAM_LEVEL0:
        return g_level[0];

    case PARAM_LEVEL1:
        return g_level[1];

    default:
        break;
    }
    return 0;
}

// TODO: Return uint16_t?
//
// Get number of parameters.
uint32_t module_get_param_count(void) { return PARAM_COUNT; }

// Buffer 'text' must provide 'MAX_PARAM_NAME_LENGTH' bytes of storage.
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
