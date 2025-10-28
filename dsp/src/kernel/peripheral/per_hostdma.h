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
 * @file    per_hostdma.h
 * @brief   Public API for HostDMA peripheral driver.
 * 
 * @details HostDMA communicates with the CPU's EMIFA peripheral, which
 *          offers maximum bandwidth in CPU-DSP data transmission.
 */

#ifndef PER_HOSTDMA_H
#define PER_HOSTDMA_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/*----- Macros -------------------------------------------------------*/

#define HOST_TO_DSP_HEADER_BASE 0x00000000 // host writes here into DSP RAM
#define DSP_TO_HOST_HEADER_BASE 0x00000020 // host reads here from DSP RAM

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    HOSTDMA_SUCCESS = 0,
    HOSTDMA_UNINITIALISED,
    HOSTDMA_BUS_OCCUPIED,
} t_hostdma_status;

typedef struct {
    uint32_t meta0;
    uint32_t meta1;
    uint32_t meta2;
    uint32_t meta3;
    uint32_t meta4;
} t_hostdma_metadata;

typedef void (*t_hostdma_cb)(t_hostdma_metadata);

/*----- Extern function prototypes -----------------------------------*/

/**
 * @brief   Initializes HostDMA peripheral.
 * 
 * @note    Registers to IVG 10, 12, and 13
 */
void per_hostdma_init(t_hostdma_cb rx_callback,
                      t_hostdma_cb tx_callback,
                      t_hostdma_cb error_callback);

/**
 * @brief   Polls event queue to execute callbacks deferred from ISRs.
 */
void per_hostdma_process_events();

/**
 * @brief   Requests to transfer a block of data to the host.
 * 
 * @param   host_address  Address in the host's virtual memory map to write to.
 * @param   words         Buffer of data.
 * @param   word_count    Number of words, which must be even.
 * @param   metadata      Gets sent along with the transfer inside the header.
 * 
 * @return  HOSTDMA_SUCCESS on success, otherwise an error code.
 */
t_hostdma_status per_hostdma_transfer(uint32_t host_address,
                                      const uint16_t *words,
                                      uint16_t word_count,
                                      t_hostdma_metadata metadata);

#ifdef __cplusplus
}
#endif
#endif
/*----- End of file --------------------------------------------------*/
