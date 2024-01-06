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
 * @file    module.h
 *
 * @brief   Public API for audio processing module.
 *
 */

#ifndef BFIN_AUDIO_MODULE_H
#define BFIN_AUDIO_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include "types.h"

/*----- Macros and Definitions ---------------------------------------*/

#ifndef SAMPLERATE
#define SAMPLERATE 48000
#endif

#define MAX_PARAM_NAME_LENGTH 16

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void module_init(void);
void module_process(fract32 *in, fract32 *out);
void module_set_param(uint16_t index, int32_t value);
int32_t module_get_param(uint16_t index);
uint32_t module_get_param_count(void); // TODO: return uint16_t ?
void module_get_param_name(uint16_t param_index, char *text);

#ifdef __cplusplus
}
#endif
#endif /* BFIN_AUDIO_MODULE_H */

/*----- End of file --------------------------------------------------*/
