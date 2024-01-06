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
 * @file midi_fsm.h.
 *
 * @brief  Finite state machine for MIDI input parsing.
 *
 */

/* Original work by the libmidi authors, modified by bangcorrupt 2023. */

// TODO: Abstract from Freetribe and port back to libmidi with original license.

/*
 * Copyright (c) 2018, The libmidi authors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// TODO: Abstract to seaparate library with original BSD license.

#ifndef MIDI_FSM_H
#define MIDI_FSM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ft_error.h"

/*
 * As libmidi is primarily intended for use in embedded environments, users
 * will likely need to configure their UART. The MIDI_BAUD_RATE macro is
 * provided as a convenience and as a reminder of the correct baud rate.
 * This constant is not used by libmidi.
 */
#define MIDI_BAUD_RATE 31250

// Length of buffer to hold incoming system exclusive data bytes.
#define SYSEX_BUFFER_LENGTH 0x2000

/*
 * This library uses an event-driven paradigm for receiving data: callers
 * register callback functions for events they are interested in, and these
 * are invoked when events are received.
 *
 * For the sake of simplicity, we define one type, midi_event_callback_t for
 * all callback functions. Please note that some parameters are unused in
 * certain contexts: the documentation is clear on what parameters are valid
 * for each event type.
 *
 * The caller is only obligated to register callback functions for those
 * events she is interested in; other events will be dispatched to a null
 * handler implements within the library.
 */
typedef void (*midi_event_callback_t)(char chan, char data1, char data2);

// Separate callback definition for system exclusive messages.
typedef void (*midi_sysex_callback_t)(char *msg, unsigned long length);

/*
 * Error codes returned by library functions.
 */
enum { E_MIDI_BAD_EVENT_HANDLER = -1, E_MIDI_BAD_CHANNEL_STATE = -2 };

/*
 * As noted elsewhere, libmidi is event-driven, and handles received data
 * via a callback mechanism. The enumeration values of event_type describe
 * the different types of events for which callers may register.
 */
typedef enum event_type {
    /*
     * System real-time messages.
     */
    EVT_SYS_REALTIME_TIMING_CLOCK = 0,
    EVT_SYS_REALTIME_RESERVED_F9,
    EVT_SYS_REALTIME_SEQ_START,
    EVT_SYS_REALTIME_SEQ_CONTINUE,
    EVT_SYS_REALTIME_SEQ_STOP,
    EVT_SYS_REALTIME_RESERVED_FD,
    EVT_SYS_REALTIME_ACTIVE_SENSE,
    EVT_SYS_REALTIME_RESET,

    /*
     * Channel messages.
     */
    EVT_CHAN_NOTE_OFF,
    EVT_CHAN_NOTE_ON,
    EVT_CHAN_POLY_AFTERTOUCH,
    EVT_CHAN_CONTROL_CHANGE,
    EVT_CHAN_PROGRAM_CHANGE,
    EVT_CHAN_AFTERTOUCH,
    EVT_CHAN_PITCH_BEND,

    /*
     * Placeholder whose value is equal to the total number of event types
     * that we support above.
     */
    EVT_MAX,

    // System exclusive messages.
    // Placed after EVT_MAX as it uses a separate callback handler.
    EVT_SYS_EXCLUSIVE

} event_type;

/**
 * Initialize the MIDI library. This routine must be called prior to using
 * library functions or unpredictable behavior may result.
 *
 * @return Zero on success; nonzero status otherwise.
 */
t_status midi_init_fsm();

/**
 * Register an event handler for the specified event. To clear an event
 * handler, simply pass a NULL pointer for the callback argument.
 *
 * Returns  0 if a callback was registered or cleared successfully.
 * Returns -1 if the event type was invalid.
 *
 * @param evt Event to register. See the EVT_xxx enumerations, above.
 * @param cb Function to invoke upon receipt of the MIDI event.
 * @return Status code indicating (see above for comments.)
 */
t_status midi_register_event_handler(event_type evt, midi_event_callback_t cb);

/**
 * @brief Register an event callback for system exclusive messages.
 *
 * @param cb Function to invoke upon receipt of the system exclusive message.
 *
 * @return Status code indicating (see above for comments.)
 *
 */
t_status midi_register_sysex_handler(midi_sysex_callback_t cb);

/**
 * Processes a byte arriving via the MIDI input of the device. This function
 * *must* be invoked for every byte that arrives so that the internal state
 * machine maintained by the library is updated properly. Calling this function
 * may result in callback invocations. The library user is cautioned to process
 * callbacks as quickly as feasible to avoid missing data and/or events/
 *
 * On success, the function returns the number of callbacks that resulted from
 * the new byte's arrival. In the event that the received byte is invalid, a
 * negative code is returned (see the section above that outlines status codes
 * for this library.)
 *
 * @param byte Byte of data received from an input port.
 *
 * @return Number of callback invocations. Returns negative status on error.
 */
t_status midi_receive_byte(char byte);

#ifdef __cplusplus
}
#endif

#endif /* MIDI_FSM_H_INCLUDED */
