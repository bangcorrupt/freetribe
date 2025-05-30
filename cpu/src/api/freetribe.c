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
 * @file    freetribe.c
 *
 * @brief   Freetribe application library.
 *
 * A set of wrappers for user applications to access kernel functions.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdio.h>

#include "knl_syscalls.h"

#include "freetribe.h"

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

static t_syscall *g_syscall_table;

static t_syscall _print;
static t_syscall _put_pixel;
static t_syscall _fill_frame;
static t_syscall _set_led;
static t_syscall _init_delay;
static t_syscall _test_delay;
static t_syscall _register_callback;
static t_syscall _event_subscribe;
static t_syscall _send_midi_msg;
static t_syscall _set_module_param;
static t_syscall _get_module_param;
static t_syscall _set_trigger_mode;
static t_syscall _shutdown;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void ft_init(void) {

    g_syscall_table = (*(t_get_syscalls *)PTR_GET_SYSCALLS)();

    _print = g_syscall_table[SYSCALL_PRINT];
    _put_pixel = g_syscall_table[SYSCALL_PUT_PIXEL];
    _fill_frame = g_syscall_table[SYSCALL_FILL_FRAME];
    _set_led = g_syscall_table[SYSCALL_SET_LED];
    _init_delay = g_syscall_table[SYSCALL_INIT_DELAY];
    _test_delay = g_syscall_table[SYSCALL_TEST_DELAY];
    _register_callback = g_syscall_table[SYSCALL_REGISTER_CALLBACK];
    _event_subscribe = g_syscall_table[SYSCALL_EVENT_SUBSCRIBE];
    _send_midi_msg = g_syscall_table[SYSCALL_SEND_MIDI_MSG];
    _set_module_param = g_syscall_table[SYSCALL_SET_MODULE_PARAM];
    _get_module_param = g_syscall_table[SYSCALL_GET_MODULE_PARAM];
    _set_trigger_mode = g_syscall_table[SYSCALL_SET_TRIGGER_MODE];
    _shutdown = g_syscall_table[SYSCALL_SHUTDOWN];
}

// Tick API
//
/**
 * @brief   Register a callback for userspace tick events.
 *
 * The kernel systick triggers once per millisecond.
 * Set the divisor greater than 0 to trigger the user tick callback less often.
 *
 * @param[in]   divisor     Ratio of kernel ticks to user ticks.
 * @param[in]   callback    Function to call.
 */
void ft_register_tick_callback(uint32_t divisor, void (*callback)(void)) {

    t_callback cb = {.id = CALLBACK_TICK, .arg = divisor, .handler = callback};

    _register_callback(&cb);
}

// Delay API
//

/// TODO: Set time as separate argument, not in state struct.
//
/**
 * @brief   Non-blocking delay initialisation.
 *
 * Initialises state of delay to zero.
 *
 * @param[out]  state   State of delay.
 * @param[in]   time    Time in microseconds.
 *
 */
void ft_start_delay(t_delay *delay) {

    //
    _init_delay(delay);
}

/**
 * @brief   Non-blocking delay.
 *
 *  Define a struct of type `t_delay_state`, then pass it to ft_start_delay().
 *
 * @param[out]  state   State of delay.
 *
 * @return      True if at least `state->delay_time` microseconds
 *              have passed since `state->start_time`.
 */
bool ft_delay(t_delay *delay) {

    _test_delay(delay);

    return delay->expired;
}

// Display API
//
//
void ft_put_pixel(uint16_t pos_x, uint16_t pos_y, bool state) {

    t_pixel pixel = {.x = pos_x, .y = pos_y, .state = state};

    _put_pixel(&pixel);
}

int8_t ft_fill_frame(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                     uint16_t y_end, bool state) {

    t_frame frame = {.x_start = x_start,
                     .y_start = y_start,
                     .x_end = x_end,
                     .y_end = y_end,
                     .state = state};

    _fill_frame(&frame);

    return 0;
}

// Print API
//
/**
 * @brief   Print ASCII string as MIDI system exclusive message.
 *
 * String will be encapsulated with 0xf7...0xf0
 * to ensure it passes through receiving MIDI drivers.
 *
 * @param[in]   text    String to be printed.
 *
 */
void ft_print(char *text) {
    //
    _print(text);
}

// Panel API
//
/**
 * @brief   Register a callback for panel control input events.
 *
 * @param[in]   event       Type of event to catch.
 * @param[in]   callback    Function to call.
 *
 */
/// TODO: Central event queue.
//
void ft_register_panel_callback(t_panel_event event, void *callback) {

    t_callback cb = {.id = CALLBACK_PANEL, .arg = event, .handler = callback};

    _register_callback(&cb);
}

void ft_set_trigger_mode(uint8_t mode) {
    //
    _set_trigger_mode(&mode);
}

// EVENT API
//
/**
 * @brief   Register a kernel events.
 *
 * @param[in]   id          ID of event to catch.
 * @param[in]   listener    Function to call.
 *
 */
void ft_event_subscribe(e_event_id id, t_listener listener) {

    struct params {
        e_event_id id;
        t_listener listener;
    };

    struct params p = {.id = id, .listener = listener};

    _event_subscribe(&p);
}

// MIDI API
//
/**
 * @brief   Register a callback MIDI input events.
 *
 * @param[in]   event       Type of event to catch.
 * @param[in]   callback    Function to call.
 *
 */
void ft_register_midi_callback(event_type event,
                               t_midi_event_callback callback) {

    t_callback cb = {.id = CALLBACK_MIDI, .arg = event, .handler = callback};

    _register_callback(&cb);
}

/**
 * @brief   Send MIDI note on message.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   note    MIDI note number.
 * @param[in]   vel     MIDI note velocity.
 *
 */
void ft_send_note_on(char chan, char note, char vel) {

    t_midi_msg msg = {.status = 0x90 & chan, .byte_1 = note, .byte_2 = vel};

    _send_midi_msg(&msg);
}

/**
 * @brief   Send MIDI note off message.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   note    MIDI note number.
 * @param[in]   vel     MIDI note velocity.
 *
 */
void ft_send_note_off(char chan, char note, char vel) {

    t_midi_msg msg = {.status = 0x80 & chan, .byte_1 = note, .byte_2 = vel};

    _send_midi_msg(&msg);
}

/**
 * @brief   Send MIDI control change message.
 *
 * @param[in]   chan    MIDI channel.
 * @param[in]   index   MIDI CC number.
 * @param[in]   val     MIDI CC value.
 *
 */
void ft_send_cc(char chan, char index, char val) {

    t_midi_msg msg = {.status = 0xB0 & chan, .byte_1 = index, .byte_2 = val};

    _send_midi_msg(&msg);
}

// LED API
//

/// FIX: DEPRECATED
///         Kernel should not keep track of LED state.
///         Move this to application library.
//
/// TODO: Update examples.
//
/**
 * @brief   Toggle LED into the opposite state.
 *
 * @param[in]   led_index   Index of LED to toggle.
 *
 */
void ft_toggle_led(t_led_index led_index) { svc_panel_toggle_led(led_index); }

/// TODO: Enums should be prefixed with e_, not t_.
//
/**
 * @brief   Set LED.
 *
 * @param[in]   led_index   Index of LED to toggle.
 *
 */
void ft_set_led(t_led_index led_index, uint8_t value) {

    t_led led = {.index = led_index, .value = value};

    _set_led(&led);
}

// DSP Command API
//
/**
 * @brief   Set parameter value in DSP audio module.
 *
 * @param[in]   module_id   Index of module to address.
 * @param[in]   param_index Index of parameter to set.
 * @param[in]   param_value Value of parameter.
 *
 */
void ft_set_module_param(uint16_t module_id, uint16_t param_index,
                         int32_t param_value) {

    t_param param = {
        .module_id = module_id, .index = param_index, .value = param_value};

    _set_module_param(&param);
}

/**
 * @brief   Request parameter value from DSP audio module.
 *
 * First register a callback for MODULE_PARAM_VALUE event,
 * then call this to request the data from the DSP.
 *
 * @param[in]   module_id   Index of module to address.
 * @param[in]   param_index Index of parameter to get.
 *
 */
void ft_get_module_param(uint8_t module_id, uint16_t param_index) {

    t_param param = {.module_id = module_id, .index = param_index};

    _get_module_param(&param);
}

/**
 * @brief   Shutdown the system.
 */
void ft_shutdown(void) {

    void *p = NULL;

    _shutdown(p);
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
