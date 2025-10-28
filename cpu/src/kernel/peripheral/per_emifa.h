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

/*----- Macros -------------------------------------------------------*/

#define HOST_TO_DSP_HEADER_BASE 0x00000000 // Host writes here into DSP RAM
#define HOST_TO_DSP_HEADER_LENGTH 16
#define DSP_TO_HOST_HEADER_BASE 0x00000020 // Host reads here from DSP RAM
#define DSP_TO_HOST_HEADER_LENGTH 16

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    EMIFA_SUCCESS = 0,
    EMIFA_UNINITIALISED,
    EMIFA_BUS_OCCUPIED,
    EMIFA_INVALID_PARAMETER,
} t_emifa_status;

typedef struct {
    uint32_t meta0;
    uint32_t meta1;
    uint32_t meta2;
    uint32_t meta3;
    uint32_t meta4;
} t_emifa_metadata;

typedef void (*t_emifa_idle_callback)();
typedef void (*t_emifa_tx_callback)();
typedef void (*t_emifa_rx_callback)(t_emifa_metadata);
typedef void (*t_emifa_error_callback)(t_emifa_metadata);

/*----- Extern function prototypes -----------------------------------*/

/**
 * @brief   Sets up the EMIFA engine.
 * 
 * @param   idle_callback   Fires when state machine is idle.
 * @param   tx_callback     Fires once a transfer completes.
 * @param   rx_callback     Fires when we received data from the DSP.
 * @param   error_callback  Fires when a DMA error was encountered and
 *                          the active transaction had been flushed.
 */
void per_emifa_init(t_emifa_idle_callback idle_callback,
                    t_emifa_tx_callback tx_callback,
                    t_emifa_rx_callback rx_callback,
                    t_emifa_error_callback error_callback);

/**
 * @brief   Update tick of the EMIFA engine. During a transfer, nothing
 *          is able to come through.
 */
void per_emifa_task();

/**
 * @brief   Perform a non-blocking transfer.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to write to.
 * @param   words         Pointer to the buffer of 16-bit words to write
 * @param   word_count    Number of 16-bit words to write. Cannot be 1.
 * @param   metadata      Gets sent along through the header.
 */
t_emifa_status per_emifa_transfer(uint32_t dsp_address,
                                  const uint16_t *words,
                                  uint16_t word_count,
                                  t_emifa_metadata metadata);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
