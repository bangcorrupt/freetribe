/*----------------------------------------------------------------------

                     This file is part of Aleph DSP

                https://github.com/bangcorrupt/aleph-dsp

         Aleph DSP is based on monome/aleph and spiricom/LEAF.

                              MIT License

            Aleph dedicated to the public domain by monome.

                LEAF Copyright Jeff Snyder et. al. 2020

                       Copyright bangcorrupt 2024

----------------------------------------------------------------------*/

/**
 * @file    Custom_Aleph_MonoVoice.h
 *
 * @brief   Public API for monovoice module.
 */

#ifndef Custom_Aleph_MonoVoice_H
#define Custom_Aleph_MonoVoice_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include "aleph.h"

#include "aleph_filter.h"
#include "aleph_filter_svf.h"
#include "aleph_lpf_one_pole.h"
#include "aleph_waveform.h"

/*----- Macros -------------------------------------------------------*/

#define Custom_Aleph_MonoVoice_DEFAULT_AMP (0)
#define Custom_Aleph_MonoVoice_DEFAULT_FREQ (220 << 16)
#define Custom_Aleph_MonoVoice_DEFAULT_FREQ_OFFSET (0)

#define Custom_Aleph_MonoVoice_DEFAULT_CUTOFF (20000 << 16)
#define Custom_Aleph_MonoVoice_DEFAULT_RES (FR32_MAX)
#define Custom_Aleph_MonoVoice_DEFAULT_FILTER_TYPE ALEPH_FILTERSVF_TYPE_LPF
#define MAP_255_TO_WAVE_TAB(valor) (((valor) * (WAVE_TAB_SIZE - 1)) >> 8)

// valor * WAVE_TAB_SIZE_SDRAM - WAVE_TAB_CYCLE_IN_SAMPLES / 255
#define MAP_255_TO_WAVE_TAB_SDRAM(valor) (((valor) << 10 ) )


#define WAVE_TAB_SIZE 4096
#define WAVE_TAB_SIZE_SDRAM 262144
#define WAVE_TAB_CYCLE_IN_SAMPLES 1024
#define SDRAM_ADDRESS 0x00000000


/*----- Typedefs -----------------------------------------------------*/

typedef struct {

    Mempool mempool;

    Aleph_Waveform waveform;
    fract32 freq_offset;

    Aleph_FilterSVF filter;
    e_Aleph_FilterSVF_type filter_type;

    Aleph_LPFOnePole amp_slew;
    Aleph_LPFOnePole freq_slew;
    Aleph_LPFOnePole cutoff_slew;

    Aleph_HPF dc_block;
    fract32 morph_amount;

} t_Custom_Aleph_MonoVoice;

typedef t_Custom_Aleph_MonoVoice *Custom_Aleph_MonoVoice;

/*----- Extern variable declarations ---------------------------------*/
extern fract32 wavtab[WAVE_TAB_SIZE];
extern fract32 *data_sdram;

/*----- Extern function prototypes -----------------------------------*/

void Custom_Aleph_MonoVoice_init(Custom_Aleph_MonoVoice *const synth, t_Aleph *const aleph);
void Custom_Aleph_MonoVoice_init_to_pool(Custom_Aleph_MonoVoice *const synth,
                                  Mempool *const mempool);
void Custom_Aleph_MonoVoice_free(Custom_Aleph_MonoVoice *const synth);

fract32 Custom_Aleph_MonoVoice_next(Custom_Aleph_MonoVoice *const synth);


void Custom_Aleph_MonoVoice_set_amp(Custom_Aleph_MonoVoice *const synth, fract32 amp);
void Custom_Aleph_MonoVoice_set_phase(Custom_Aleph_MonoVoice *const synth, fract32 phase);

void Custom_Aleph_MonoVoice_set_freq(Custom_Aleph_MonoVoice *const synth, fract32 freq);
void Custom_Aleph_MonoVoice_set_freq_offset(Custom_Aleph_MonoVoice *const synth,
                                     fract32 freq_offset);

void Custom_Aleph_MonoVoice_set_filter_type(Custom_Aleph_MonoVoice *const synth,
                                     e_Aleph_FilterSVF_type type);

void Custom_Aleph_MonoVoice_set_cutoff(Custom_Aleph_MonoVoice *const synth, fract32 cutoff);
void Custom_Aleph_MonoVoice_set_res(Custom_Aleph_MonoVoice *const synth, fract32 res);

void Custom_Aleph_MonoVoice_set_amp_slew(Custom_Aleph_MonoVoice *const synth,
                                  fract32 amp_slew);

void Custom_Aleph_MonoVoice_set_freq_slew(Custom_Aleph_MonoVoice *const synth,
                                   fract32 freq_slew);

void Custom_Aleph_MonoVoice_set_cutoff_slew(Custom_Aleph_MonoVoice *const synth,
                                     fract32 cutoff_slew);

void Custom_Aleph_MonoVoice_set_shape(Custom_Aleph_MonoVoice *const synth, e_Aleph_Waveform_shape shape) ;

void Custom_Aleph_MonoVoice_set_morph_amount(Custom_Aleph_MonoVoice *const synth,fract32 morph_amount) ;
void Custom_Aleph_MonoVoice_record(fract32 data) ;
void Custom_Aleph_MonoVoice_record_reset();

#ifdef __cplusplus
}
#endif
#endif

/*----- End of file --------------------------------------------------*/
