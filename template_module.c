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
 * @file    template_module.c
 *
 * @brief   Template for DSP module source files.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include "fract_math.h"
#include "types.h"

#include "module.h"

#include "utils.h"

/*----- Macros and Definitions ---------------------------------------*/

// Enumerate parameters.
enum params {

    // Number of parameters.
    PARAM_COUNT
};

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise module.
 */
void module_init(void) {
    
    //
}

/**
 * @brief   Process audio.   
 *
 * @param[in]   in  Pointer to input buffer.
 * @param[out]  out Pointer to input buffer.
 */
void module_process(fract32 *in, fract32 *out) {
// void module_process(fract32 **inputs, fract32 **outputs, uint32_t frames) {

}

/**
 * @brief   Set parameter.   
 *
 * @param[in]   param_index Index of parameter to set. 
 * @param[in]   vallue      Value of parameter. 
 */
void module_set_param(uint16_t param_index, int32_t value) {

    switch (param_index) {

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
