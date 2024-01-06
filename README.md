# Freetribe

Freetribe is a free, open-source firmware for Electribe 2,
built from the ground up using original code and open-source projects.
Freetribe aims to provide a user-friendly API for connecting control input to audio processing.

## Features

From a user application perspective, Freetribe is currently light on features.
Most of the hardware initialisation is complete, with driver stacks for much of the system.
Built on this is a set of services providing a high-level interface to the device.
Some basic examples are provided, showing how to integrate user application code with the Freetribe kernel.

### CPU Kernel

The CPU kernel initialises the hardware, processes communication with the outside world
and controls the Blackfin DSP via a serial command interface.

- **MIDI service**

  - Serial MIDI input and output via TRS port.
  - Register callbacks for each type of message received.
  - Send simple messages such as note and CC.

- **Display service**

  - Set or clear a pixel anywhere in the vast 128x64 dot-matrix.
  - Control backlight RGB (binary).

- **Panel service**

  - Register callbacks for all of the panel controls.
  - Set and toggle LEDs, with brightness control for those with support.

- **System service**

  - Aggregates useful things like print and timing.

- **DSP service**

  - Send commands to the Blackfin DSP and receive feedback.

### DSP Kernel

The DSP kernel receives commands from the CPU and processes audio frames.
A user defined module runs in the audio callback,
with an interface similar to many plugin formats.

### Examples

The main demo uses the volume knob to control attenuation of audio pass-through,
while sending MIDI CC and printing to the display. An LED blinks at regular intervals.

Future examples will deal separately with individual features,
building up to a more complex application.

### Planned Features

Some of these are in progress, most should be possible.

- **High speed DSP control**

  - CPU EMIFA is connected to DSP HostDMA with a 16 bit parallel interface.
  - (Currently Freetribe uses SPI to control DSP).

- **DMA driver**

  - Implement CPU DMA driver and update device drivers.

- **USB driver**

  - Port TinyUSB.

- **SD card driver**

  - Port FatFS.

- **DSP block processing**

  - Implement block processing using DMA linked descriptors.

- **Cache and memory protection**

  - Currently everything runs in system mode with no cacheing.

- **Dynamic linking**

  - CPU app and DSP module are currently compiled into the kernels.
  - Implement as DLL to allow runtime switching of apps/modules/plugins.

- **Preemptive scheduling**

  - Port FreeRTOS.

- **MicroPython**

  - As FreeRTOS task.

- **Sync ports**

  - Probably debounced GPIO, haven't checked.

## Building

If you're reading this on Github, create a codespace from this repo, then run:

```
make clean && make
```

There will be a lot of warnings about incompatible types and implicit declarations, but there should be no errors.

Alternatively, if you have `docker compose` installed on your local machine,
change to the `freetribe` directory and build the docker image:

```
docker compose build
```

Then start a container:

```
docker compose up -d
```

The current working directory will be mounted in the container at `/freetribe`.

Now we can run `make` in the container to build Freetribe.

```
docker compose exec freetribe make clean
docker compose exec freetribe make
```

The final binary will be at `freetribe/cpu/build/cpu.bin`.

This includes the DSP firmware, which is sent to the Blackfin by the CPU, via SPI.

I will add documentation for setting up the build environment locally,
but you can probably work it out from the commands in the Dockerfile.

## Debugging

See the [Hacktribe Debrick Guide](https://github.com/bangcorrupt/hacktribe/wiki/Debrick#rpi-and-openocd) for more details on getting a debugger attached.

### CPU

For the CPU, a JLink EDU and JLinkGDBServer works well.
We can also use a Raspberry Pi and OpenOCD.

If using JLink, connect the debugger to USB host,
then power electribe using modified power switch. Then run:

```
JLinkGDBServer -device am1802 -speed 0
```

If using Raspberry Pi, OpenOCD will exit if the electribe is not powered,
but the electribe will boot if OpenOCD is not attached,
so we need some trickery. First power up electribe, then run OpenOCD,
then power off electribe with OpenOCD still running.
Then power up electribe again before restarting OpenOCD.
It is a lot easier to use a JLink.

Once you have a gdb server attached to the CPU,
change directory to `freetribe/cpu` and run:

```
arm-none-eabi-gdb
```

The commands in `freetribe/cpu/.gdbinit` should connect to the gdb server,
load the symbols from `cpu/build/cpu.elf` and run the firmware.
You may need to edit the port number in `.gdbinit`. OpenOCD uses `3333` by default,
with JLinkGDBServer using `2331`.

If all is well, you should hit a breakpoint at `main`.  
Other useful breakpoints may be `knl_init`, `knl_run`, `app_init` and `app_run`.

### DSP

For the DSP, I've only managed to get the official debugger from Analog Devices working.  
The ADSP-ICE-1000 costs around £180 (thank you sponsors) and is very much a one-trick pony.
I will try again to get the Raspberry Pi set up with the DSP,
but I'm not sure if the Analog Devices OpenOCD fork is even supposed to support this.

The good news is, the DSP firmware is loaded by the CPU.
So if you can deal without debugging you can still load and run your code.

If you have a debugger for the DSP, first run the binary on the CPU.
By the time the CPU reaches `app_run` the DSP kernel will be running.

Change directory to `freetribe/dsp` and run:

```
sudo openocd-bfin -s /usr/share/openocd-bfin/scripts \
                  -f /usr/share/openocd-bfin/scripts/interface/ice1000.cfg \
                  -f /usr/share/openocd-bfin/scripts/target/bf527.cfg
```

Then in another shell in the same directory:

```
bfin-elf-gdb
```

The commands in `freetribe/dsp/.gdbinit` should connect to the gdb server,
load the symbols from `freetribe/dsp/build/bfin.elf` and
set a breakpoint at `module_process`, the audio processing callback.

I will try to add new sysex functions to Hacktribe,
so we can load and execute code without needing a debugger.

## Sponsor

Freetribe is free (as in GPL) and always will be.
If you would like to support my work you are most welcome to become a sponsor.
Freetribe exists because people sponsored Hacktribe.

## Acknowledgements

Freetribe would be almost impossible without other open-source projects.
The CPU drivers are based on [StarterWare](https://www.ti.com/tool/STARTERWARE-SITARA) by Texas Instruments. The hardware abstraction,
build environment and code examples provided the stepping-stone needed to get started.

In much the same way, the DSP drivers are based on [monome/aleph](https://github.com/monome/aleph).
This showed how to initialise the Blackfin processor and configure peripherals.
They also provide a public domain DSP library, with many of the difficult
maths problems packaged into convenient unit generators.

MIDI input parsing is based on [mikromodular/libmidi](https://github.com/mikromodular/libmidi),
with sysex/binary conversion borrowed from the [Arduino MIDI library](https://github.com/FortySevenEffects/arduino_midi_library/blob/master/src/MIDI.cpp) by Francois Best.

[UGUI](https://github.com/deividAlfa/UGUI) and [micromenu](https://github.com/abcminiuser/micromenu-v2) provide a graphical interface for user applications.

Special thanks to countless stackoverflow users.

## License

AGPL-3.0-or-later.
