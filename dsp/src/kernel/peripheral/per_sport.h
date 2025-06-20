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
 * @file    per_sport.h
 *
 * @brief   Public API for BF523 SPORT peripheral driver.
 */

#ifndef PER_SPORT_H
#define PER_SPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include "types.h"
#include <stdbool.h>

/*----- Macros -------------------------------------------------------*/

#define DSP_CHANNEL_COUNT 2
#define DSP_BLOCK_SIZE 16
#define DSP_BUFFER_SIZE (DSP_BLOCK_SIZE * DSP_CHANNEL_COUNT)

/*----- Typedefs -----------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

/*----- Extern function prototypes -----------------------------------*/

void sport0_init(void);
bool sport0_frame_received(void);
void sport0_frame_processed(void);

fract32 *sport0_get_rx_buffer(void);
fract32 *sport0_get_tx_buffer(void);

#ifdef __cplusplus
}
#endif
#endif /* PER_SPORT_H */

/*----- End of file --------------------------------------------------*/
