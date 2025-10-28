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
 * @file    per_emifa.h
 *
 * @brief   Public API for EMIFA peripheral driver.
 *          EMIFA communicates with the DSP's HostDMA engine and has the
 *          highest bandwidth for transmitting data between CPU and DSP.
 */

#ifndef PER_EMIFA_H
#define PER_EMIFA_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    EMIFA_SUCCESS = 0,
    EMIFA_UNINITIALISED,
    EMIFA_BUS_OCCUPIED,
} t_emifa_status;

/*----- Extern function prototypes -----------------------------------*/

/**
 * @brief   Sets up the EMIFA engine.
 */
void per_emifa_init();

/**
 * @brief   Poll for incoming HostDMA requests. When the bus is occupied
 *          nothing is able to come through. When the HostDMA connection
 *          has not been initialised we will wait for it to do so.
 */
void per_emifa_poll();

/**
 * @brief   Perform a blocking transfer.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to write to.
 * @param   words         Pointer to the buffer of 16-bit words to write
 * @param   word_count    Number of 16-bit words to write. Cannot be 1.
 */
t_emifa_status per_emifa_transfer(uint32_t dsp_address, uint16_t *words, uint16_t word_count);

/**
 * @brief   Whenever the EMIFA bus is occupied it's impossible for any new
 *          transmissions to happen.
 * 
 * @return  True if EMIFA bus currently occupied transferring or receiving.
 */
bool per_emifa_is_bus_available();

#ifdef __cplusplus
}
#endif
#endif
/*----- End of file --------------------------------------------------*/
