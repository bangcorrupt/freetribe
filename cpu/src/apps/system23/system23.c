#include "freetribe.h"

#include "sequencer.h"
#include "param_scale.h"
#include "lut.h"


#define TRIGGER_MODE_CONTINUOUS 1
#define SAMPLERATE 48000
#define BPM 150
#define BEAT_1_4 (60000 / (BPM * 4))
#define BUTTON_EXIT 0x0D
#define BUTTON_BAR_0 0x1C
#define BUTTON_BAR_1 0x1D
#define BUTTON_BAR_2 0x1E
#define BUTTON_BAR_3 0x1F

#define KNOB_LEVEL 0x00
#define KNOB_PITCH 0x02
#define KNOB_RESONANCE 0x03
#define KNOB_EG 0x04
#define KNOB_ATTACK 0x06
#define KNOB_DECAY 0x08
#define KNOB_MOD_DEPTH 0x05
#define KNOB_MOD_SPEED 0x0a

#define ENCODER_OSC 0x01
#define ENCODER_CUTOFF 0x02
#define ENCODER_MOD 0x03

#define PARAM_NOTE_FREQ 2
#define PARAM_GATE_TICKS 3
#define PARAM_NEXT_NOTE_FREQ 4
#define PARAM_TICKS_TIL_NEXT 5 // the number of DSP frames it takes to get to the next note after the current one's gate ends
#define PARAM_CUTOFF 10
#define PARAM_RESONANCE 11

const int32_t NOTE_FREQS[128] = {
    0x00077340, 0x0007E4AA, 0x00085CD1, 0x0008DC1E, 0x000962FD, 0x0009F1E0, 0x000A8943, 0x000B29A6, 0x000BD393, 0x000C879A, 
    0x000D4656, 0x000E1069, 0x000EE681, 0x000FC953, 0x0010B9A3, 0x0011B83C, 0x0012C5F9, 0x0013E3C0, 0x00151286, 0x0016534C, 
    0x0017A726, 0x00190F34, 0x001A8CAC, 0x001C20D3, 0x001DCD02, 0x001F92A7, 0x00217345, 0x00237078, 0x00258BF2, 0x0027C781, 
    0x002A250C, 0x002CA698, 0x002F4E4B, 0x00321E69, 0x00351958, 0x003841A6, 0x003B9A04, 0x003F254E, 0x0042E68B, 0x0046E0F0, 
    0x004B17E5, 0x004F8F01, 0x00544A17, 0x00594D31, 0x005E9C96, 0x00643CD2, 0x006A32B1, 0x0070834C, 0x00773407, 0x007E4A9B, 
    0x0085CD15, 0x008DC1E1, 0x00962FC9, 0x009F1E03, 0x00A8942E, 0x00B29A62, 0x00BD392D, 0x00C879A3, 0x00D46562, 0x00E10697, 
    0x00EE680F, 0x00FC9536, 0x010B9A2B, 0x011B83C2, 0x012C5F93, 0x013E3C06, 0x0151285D, 0x016534C3, 0x017A725A, 0x0190F347, 
    0x01A8CAC3, 0x01C20D2F, 0x01DCD01D, 0x01F92A6D, 0x02173456, 0x02370783, 0x0258BF26, 0x027C780B, 0x02A250BA, 0x02CA6987, 
    0x02F4E4B4, 0x0321E68D, 0x03519586, 0x03841A5D, 0x03B9A03A, 0x03F254D9, 0x042E68AC, 0x046E0F07, 0x04B17E4B, 0x04F8F017, 
    0x0544A173, 0x0594D30D, 0x05E9C968, 0x0643CD1B, 0x06A32B0D, 0x070834BA, 0x07734075, 0x07E4A9B2, 0x085CD157, 0x08DC1E0D, 
    0x0962FC96, 0x09F1E02D, 0x0A8942E7, 0x0B29A61A, 0x0BD392D0, 0x0C879A35, 0x0D46561A, 0x0E106974, 0x0EE680E9, 0x0FC95364, 
    0x10B9A2AF, 0x11B83C1A, 0x12C5F92C, 0x13E3C05A, 0x151285CE, 0x16534C35, 0x17A725A0, 0x190F346A, 0x1A8CAC34, 0x1C20D2E8, 
    0x1DCD01D3, 0x1F92A6C8, 0x2173455E, 0x23707835, 0x258BF259, 0x27C780B5, 0x2A250B9C, 0x2CA6986A
};

typedef struct {
    uint32_t gate;
    uint8_t note;
    bool slide;
    bool accent;
} t_step;

static uint32_t g_beat_ticks;
static uint32_t g_old_systicks;
static uint8_t g_step = 0; // position within current bar
static uint8_t g_total_steps = 0;
static uint8_t g_bar = 0;
static uint8_t g_sequencer_num_steps = 8;
static uint8_t g_selected_bar = 0;
static t_step g_sequencer_steps[64] = {
    { GATE_4TH, 57+0, false, false },
    { GATE_16TH, 57+1, false, false },
    { GATE_4TH, 57+1 , false, true  },
    { GATE_4TH, 57+0, false, false },
    { GATE_16TH, 57+12, false, false },
    { GATE_4TH, 57+24 , false, false },
    { GATE_8TH, 57+13, false, false },
    { GATE_4TH, 57+25 , false, false },
};

static void _trigger_callback(uint8_t pad, uint8_t vel, bool state);
static void _button_callback(uint8_t index, bool state);
static void _knob_callback(uint8_t index, uint8_t value);
static void _encoder_callback(uint8_t index, uint8_t value);
static void _tick_callback(void);
static void _update_sequencer_display();


/**
 * @brief   Initialise application.
 *
 * @return status   Status code indicating success:
 *                  - SUCCESS
 *                  - WARNING
 *                  - ERROR
 */
t_status app_init(void) {

    t_status status = ERROR;

    g_old_systicks = systick_get(); // reset beat timer
    ft_register_tick_callback(0, _tick_callback);
    ft_register_panel_callback(TRIGGER_EVENT, _trigger_callback);
    ft_register_panel_callback(BUTTON_EVENT, _button_callback);
    ft_register_panel_callback(ENCODER_EVENT, _encoder_callback);
    ft_register_panel_callback(KNOB_EVENT, _knob_callback);
    // ft_set_trigger_mode(TRIGGER_MODE_CONTINUOUS); // wtf is this

    lut_init();
    ft_print("System23 initialised.\n");


    status = SUCCESS;
    return status;
}

/**
 * @brief   Run application.
 */
void app_run(void) {

    _update_sequencer_display();

}

/*----- Static function implementations ------------------------------*/

static void _trigger_callback(uint8_t pad, uint8_t vel, bool state) {

    // ft_printf("trigger callback: %02X %u %u", pad, vel, state);

    uint8_t step_index = pad + (g_selected_bar * 16);
    if (g_sequencer_steps[step_index].gate > 0) {
        g_sequencer_steps[step_index].gate = 0; // note is off
    } else {
        g_sequencer_steps[step_index].gate = GATE_4TH; // note is on
    }

}

static void _knob_callback(uint8_t index, uint8_t value) {

    // char text[64];
    // sprintf(text, "knob callback: %02X %02X", index, value);
    // ft_print(text);

    switch (index) {
    case KNOB_RESONANCE:
        ft_set_module_param(0, PARAM_RESONANCE, g_resonance_lut[value]);
        break;
    }
}

static void _encoder_callback(uint8_t index, uint8_t value) {
    static uint8_t cutoff = 0x7f;

    switch (index) {

    case ENCODER_CUTOFF:
        if (value == 0x01) {
            if (cutoff < 0x7f)
                cutoff++;
        } else {
            if (cutoff > 0)
                cutoff--;
        }
        // char text[64];
        // sprintf(text, "encoder callback: %02X %f", cutoff, note_to_cv(cutoff));
        // ft_print(text);
        ft_set_module_param(0, PARAM_CUTOFF, g_cutoff_lut[cutoff]);
        break;

    }

}

static void _button_callback(uint8_t index, bool state) {
    ft_printf("button callback: %u=%u", index, state);

    switch (index) {
    case BUTTON_EXIT: ft_shutdown(); break;
    case BUTTON_BAR_0: g_selected_bar = 0; break;
    case BUTTON_BAR_1: g_selected_bar = 1; break;
    case BUTTON_BAR_2: g_selected_bar = 2; break;
    case BUTTON_BAR_3: g_selected_bar = 3; break;
    }
}

static void _tick_callback(void) {

    g_beat_ticks += systick_get() - g_old_systicks;
    g_old_systicks = systick_get();
    if (g_beat_ticks > BEAT_1_4) {
        g_beat_ticks = 0;

        // Play note
        uint8_t step = 16 * g_bar + g_step;
        if (g_sequencer_steps[step].gate > 0) {
            ft_set_module_param(0, PARAM_NOTE_FREQ, NOTE_FREQS[g_sequencer_steps[step].note]);
            ft_set_module_param(0, PARAM_GATE_TICKS, g_sequencer_steps[step].gate);
            uint8_t next_step = (step+1)%(sizeof(g_sequencer_steps)/sizeof(t_step));
            int32_t ticks_til_next_gate = GATE_4TH - g_sequencer_steps[step].gate;
            ft_set_module_param(0, PARAM_TICKS_TIL_NEXT, ticks_til_next_gate);
            ft_set_module_param(0, PARAM_NEXT_NOTE_FREQ, NOTE_FREQS[g_sequencer_steps[next_step].note]);
        }

        g_step++;
        if (g_step >= 16) {
            g_step = 0;
            g_bar++;
            if (g_bar > 3) {
                g_bar = 0;
            }
        }

        // if the pattern is smaller than 64, we gotta reset the sequencer position
        g_total_steps++;
        if (g_total_steps >= g_sequencer_num_steps) {
            g_total_steps = 0;
            g_bar = 0;
            g_step = 0;
        }

    }
}


static void _update_sequencer_display() {

    // step in pad
    if (g_bar == g_selected_bar) {
        for (uint8_t i = 0; i <= 15; i++) {
            if (i != g_step)
                ft_set_led(LED_PAD_0_BLUE + (i<<1), false);
        }
        ft_set_led(LED_PAD_0_BLUE + (g_step<<1), true);
    } else {
        for (uint8_t i = 0; i <= 15; i++)
            ft_set_led(LED_PAD_0_BLUE + (i<<1), false);
    }

    // draw steps
    for (uint8_t i = 0; i <= 15; i++) {
        bool has_note = g_sequencer_steps[16 * g_selected_bar + i].gate > 0;
        ft_set_led(LED_PAD_0_RED + (i<<1), has_note);
    }

    // bars
    for (uint8_t i = 0; i <= 3; i++) {
        ft_set_led(LED_BAR_0_BLUE + i, (i == g_bar));
    }
    for (uint8_t i = 0; i <= 3; i++) {
        ft_set_led(LED_BAR_0_RED + i, (i == g_selected_bar));
    }
}
