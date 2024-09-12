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
 * @file    error.c
 *
 * @brief   Error handling.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>

#include "ft_error.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/// TODO: Count recoverable errors.

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _failed(void);

/*----- Extern function implementations ------------------------------*/

/// TODO: This is jank.
/// TODO: Refactor for debug string.

/*
 * @brief Log errors and attempt to recover.
 */
t_status error_check(t_status error) {

    static t_status result = ERROR;

    switch (error) {

    case SUCCESS:
        result = SUCCESS;
        break;

    case ERROR:
        result = ERROR;
        _failed();
        break;

    case WARNING:
        result = WARNING;
        break;

    case UNRECOVERABLE_ERROR:
        result = ERROR;
        _failed();
        break;

    case TASK_INIT_ERROR:
        /// TODO: Log error.
        result = ERROR;
        break;

    case UNHANDLED_STATE_ERROR:
        result = ERROR;
        break;

    case RING_BUFFER_INIT_ERROR:
        result = ERROR;
        break;

    case RING_BUFFER_PUT_ERROR:
        // Ring buffer full, handle gracefully.
        result = WARNING;
        break;

    case RING_BUFFER_GET_ERROR:
        // Not usually an error if data not available.
        result = WARNING;
        break;

    case PANEL_PARSE_ERROR:
        result = WARNING;
        break;

    default:
        result = ERROR;
        break;
    }

    return result;
}

/*----- Static function implementations ------------------------------*/

static void _failed(void) {
    while (true)
        ;
}

/*----- End of file --------------------------------------------------*/
