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
 * @file    module.c
 *
 * @brief   Audio processing module.
 *          
 */

/*----- Includes -----------------------------------------------------*/

#include "module.h"

/*----- Macros -------------------------------------------------------*/

// Enumerate parameters.
enum params {
    // Number of parameters.
    PARAM_COUNT
};

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

__attribute__((weak)) void module_init(void) {
    //
}

__attribute__((weak)) void module_process(fract32 *in, fract32 *out) {
    //
}

__attribute__((weak)) void module_set_param(uint16_t param_index,
                                            int32_t value) {
    //
}

__attribute__((weak)) int32_t module_get_param(uint16_t param_index) {

    return 0;
}

// Get number of parameters
__attribute__((weak)) uint32_t module_get_param_count(void) {
    return PARAM_COUNT;
}

// Buffer 'text' must provide 'MAX_PARAM_NAME_LENGTH' bytes of storage.
__attribute__((weak)) void module_get_param_name(uint16_t param_index,
                                                 char *text) {
    //
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
