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

/*
 * @file    svc_dsp.h
 *
 * @brief   Public API for communicating with DSP.
 *
 */

#ifndef SVC_DSP_H
#define SVC_DSP_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>

#include "ft_error.h"

/*----- Macros and Definitions ---------------------------------------*/

// TODO: Move protocol definition to common module.
//       Union struct for message.
enum e_message_type { MSG_TYPE_MODULE, MSG_TYPE_SYSTEM };

enum e_module_msg_id {
    MODULE_GET_PARAM_VALUE,
    MODULE_SET_PARAM_VALUE,
    MODULE_PARAM_VALUE,
    MODULE_GET_PARAM_NAME,
    MODULE_PARAM_NAME
};

enum e_system_msg_id {
    SYSTEM_GET_PORT_STATE,
    SYSTEM_SET_PORT_STATE,
    SYSTEM_PORT_STATE
};

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void svc_dsp_task(void);

void svc_dsp_register_callback(uint8_t msg_type, uint8_t msg_id,
                               void (*callback)(void));

void svc_dsp_set_module_param(uint16_t module_id, uint16_t param_index,
                              int32_t param_value);

void svc_dsp_get_module_param(uint8_t module_id, uint16_t param_index);

void svc_dsp_get_port_state(void);

#ifdef __cplusplus
}
#endif
#endif /* SVC_DSP_H */

/*----- End of file --------------------------------------------------*/
