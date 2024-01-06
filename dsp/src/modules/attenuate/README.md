## Freetribe Attenuate

This is a simple template module that does very little. 

It has 2 parameters:

- Amplitude for each input channel

Input is scaled and copied to the corresponding output channel.


### Customisation

This module serves as a base for customisation.

#### Custom Module Files

- attenuate.c
    - User defined initialisation, frame processing, control change processing.

#### Freetribe Kernel Files

- module.h
    - Required inclusion.  Does not need modification.

#### Library Files

- aleph_dsp/filter_1p.h
- aleph_dsp/filter_1p.c
    - A simple 1-pole filter class, useful for parameter smoothing.
    Included here as an example of how to import and use classes from aleph_dsp.
    
    - You may also want to make your own unit generators,
    consider adding them to the lib!

**WARNING: You can fry your blackfin by changing Freetribe kernel sources.
In particular, do not mess with clock settings in init.c!
Unless you really know what you are doing!**

For support, or to share your work, 
see the [Freetribe discussion forum](https://github.com/bangcorrupt/freetribe/discussions) on Github.

With thanks to @catfact, @tehn, @monome.
