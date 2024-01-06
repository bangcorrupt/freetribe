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
 * @file    fix.h
 *
 * @brief   Some extra utilities for fixed point formatting
 *
 */

#ifndef BFIN_FIX_H
#define BFIN_FIX_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include "fix16.h"
#include "types.h"

/*----- Macros and Definitions ---------------------------------------*/

#define FIX_DIG_LO 5
#define FIX_DIG_HI 5
#define BIT_INVERT(x) ((s32)(((x) ^ 0xffffffff) + 1))
#define BIT_SIGN(x) ((x)&0x80000000)

// constants
#define FR32_MAX 0x7fffffff
#define FR32_MIN 0x80000000
#define FR16_MAX 0x7fff
#define FR16_MIN 0x8000

#define FIX16_ONE 0x00010000

// FIXME: these are so awful...
#define BIT_SIGN_32(x) ((x)&0x80000000)
#define BIT_INVERT_32(x) ((s32)(((x) ^ 0xffffffff) + 1))
#define BIT_ABS_32(x) (BIT_SIGN_32(x) ? BIT_INVERT_32(x) : (x))
#define BIT_NEG_ABS_32(x) (BIT_SIGN_32(x) ? (x) : BIT_INVERT_32(x))

#define BIT_SIGN_16(x) ((x)&0x8000)
#define BIT_INVERT_16(x) ((s16)(((x) ^ 0xffff) + 1))
#define BIT_ABS_16(x) (BIT_SIGN_16(x) ? BIT_INVERT_16(x) : (x))
#define BIT_NEG_ABS_16(x) (BIT_SIGN_16(x) ? (x) : BIT_INVERT_16(x))

// macros for fract32/fix16 conversion
#define FIX16_TO_U16(x) (u16)((x) >> 16)
#define FIX16_TO_S16(x) (s16)((x) >> 16)
#define S16_TO_FIX16(x) ((fix16_t)(x)) << 1

#define U16_TO_FIX16(x) ((fix16_t)(x)) << 16
// #define FIX16_FRACT_TRUNC(x) (fract32)( (( (x) & 0xffff) << 15) - (
// BIT_SIGN_32(x) ? 1 : 0) )
#define FIX16_FRACT_TRUNC(x) (shl_fr1x32((x), 16))
#define FIX16_FRACT(x) FIX16_FRACT_TRUNC(x)
// #define FRACT_FIX16(x) ( BIT_SIGN_32(x) ? ((x)>>15) | 0xffff0000 : (x)>>15 )
#define FRACT_FIX16(x) shr_fr1x32((x), 15)

#define fix16_add(x, y) add_fr1x32((x), (y))
#define fix16_sub(x, y) sub_fr1x32((x), (y))

#define fix16_sadd(x, y) add_fr1x32((x), (y))
#define fix16_ssub(x, y) sub_fr1x32((x), (y))

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
