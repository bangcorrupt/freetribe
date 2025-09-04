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
 * @file    dev_cpu_ipc.h
 *
 * @brief   Public API for high-bandwidth communication with the DSP.
 */

#ifndef DEV_DSP_IPC_H
#define DEV_DSP_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    IPC_SUCCESS = 0,
    IPC_FAILED,
    IPC_INVALID_ARGUMENT,
    IPC_QUEUE_FULL,
    IPC_DRIVER_LOCKED,
} t_ipc_status;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

/**
 * @brief   Initialize high speed communication channel with the DSP.
 */
void dev_dsp_ipc_init();

/**
 * @brief   Request to transfer data to the DSP memory.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to write to.
 * @param   buffer        Pointer to the buffer of 32-bit values to write
 * @param   count         Number of 32-bit values to write.
 * @param   callback      Function to call when failed or completed.
 * @param   user_ctx      User provided context for callback.
 */
t_ipc_status dev_dsp_ipc_transfer(uint32_t dsp_address,
                                  const uint32_t* buffer,
                                  uint16_t count,
                                  void (*callback)(void *, t_ipc_status),
                                  void *user_ctx);

/**
 * @brief   Request a buffer of data from the DSP's memory.
 * 
 * @param   dsp_address   Address in the DSP's virtual memory map to read from.
 * @param   destination   Pointer to buffer of 32-bit values to write read memory from.
 * @param   count         Number of 32-bit values to request.
 * @param   callback      Function to call when failed or completed.
 * @param   user_ctx      User provided context for callback.
 */
t_ipc_status dev_dsp_ipc_read(uint32_t dsp_address,
                              uint32_t *destination,
                              uint16_t count,
                              void (*callback)(void *, t_ipc_status),
                              void *user_ctx);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
