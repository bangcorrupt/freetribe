
// Freetribe: midi-knob
// License: AGPL-3.0-or-later

/*----- Includes -----------------------------------------------------*/

#include "freetribe.h"

/*----- Macros and Definitions ---------------------------------------*/

#define BUTTON_PLAY 0x2

/*----- Static variable definitions ----------------------------------*/

static bool g_toggle_led = false;

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

void _knob_callback(uint8_t index, uint8_t value);
void _button_callback(uint8_t index, bool state);

/*----- Extern function implementations ------------------------------*/

/**
 * @brief   Initialise application.
 */
t_status app_init(void) {

    t_status status = ERROR;

    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    ft_register_panel_callback(BUTTON_EVENT, _button_callback);

    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 */
void app_run(void) {

    if (g_toggle_led) {
        ft_toggle_led(LED_PLAY);

        g_toggle_led = false;
    }
}

/*----- Static function implementations ------------------------------*/

/**
 * @brief   Callback triggered by panel knob events.
 */
void _knob_callback(uint8_t index, uint8_t value) {

    ft_send_cc(0, index, value >> 1);
}

/**
 * @brief   Callback triggered by panel button events.
 */
void _button_callback(uint8_t index, bool state) {

    switch (index) {

    case BUTTON_PLAY:
        if (state == 1) {
            g_toggle_led = true;
        }
        break;
    }
}

/*----- End of file --------------------------------------------------*/
