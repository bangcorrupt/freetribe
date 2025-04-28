# Freetribe

Freetribe is a free, open-source firmware for Electribe 2, built from the ground
up using original code and open-source projects. Freetribe aims to provide a
user-friendly API for connecting control input to audio processing.

See
[Getting Started](https://bangcorrupt.github.io/freetribe-docs/getting-started/)
to jump right into the Freetribe API, or
[Architecture](https://bangcorrupt.github.io/freetribe-docs/architecture/) for a
deeper look at the system.

## Features

From a user application perspective, Freetribe is currently light on features.
Most of the hardware initialisation is complete, with driver stacks for much of
the system. Built on this is a set of services providing a high-level interface
to the device. Some basic examples are provided, showing how to integrate user
application code with the Freetribe kernel.

### Existing Features

- Serial MIDI input and output via TRS port.
- Set or clear a pixel anywhere in the vast 128x64 dot-matrix.
- Control backlight RGB (binary).
- Register callbacks for all of the panel controls.
- Set and toggle LEDs, with brightness control for those with support.
- Send commands to the Blackfin DSP and receive feedback.
- Process audio based on control input.
- Audio module API similar to many plugin formats.

### Planned Features

Some of these are in progress, most should be possible.

- High speed DSP control via EMIFA/HostDMA.
- DMA support for peripheral drivers.
- USB driver.
- SD card driver.
- DSP block processing.
- Memory protection.
- Dynamic loading of apps and modules.
- Preemptive scheduling using FreeRTOS.
- Embedded Lua and MicroPython.
- Support for sync ports.

## Support for You

If you need help with this project, please visit the
[Freetribe discussion forum](https://github.com/bangcorrupt/freetribe/discussions).

## Support for Me

Freetribe is free (as in GPL) and always will be. If you would like to support
my work you are most welcome to
[become a sponsor](https://github.com/sponsors/bangcorrupt). Freetribe exists
because people sponsored Hacktribe. Your support helps keep me motivated,
fuelled and focussed.

## Acknowledgements

Freetribe would be almost impossible without other open-source projects. The CPU
drivers are based on [StarterWare](https://www.ti.com/tool/STARTERWARE-SITARA)
by Texas Instruments. The hardware abstraction, build environment and code
examples provided the stepping-stone needed to get started.

In much the same way, the DSP drivers are based on
[monome/aleph](https://github.com/monome/aleph). This showed how to initialise
the Blackfin processor and configure peripherals. They also provide a public
domain DSP library, with many of the difficult maths problems packaged into
convenient unit generators.

MIDI input parsing is based on
[mikromodular/libmidi](https://github.com/mikromodular/libmidi), with
sysex/binary conversion borrowed from the
[Arduino MIDI library](https://github.com/FortySevenEffects/arduino_midi_library/blob/master/src/MIDI.cpp)
by Francois Best.

[UGUI](https://github.com/deividAlfa/UGUI) and
[micromenu](https://github.com/abcminiuser/micromenu-v2) provide a graphical
interface for user applications.

Special thanks to countless stackoverflow users.

## License

AGPL-3.0.
