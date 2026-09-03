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

----------------------------------------------------------------------*/

/**
 * @file    panel_codes.c
 *
 * @brief   Example application for identifying panel control codes.
 *
 * Log every button, encoder, and knob event with its numeric code. Known
 * controls are named, while unknown controls remain visible for discovery.
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "freetribe.h"

/*----- Macros -------------------------------------------------------*/

#define BUTTON_MENU 0x09
#define BUTTON_SHIFT 0x0a
#define BUTTON_EXIT 0x0d
#define BUTTON_AMP_EG 0x20
#define BUTTON_LPF 0x12
#define BUTTON_HPF 0x14
#define BUTTON_BPF 0x16
#define BUTTON_PLAY 0x02

#define KNOB_LEVEL 0x00
#define KNOB_PITCH 0x02
#define KNOB_RES 0x03
#define KNOB_EG 0x04
#define KNOB_MOD_DEPTH 0x05
#define KNOB_ATTACK 0x06
#define KNOB_DECAY 0x08
#define KNOB_MOD_SPEED 0x0a

#define ENCODER_OSC 0x01
#define ENCODER_CUTOFF 0x02
#define ENCODER_MOD 0x03

/*----- Static function prototypes -----------------------------------*/

static const char *_button_name(uint8_t button);
static const char *_knob_name(uint8_t knob);
static const char *_encoder_name(uint8_t encoder);
static void _button_callback(uint8_t button, bool state);
static void _knob_callback(uint8_t knob, uint8_t value);
static void _encoder_callback(uint8_t encoder, int8_t value);

/*----- Extern function implementations ------------------------------*/

t_status app_init(void) {

    ft_register_panel_callback(BUTTON_EVENT, _button_callback);
    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    ft_register_panel_callback(ENCODER_EVENT, _encoder_callback);

    ft_printf("Panel code example");
    ft_printf("Move controls; events are printed with code and value.");

    return SUCCESS;
}

void app_run(void) {}

/*----- Static function implementations ------------------------------*/

static const char *_button_name(uint8_t button) {

    switch (button) {
    case BUTTON_MENU: return "MENU";
    case BUTTON_SHIFT: return "SHIFT";
    case BUTTON_EXIT: return "EXIT";
    case BUTTON_AMP_EG: return "AMP_EG";
    case BUTTON_LPF: return "LPF";
    case BUTTON_HPF: return "HPF";
    case BUTTON_BPF: return "BPF";
    case BUTTON_PLAY: return "PLAY";
    default: return "UNKNOWN";
    }
}

static const char *_knob_name(uint8_t knob) {

    switch (knob) {
    case KNOB_LEVEL: return "LEVEL";
    case KNOB_PITCH: return "PITCH";
    case KNOB_RES: return "RES";
    case KNOB_EG: return "EG";
    case KNOB_MOD_DEPTH: return "MOD_DEPTH";
    case KNOB_ATTACK: return "ATTACK";
    case KNOB_DECAY: return "DECAY";
    case KNOB_MOD_SPEED: return "MOD_SPEED";
    default: return "UNKNOWN";
    }
}

static const char *_encoder_name(uint8_t encoder) {

    switch (encoder) {
    case ENCODER_OSC: return "OSC";
    case ENCODER_CUTOFF: return "CUTOFF";
    case ENCODER_MOD: return "MOD";
    default: return "UNKNOWN";
    }
}

static void _button_callback(uint8_t button, bool state) {

    ft_printf("BUTTON 0x%02x %s %s", button, _button_name(button),
              state ? "DOWN" : "UP");
}

static void _knob_callback(uint8_t knob, uint8_t value) {

    ft_printf("KNOB 0x%02x %s VALUE %u", knob, _knob_name(knob), value);
}

static void _encoder_callback(uint8_t encoder, int8_t value) {

    ft_printf("ENCODER 0x%02x %s DELTA %d", encoder, _encoder_name(encoder),
              value);
}

/*----- End of file --------------------------------------------------*/
