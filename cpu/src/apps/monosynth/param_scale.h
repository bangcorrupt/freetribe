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
 * @file    param_scale.h
 *
 * @brief   Public API for parameter scaling.
 */

#ifndef PARAM_SCALE_H
#define PARAM_SCALE_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <math.h>
#include <stdint.h>

#include "lookup_tables.h"

/*----- Macros -------------------------------------------------------*/

// Maximum and minimum float values representable as fract32.
#define FRACT32_MAX_FLOAT 0x0.FFFFFFp0F
#define FRACT32_MIN_FLOAT -1.0F

#define FIX16_MAX 0x7FFFFFFF
#define FIX16_MIN 0x80000000
#define FIX16_ONE 0x00010000

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

static inline float clamp_value(float value) {

    return fmaxf(fminf(value, FRACT32_MAX_FLOAT), FRACT32_MIN_FLOAT);
}

static inline int32_t float_to_fract32(float value) {

    int32_t result;

    if (value == 0) {
        result = 0;

    } else {
        value = clamp_value(value);

        result = (int32_t)roundf(scalbnf(value, 31));
    }
    return result;
}

static inline int32_t float_to_fix16(float value) {

    int32_t result;

    if (value == 0) {
        result = 0;

    } else {
        result = (int32_t)(value * FIX16_ONE);
    }

    return result;
}

static inline float note_to_freq(float note) {
    //
    return 440 * powf(2.0, ((note - 69) / 12.0));
}

static inline float freq_to_cv(float freq) {

    // 0.1 per octave, 0 == 27.5 Hz (A0).
    return (logf(freq / 27.5) / logf(2.0)) / 10;
}

static inline float note_to_cv(float note) {
    //
    return freq_to_cv(note_to_freq(note));
}

static inline float cv_to_freq(float cv) {
    //
    return powf(2.0, cv) * 27.5;
}

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
