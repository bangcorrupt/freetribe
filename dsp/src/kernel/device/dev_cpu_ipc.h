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
 * @brief   Public API for high-bandwidth communication with the CPU.
 */

#ifndef CPU_IPC_H
#define CPU_IPC_H

typedef enum {
    IPC_SUCCESS = 0,
    IPC_FAILED,
    IPC_INVALID_ARGUMENT,
    IPC_QUEUE_FULL,
    IPC_DRIVER_LOCKED
} t_ipc_status;

void dev_cpu_ipc_init();

void dev_cpu_ipc_tick();

/**
 * @brief   Queue a request to transfer data from the DSP to CPU memory.
 * 
 * @param   cpu_address   Address in the CPU's virtual memory map to write to.
 * @param   buffer        Pointer to source buffer of 32-bit words.
 * @param   count         Number of 32-bit values to write.
 * @param   callback      Function to call when failed or completed.
 * @param   user_ctx      User provided context for callback.
 *
 * @return  IPC_SUCCESS on success,
 *          IPC_INVALID_ARGUMENT if arguments are out of range,
 *          IPC_QUEUE_FULL if the request queue is full.
 */
t_ipc_status dev_cpu_ipc_transfer(uint32_t cpu_address,
                                  const uint32_t *buffer,
                                  uint16_t count,
                                  void (*callback)(void *, t_ipc_status),
                                  void *user_ctx);

/**
 * @brief   Request a buffer of data from the CPU's memory.
 * 
 * @param   cpu_address   Address in the CPU's virtual memory map to read from.
 * @param   destination   Destination buffer to write requested data in.   
 * @param   count         Number of 32-bit values to request.
 * @param   callback      Function called on failure or completion.
 * @param   user_ctx      User provided context for callback function.
 * 
 * @return  IPC_SUCCESS          - transfer started or queued
 *          IPC_INVALID_ARGUMENT - arguments are out of range
 *          IPC_QUEUE_FULL       - the request queue is full
 *          IPC_DRIVER_LOCKED    - the driver cannot accept any new requests
 */
t_ipc_status dev_cpu_ipc_request_data(uint32_t   cpu_address,
                                      uint32_t  *destination,
                                      uint16_t   count,
                                      void     (*callback)(void*, t_ipc_status),
                                      void      *user_ctx);

#endif