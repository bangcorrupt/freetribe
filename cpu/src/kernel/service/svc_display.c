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
 * @file    svc_display.c
 *
 * @brief   Configuration and handling for LCD.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "dev_lcd.h"

#include "ft_error.h"

#include "svc_delay.h"
#include "svc_event.h"

#include "svc_display.h"

/*----- Macros -------------------------------------------------------*/

#define FRAME_BUF_LEN 0x400
#define LCD_COLUMNS 0x80

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    STATE_ASSERT_RESET,
    STATE_RELEASE_RESET,
    STATE_INIT,
    STATE_RUN,
    STATE_ERROR
} t_display_task_state;

/*----- Static variable definitions ----------------------------------*/

static uint8_t g_frame_buffer_a[FRAME_BUF_LEN];
static uint8_t g_frame_buffer_b[FRAME_BUF_LEN];

/*----- Extern variable definitions ----------------------------------*/

static t_status _display_init(void);

/*----- Static function prototypes -----------------------------------*/

static void _put_pixel_listener(const t_event *event);
static void _fill_frame_listener(const t_event *event);

static void _put_pixel(uint16_t pos_x, uint16_t pos_y, bool state);

static int8_t _fill_frame(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                          uint16_t y_end, bool state);

/*----- Extern function implementations ------------------------------*/

void svc_display_task(void) {

    static t_display_task_state state = STATE_ASSERT_RESET;

    static t_delay reset_delay;

    static uint8_t page_index;

    uint8_t *page_buffer_a;
    uint8_t *page_buffer_b;

    switch (state) {

    case STATE_ASSERT_RESET:

        dev_lcd_reset(true);

        reset_delay.delay_time = 5;
        delay_start(&reset_delay);

        state = STATE_RELEASE_RESET;
        break;

    case STATE_RELEASE_RESET:
        // Hold in reset for 5 us.
        if (delay_us(&reset_delay)) {
            dev_lcd_reset(false);

            delay_start(&reset_delay);

            state = STATE_INIT;
        }
        break;

    // Initialise LCD task.
    case STATE_INIT:
        // Wait 5 us after reset released.
        if (delay_us(&reset_delay)) {

            if (error_check(_display_init()) == SUCCESS) {
                state = STATE_RUN;
            }
        } // Remain in INIT state until initialisation successful.
        break;

    case STATE_RUN:
        /// TODO: Ping-pong buffer would be better than double buffer.
        //
        // Send single page per task invocation.
        page_buffer_a = g_frame_buffer_a + page_index * LCD_COLUMNS;
        page_buffer_b = g_frame_buffer_b + page_index * LCD_COLUMNS;

        if (memcmp(page_buffer_a, page_buffer_b, LCD_COLUMNS)) {

            /// TODO: Is DMA faster?
            memcpy(page_buffer_b, page_buffer_a, LCD_COLUMNS);
            dev_lcd_set_page(page_index, page_buffer_b);
        }
        page_index++;

        if (page_index >= 8) {
            page_index = 0;
        }
        break;

    case STATE_ERROR:
        error_check(UNRECOVERABLE_ERROR);
        break;

    default:
        /// TODO: Record unhandled state.
        if (error_check(UNHANDLED_STATE_ERROR) != SUCCESS) {
            state = STATE_ERROR;
        }
        break;
    }
}

void svc_display_set_contrast(uint8_t contrast) {

    dev_lcd_set_contrast(contrast);
}

/*----- Static function implementations ------------------------------*/

static t_status _display_init(void) {

    uint8_t *page_buffer;
    uint8_t page_index;

    dev_lcd_init();

    for (page_index = 0; page_index < 8; page_index++) {

        page_buffer = g_frame_buffer_b + page_index * LCD_COLUMNS;
        dev_lcd_set_page(page_index, page_buffer);
    }

    svc_event_subscribe(SVC_EVENT_PUT_PIXEL, _put_pixel_listener);
    svc_event_subscribe(SVC_EVENT_FILL_FRAME, _fill_frame_listener);

    return SUCCESS;
}

static void _put_pixel_listener(const t_event *event) {

    t_pixel *pixel = (t_pixel *)event->data;

    _put_pixel(pixel->x, pixel->y, pixel->state);
}

static void _fill_frame_listener(const t_event *event) {

    t_frame *frame = (t_frame *)event->data;

    _fill_frame(frame->x_start, frame->y_start, frame->x_end, frame->y_end,
                frame->state);
}

static void _put_pixel(uint16_t pos_x, uint16_t pos_y, bool state) {

    // Calculate pixel location in buffer.
    // uint16_t column_index = pos_x;
    // uint16_t page_index = pos_y >> 3;
    // uint16_t byte_index = column_index + (128 * page_index);

    uint16_t byte_index = pos_x + ((pos_y >> 3) << 7);
    uint16_t bit_index = pos_y & 7;

    // Get current byte from frame buffer.
    uint8_t byte = g_frame_buffer_a[byte_index];

    // Set pixel bit and write to frame buffer.
    g_frame_buffer_a[byte_index] =
        (byte & ~(1UL << bit_index)) | (state << bit_index);

    /// TODO: Is this any faster?
    ///         Are reads and conditionals faster than writes?
    ///             Is it worth testing if byte differs
    ///             before writing?
    ///             (Probably not, if cache enabled.)
    //
    // Get current byte from frame buffer.
    // uint8_t *byte = &g_frame_buffer[byte_index];
    //
    // if ((*byte & ~(1UL << bit_index)) != (state << bit_index)) {
    //     // Set pixel bit and write to frame buffer.
    //     g_frame_buffer[byte_index] =
    //         (*byte & ~(1UL << bit_index)) | (state << bit_index);
    // }
}

static int8_t _fill_frame(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                          uint16_t y_end, bool state) {

    /// TODO: Handle partial bytes.

    uint8_t fill;
    uint16_t length;

    // uint8_t partial_start = y_start & 7;
    // uint8_t partial_end = y_end & 7;

    uint16_t byte_start = x_start + ((y_start >> 3) << 7);
    uint16_t byte_end = x_end + ((y_end >> 3) << 7);

    if (state) {
        fill = 0;
    } else {
        fill = 0xff;
        // partial_start = ~partial_start;
        // partial_end = ~partial_end;
    }

    /// TODO: This does not work.
    //
    // if (partial_start) {
    //     memset(g_frame_buffer_a + byte_start, partial_start, 128 - x_start);
    //     byte_start += 128;
    // }
    //
    // if (partial_end) {
    //     byte_end -= 128;
    //     memset(g_frame_buffer_a + byte_end, partial_end, 128 - x_end);
    // }

    length = (byte_end + 1) - byte_start;

    memset(g_frame_buffer_a + byte_start, fill, length);

    return 0;
}

/*----- End of file --------------------------------------------------*/
