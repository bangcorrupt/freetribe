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
 * @file    Custom_Aleph_MonoVoice.c
 *
 * @brief   Monophonic synth voice module.
 */

/*----- Includes -----------------------------------------------------*/

#include "aleph.h"

#include "aleph_env_adsr.h"
#include "aleph_filter.h"
#include "aleph_filter_svf.h"
#include "aleph_lpf_one_pole.h"
#include "aleph_oscillator.h"
#include "aleph_waveform.h"

#include "custom_aleph_monovoice.h"
#include "wave_lookup.h"


/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/
//static const 
fract32 wavtab[WAVE_TAB_SIZE] = 
    #include "util/sylenthva.data"

    fract32 *data_sdram;

unsigned long wavtab_index = 0;

/*----- Extern function implementations ------------------------------*/

void Custom_Aleph_MonoVoice_init(Custom_Aleph_MonoVoice *const synth, t_Aleph *const aleph) {

    Custom_Aleph_MonoVoice_init_to_pool(synth, &aleph->mempool);
    wavtab_index = 0;
    data_sdram = (fract32 *)SDRAM_ADDRESS;
}

void Custom_Aleph_MonoVoice_init_to_pool(Custom_Aleph_MonoVoice *const synth,
                                  Mempool *const mempool) {

    t_Mempool *mp = *mempool;

    t_Custom_Aleph_MonoVoice *syn = *synth =
        (t_Custom_Aleph_MonoVoice *)mpool_alloc(sizeof(t_Custom_Aleph_MonoVoice), mp);

    syn->mempool = mp;

    syn->freq_offset = Custom_Aleph_MonoVoice_DEFAULT_FREQ_OFFSET;
    syn->filter_type = Custom_Aleph_MonoVoice_DEFAULT_FILTER_TYPE;

    Aleph_Waveform_init_to_pool(&syn->waveform, mempool);

    Aleph_FilterSVF_init_to_pool(&syn->filter, mempool);

    Aleph_HPF_init_to_pool(&syn->dc_block, mempool);

    Aleph_LPFOnePole_init_to_pool(&syn->freq_slew, mempool);
    Aleph_LPFOnePole_set_output(&syn->freq_slew, Custom_Aleph_MonoVoice_DEFAULT_FREQ);

    Aleph_LPFOnePole_init_to_pool(&syn->cutoff_slew, mempool);
    Aleph_LPFOnePole_set_output(&syn->cutoff_slew,
                                Custom_Aleph_MonoVoice_DEFAULT_CUTOFF);

    Aleph_LPFOnePole_init_to_pool(&syn->amp_slew, mempool);
    Aleph_LPFOnePole_set_output(&syn->amp_slew, Custom_Aleph_MonoVoice_DEFAULT_AMP);
}

void Custom_Aleph_MonoVoice_free(Custom_Aleph_MonoVoice *const synth) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_Waveform_free(&syn->waveform);

    Aleph_FilterSVF_free(&syn->filter);

    Aleph_HPF_free(&syn->dc_block);

    Aleph_LPFOnePole_free(&syn->freq_slew);
    Aleph_LPFOnePole_free(&syn->cutoff_slew);

    mpool_free((char *)syn, syn->mempool);
}



fract32 custom_Aleph_Waveform_next(Aleph_Waveform *const wave, fract32 morph_amount) {

    t_Aleph_Waveform *wv = *wave;

    fract32 next;

    Aleph_Phasor_next(&wv->phasor);
        next =  wavetable_lookup_delta(wv->phasor->phase, morph_amount);
    
    return shl_fr1x32(next, 16);
}
fract32 Custom_Aleph_MonoVoice_next(Custom_Aleph_MonoVoice *const synth) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    fract32 output;

    fract32 amp;
    fract32 freq;
    fract32 cutoff;

    // Get slewed frequency.
    freq = Aleph_LPFOnePole_next(&syn->freq_slew);

    /// TODO: Set oscillator type (Dual, Unison, etc...).

    // Set oscillator frequency.
    Aleph_Waveform_set_freq(&syn->waveform, freq);

       // Get slewed cutoff.
    cutoff = Aleph_LPFOnePole_next(&syn->cutoff_slew);

    // Generate waveforms.
    //output = custom_Aleph_Waveform_next(&syn->waveform,syn->morph_amount);
    output = custom_Aleph_Waveform_next(&syn->waveform,cutoff);

    // Shift right to prevent clipping.
    output = shr_fr1x32(output, 1);

    // Get slewed amplitude.
    amp = Aleph_LPFOnePole_next(&syn->amp_slew);

    // Apply amp modulation.
    output = mult_fr1x32x32(output, amp);

 
    /*
    // Set filter cutoff.
    Aleph_FilterSVF_set_coeff(&syn->filter, cutoff);

    // Apply filter.
    switch (syn->filter_type) {

    case ALEPH_FILTERSVF_TYPE_LPF:
        output = Aleph_FilterSVF_lpf_next(&syn->filter, output);
        break;

    case ALEPH_FILTERSVF_TYPE_BPF:
        output = Aleph_FilterSVF_bpf_next(&syn->filter, output);
        break;

    case ALEPH_FILTERSVF_TYPE_HPF:
        output = Aleph_FilterSVF_hpf_next(&syn->filter, output);
        break;

    default:
        // Default to LPF.
        output = Aleph_FilterSVF_lpf_next(&syn->filter, output);
        break;
    }    // Block DC.
    output = Aleph_HPF_dc_block(&syn->dc_block, output);*/

    return output;
}



void Custom_Aleph_MonoVoice_set_shape(Custom_Aleph_MonoVoice *const synth, e_Aleph_Waveform_shape shape) {

    t_Custom_Aleph_MonoVoice *syn = *synth;
    Aleph_Waveform_set_shape(&syn->waveform, shape);
}


void Custom_Aleph_MonoVoice_set_amp(Custom_Aleph_MonoVoice *const synth, fract32 amp) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_LPFOnePole_set_target(&syn->amp_slew, amp);
}

void Custom_Aleph_MonoVoice_set_phase(Custom_Aleph_MonoVoice *const synth, fract32 phase) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_Waveform_set_phase(&syn->waveform, phase);
}

void Custom_Aleph_MonoVoice_set_freq(Custom_Aleph_MonoVoice *const synth, fract32 freq) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_LPFOnePole_set_target(&syn->freq_slew, freq);
}

void Custom_Aleph_MonoVoice_set_freq_offset(Custom_Aleph_MonoVoice *const synth,
                                     fract32 freq_offset) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    syn->freq_offset = freq_offset;
}

void Custom_Aleph_MonoVoice_set_morph_amount(Custom_Aleph_MonoVoice *const synth,fract32 morph_amount) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    syn->morph_amount = morph_amount;
}
void Custom_Aleph_MonoVoice_set_filter_type(Custom_Aleph_MonoVoice *const synth,
                                     e_Aleph_FilterSVF_type type) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    syn->filter_type = type;
}

void Custom_Aleph_MonoVoice_set_cutoff(Custom_Aleph_MonoVoice *const synth, fract32 cutoff) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_LPFOnePole_set_target(&syn->cutoff_slew, cutoff);
}

void Custom_Aleph_MonoVoice_set_res(Custom_Aleph_MonoVoice *const synth, fract32 res) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_FilterSVF_set_rq(&syn->filter, res);
}

void Custom_Aleph_MonoVoice_set_amp_slew(Custom_Aleph_MonoVoice *const synth,
                                  fract32 amp_slew) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_LPFOnePole_set_coeff(&syn->amp_slew, amp_slew);
}

void Custom_Aleph_MonoVoice_set_freq_slew(Custom_Aleph_MonoVoice *const synth,
                                   fract32 freq_slew) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_LPFOnePole_set_coeff(&syn->freq_slew, freq_slew);
}

void Custom_Aleph_MonoVoice_set_cutoff_slew(Custom_Aleph_MonoVoice *const synth,
                                     fract32 cutoff_slew) {

    t_Custom_Aleph_MonoVoice *syn = *synth;

    Aleph_LPFOnePole_set_coeff(&syn->cutoff_slew, cutoff_slew);
}
void Custom_Aleph_MonoVoice_record(fract32 data) {
    wavtab_index++;
    /*if (wavtab_index >= WAVE_TAB_SIZE) {
        wavtab_index = 0;  
    }
    wavtab[wavtab_index] = data;*/

/*    if (wavtab_index >= WAVE_TAB_SIZE_SDRAM) { 
            wavtab_index = 0;  
        }*/
        // store data in SDRAM
        data_sdram[wavtab_index] = data;
}
void Custom_Aleph_MonoVoice_record_reset() {
    wavtab_index = 0;
}
/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
