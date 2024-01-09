/* filter_svf.h
   audio library
   aleph

   a digital state-variable filter for 32-bit fixed point audio.
*/

#ifndef _ALEPH_AUDIO_FILTER_SVF_H_
#define _ALEPH_AUDIO_FILTER_SVF_H_


#include "fix.h"
#include "types.h"


//==============================================
//===== types

typedef struct _filter_svf {
  fract32 freq;  // normalized frequency
  fract32 rq;    // reciprocal of q (resonance / bandwidth)
                 // range is [0, 2]
  fract32 low;   // lowpass
  fract32 band;  // bandpass
  fract32 high;
  fract32 notch;
  
  // output mix
  fract32 lowMix;
  fract32 highMix;
  fract32 bandMix;
  fract32 notchMix;
  fract32 peakMix;
  
  // kinda retarded, but use rshift for rq values >=1
  u8 rqShift;

} filter_svf;


//=============================================
//===== functions
// init
extern void filter_svf_init      ( filter_svf* f );
// set cutoff in hz
//extern void filter_svf_set_hz    ( filter_svf* f, fix16 hz );
// set cutoff coefficient
extern void filter_svf_set_coeff    ( filter_svf* f, fract32 coeff );

// set RQ (reciprocal of q: resonance/bandwidth)
extern void filter_svf_set_rq ( filter_svf* f, fract32 rq );
// set output mixes
extern void filter_svf_set_low   ( filter_svf* f, fract32 mix );
extern void filter_svf_set_high  ( filter_svf* f, fract32 mix );
extern void filter_svf_set_band  ( filter_svf* f, fract32 mix );
extern void filter_svf_set_notch ( filter_svf* f, fract32 mix );
extern void filter_svf_set_peak ( filter_svf* f, fract32 mix );
// get next value (with input)

typedef fract32 (*svf_func_t) ( filter_svf* f, fract32 in);

extern fract32 filter_svf_next( filter_svf* f, fract32 in );
extern fract32 filter_svf_hpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_bpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_lpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_notch_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_softclip_hpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_softclip_bpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_softclip_lpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_softclip_notch_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_softclip_asym_lpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_softclip_asym_bpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_softclip_asym_hpf_next( filter_svf* f, fract32 in);
extern fract32 filter_svf_softclip_asym_notch_next( filter_svf* f, fract32 in);

const extern svf_func_t svf_funcs[3][4];

#endif // h guard
