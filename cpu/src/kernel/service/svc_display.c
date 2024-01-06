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
 * @file    svc_display.c
 *
 * @brief   Configuration and handling for LCD.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "dev_lcd.h"

#include "ft_error.h"
#include "svc_delay.h"
#include "svc_display.h"

/*----- Macros and Definitions ---------------------------------------*/

#define FRAME_BUF_LEN 0x400
#define LCD_COLUMNS 0x80

typedef enum {
    STATE_ASSERT_RESET,
    STATE_RELEASE_RESET,
    STATE_INIT,
    STATE_RUN,
    STATE_ERROR
} t_display_task_state;

/*----- Static variable definitions ----------------------------------*/

static uint8_t g_frame_buffer[FRAME_BUF_LEN];

/*----- Extern variable definitions ----------------------------------*/

static t_status _display_init(void);

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void svc_display_task(void) {

    static t_display_task_state state = STATE_ASSERT_RESET;
    static uint32_t start_time;

    switch (state) {

    case STATE_ASSERT_RESET:

        dev_lcd_reset(true);

        start_time = delay_get_current_count();
        state = STATE_RELEASE_RESET;
        break;

    case STATE_RELEASE_RESET:
        // Hold in reset for 5 us.
        if (delay_us(start_time, 5)) {
            dev_lcd_reset(false);

            start_time = delay_get_current_count();
            state = STATE_INIT;
        }
        break;

    // Initialise LCD task.
    case STATE_INIT:
        // Wait 5 us after reset released.
        if (delay_us(start_time, 5)) {

            if (error_check(_display_init()) == SUCCESS) {
                state = STATE_RUN;
            }
        } // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:
        // TODO: Implement ping-pong frame buffer.

        // Full frame starves system if polling.
        //      Seems OK when interrupt driven.
        dev_lcd_set_frame(g_frame_buffer);

        // Send single page per task invocation.
        //      Implement ping-pong buffer.
        /* page_buffer = g_frame_buffer + page_index * LCD_COLUMNS; */
        /* dev_lcd_set_page(page_index++, page_buffer); */

        /* if (page_index >= 8) { */
        /*     page_index = 0; */
        /* } */
        break;

    case STATE_ERROR:
        error_check(UNRECOVERABLE_ERROR);
        break;

    default:
        // TODO: Record unhandled state.
        if (error_check(UNHANDLED_STATE_ERROR) != SUCCESS) {
            state = STATE_ERROR;
        }
        break;
    }
}

void svc_display_put_pixel(uint16_t pos_x, uint16_t pos_y, bool state) {

    // Calculate pixel location in buffer.
    uint16_t column_index = pos_x;
    uint16_t page_index = pos_y >> 3;
    uint16_t bit_index = pos_y & 7;

    uint16_t byte_index = column_index + (128 * page_index);

    // Set pixel to state
    uint8_t byte = g_frame_buffer[byte_index];

    g_frame_buffer[byte_index] =
        (byte & ~(1UL << bit_index)) | (state << bit_index);
}

void svc_display_set_contrast(uint8_t contrast) {

    dev_lcd_set_contrast(contrast);
}

/*----- Static function implementations ------------------------------*/

static t_status _display_init(void) {

    dev_lcd_init();
    // TODO: frame_buffer_init();

    /* for (int i = 0; i < FRAME_BUF_LEN; i++) { */
    /*     g_frame_buffer[i] = 0xff; */
    /* } */

    return SUCCESS;
}

/*----- End of file --------------------------------------------------*/
