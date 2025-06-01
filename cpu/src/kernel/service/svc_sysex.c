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
 * @file    svc_sysex.c
 *
 * @brief   MIDI system exclusive message handling.
 */

/// TODO: This is specific to Korg sysex format.
///       Abstract to separate library, should not be part of kernel.

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <string.h>

#include "svc_sysex.h"

#include "midi_fsm.h"
#include "svc_midi.h"
#include "svc_panel.h"
#include "sysex_codec.h"

#include "freetribe.h"

/*----- Macros -------------------------------------------------------*/

// Decoded data length is always less than received message length.
static uint8_t g_decode_buffer[SYSEX_BUFFER_LENGTH];
static uint8_t g_encode_buffer[SYSEX_BUFFER_LENGTH];

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

e_sysex_parse_result _parse_msg_body(uint8_t *msg, uint32_t msg_length);

void _respond_read_cpu_ram(uint8_t *read_address, uint32_t read_length);
void _respond_search_device(uint8_t echo_id);

/*----- Extern function implementations ------------------------------*/

/// TODO: Abstract this to separate library.

e_sysex_parse_result sysex_parse(uint8_t *msg, uint32_t length) {

    ft_set_led(LED_REC, 1);

    uint32_t product_id;

    e_sysex_parse_result state = PARSE_MANU_ID;

    while (length--) {

        switch (state) {

        case PARSE_MANU_ID:

            /// TODO: Use unique manufacturer ID.
            if (*msg++ == 0x42) { /// TODO: Use #define instead of magic number.
                state = PARSE_GLOBAL_CHANNEL;
            } else {
                state = SYSEX_PARSE_ERROR;
            }
            break;

        case PARSE_GLOBAL_CHANNEL:

            // Search device.
            if (*msg++ == 0x50) {

                // Search device request.
                if (*msg++ == 0) {
                    _respond_search_device(*msg);
                }

                /// TODO: Handle received Search Device Response.
                //
                // Ignore Search Device Response for now.

                state = SYSEX_PARSE_COMPLETE;

            } else {

                if ((*msg & 0xf) <= 0xf) {
                    state = PARSE_PRODUCT_ID;
                } else {
                    state = SYSEX_PARSE_ERROR;
                }
            }
            break;

        case PARSE_PRODUCT_ID:

            product_id = *msg++ << 16;
            product_id |= *msg++ << 8;
            product_id |= *msg++;

            length -= 2;

            if (product_id == 0x124) {
                state = PARSE_MSG_BODY;
            } else {
                state = SYSEX_PARSE_ERROR;
            }
            break;

        case PARSE_MSG_BODY:
            state = _parse_msg_body(msg, length);
            break;

        case SYSEX_PARSE_ERROR:
            length = 0;
            break;

        case SYSEX_PARSE_COMPLETE:
            length = 0;
            break;

        default:
            break;
        }
    }
    return state;
}

// void sysex_register_callback()

/*----- Static function implementations ------------------------------*/

e_sysex_parse_result _parse_msg_body(uint8_t *msg, uint32_t msg_length) {

    e_sysex_parse_result result = SYSEX_PARSE_ERROR;

    static uint8_t *write_address;
    static uint32_t write_length;

    static uint8_t *read_address;
    static uint32_t read_length;

    e_msg_id msg_id = *msg++;

    switch (msg_id) {

    case READ_CPU_RAM:

        if (msg_length) {

            sysex_decode(msg, g_decode_buffer, msg_length);

            /// TODO: Helper macro to pack/unpack int.
            //
            read_address =
                (uint8_t *)(g_decode_buffer[0] | g_decode_buffer[1] << 8 |
                            g_decode_buffer[2] << 16 |
                            g_decode_buffer[3] << 24);

            read_length = g_decode_buffer[4] | g_decode_buffer[5] << 8 |
                          g_decode_buffer[6] << 16 | g_decode_buffer[7] << 24;

            _respond_read_cpu_ram(read_address, read_length);

            result = SYSEX_PARSE_COMPLETE;
        }
        break;

    case SET_WRITE_ADDRESS:

        if (msg_length) {
            sysex_decode(msg, g_decode_buffer, msg_length);

            /// TODO: Helper macro to pack/unpack int.
            //
            write_address =
                (uint8_t *)(g_decode_buffer[0] | g_decode_buffer[1] << 8 |
                            g_decode_buffer[2] << 16 |
                            g_decode_buffer[3] << 24);

            write_length = g_decode_buffer[4] | g_decode_buffer[5] << 8 |
                           g_decode_buffer[6] << 16 | g_decode_buffer[7] << 24;

            result = SYSEX_PARSE_COMPLETE;
        }
        break;

    case WRITE_CPU_RAM:

        if (write_length && msg_length) {
            sysex_decode(msg, g_decode_buffer, msg_length);
            memcpy(write_address, g_decode_buffer, write_length);

            result = SYSEX_PARSE_COMPLETE;
        }

        write_address = 0;
        write_length = 0;
        break;

    case WRITE_FLASH:

        if (write_length && msg_length) {
            sysex_decode(msg, g_decode_buffer, msg_length);

            /// TODO: Sanity check before writing flash.
            //
            // flash_write(write_address, g_decode_buffer, write_length);

            result = SYSEX_PARSE_COMPLETE;
        }

        write_address = 0;
        write_length = 0;
        break;

    default:
        break;
    }

    return result;
}

void _respond_search_device(uint8_t echo_id) {

    //
    svc_midi_send_byte(0xf0);

    svc_midi_send_byte(0x42);
    svc_midi_send_byte(0x50);
    svc_midi_send_byte(0x01);
    svc_midi_send_byte(0x30);

    svc_midi_send_byte(echo_id);

    svc_midi_send_byte(0x24);
    svc_midi_send_byte(0x01);
    svc_midi_send_byte(0x01);
    svc_midi_send_byte(0x00);

    svc_midi_send_byte(0x00);
    svc_midi_send_byte(0x00);
    svc_midi_send_byte(0x00);
    svc_midi_send_byte(0x00);

    svc_midi_send_byte(0xf7);
}

void _respond_read_cpu_ram(uint8_t *read_address, uint32_t read_length) {

    uint32_t tx_length;

    svc_midi_send_byte(0xf0);
    svc_midi_send_byte(0x30);
    svc_midi_send_byte(0x42);
    svc_midi_send_byte(0x00);
    svc_midi_send_byte(0x01);
    svc_midi_send_byte(0x24);

    // Message ID.
    svc_midi_send_byte(0x4c);

    // Padding.
    svc_midi_send_byte(0x00);
    svc_midi_send_byte(0x00);

    tx_length = sysex_encode(read_address, g_encode_buffer, read_length);

    int i;
    for (i = 0; i < tx_length; i++) {

        svc_midi_send_byte(g_encode_buffer[i]);
    }

    svc_midi_send_byte(0xf7);
}

/*----- End of file --------------------------------------------------*/
