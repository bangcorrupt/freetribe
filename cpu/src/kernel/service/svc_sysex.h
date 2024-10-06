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
 * @file    svc_sysex.h
 *
 * @brief   Header for sysex.c.
 *
 */

#ifndef SVC_SYSEX_H
#define SVC_SYSEX_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    PARSE_MANU_ID,
    PARSE_GLOBAL_CHANNEL,
    PARSE_PRODUCT_ID,
    PARSE_MSG_BODY,
    SYSEX_PARSE_ERROR,
    SYSEX_PARSE_COMPLETE
} t_sysex_parse_result;

typedef enum {
    // CURRENT_PATTERN_REQUEST = 0x10,
    // PATTERN_REQUEST = 0x1c,
    // GLOBAL_REQUEST = 0x0e,
    // PATTERN_WRITE = 0x11,
    // CURRENT_PATTERN_DUMP = 0x40,
    // PATTERN_DUMP = 0x4c,
    // GLOBAL_DUMP = 0x51,
    READ_CPU_RAM = 0x52,
    SET_WRITE_ADDRESS = 0x53,
    WRITE_CPU_RAM = 0x54,
    READ_FLASH = 0x55,
    WRITE_FLASH = 0x56,
    // DATA_FORMAT_ERROR = 0x26,
    // DATA_LOAD_COMPLETE = 0x23,
    // DATA_LOAD_ERROR = 0x24,
    WRITE_COMPLETE = 0x21,
    WRITE_ERROR = 0x22,
} t_msg_id;

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

t_sysex_parse_result sysex_parse(uint8_t *msg, uint32_t length);

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
