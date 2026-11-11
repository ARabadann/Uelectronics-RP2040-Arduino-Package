# USB Audio patch (UNIT Pulsar RP2350A)

UAC2 microphone, 16-bit / mono, isochronous IN. The host selects
16, 24, 44.1 or 48 kHz (Windows: Sound > Properties > Format); the sketch
learns it through `USBAudio.sampleRateChanged()` / `sampleRate()`.

| File | Destination in the core |
|---|---|
| `cores/rp2040/USBAudio.cpp`, `USBAudio.h` | `cores/rp2040/` |
| `include/tusb_config.h` | `include/` (enables `CFG_TUD_AUDIO`, sizes the endpoint for 48 kHz) |
| `lib/rp2350/libpico.a` | `lib/rp2350/` |
| `libraries/PDM/src/rp2040/PDM.cpp` | `libraries/PDM/src/rp2040/` |

`PDM.cpp` fixes the clock limit check: it compared
`sampleRate * decimation * 2` against 3.25 MHz, rejecting 48 kHz although
the real PDM clock is `48000 * 64 = 3.072 MHz`. It also resets the
decimation on every `begin()`, so switching 48 kHz -> 24 kHz works.
Both rates now run the microphone at 3.072 MHz (x128 at 24 kHz, x64 at 48 kHz).

## Why `libpico.a` is included

TinyUSB is precompiled into `libpico.a` using `include/tusb_config.h`.
The stock 3.0.0 library was built with `CFG_TUD_AUDIO = 0`, so it has no
audio class driver. This one is rebuilt from the **unmodified** pico-sdk
TinyUSB sources of 3.0.0 with this `tusb_config.h`. No TinyUSB source is patched.

Only RP2350 is rebuilt. RP2040 boards keep the stock library and cannot use `USBAudio`.

## Rebuilding on Windows (Ninja)

```bash
export PATH="$LOCALAPPDATA/Arduino15/packages/Uelectronics/tools/pqt-gcc/5.0.0-9576866/bin:$PATH"
cd tools/libpico/build-rp2350
CPU=rp2350 ninja libpico-rp2350.a
```

The `ar d` post-build steps in `tools/libpico/CMakeLists.txt` list `*.c.o`
members, but Ninja on Windows names them `*.c.obj`, so they are not removed.
Remove them by hand afterwards, or the core's own implementations collide:

```bash
arm-none-eabi-ar d ../../../lib/rp2350/libpico.a \
  hid_device.c.obj midi_device.c.obj msc_device.c.obj ncm_device.c.obj \
  new_delete.cpp.obj newlib_interface.c.obj stdio.c.obj stdio_uart.c.obj stdlib.c.obj
```

## Driver workarounds in `USBAudio.cpp`

- The clock source descriptor is patched to an internal *programmable*
  clock with a read/write frequency. TinyUSB's
  `TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR` declares a fixed, read-only clock:
  Windows lists every rate from GET RANGE but never sends SET CUR, so
  only the rate GET CUR reports (48 kHz) could be opened.
- The Feature Unit volume (-20 dB .. +30 dB) and mute are applied in
  `write()`. Windows hands its input-level slider to a device that
  declares hardware volume, so ignoring it made the slider do nothing.
  It starts at +10 dB, which Windows adopts as the initial slider
  position (a voice at ~25 cm then peaks around -10 dBFS). Windows
  caches this range: after changing it, uninstall the device once.
- `tud_audio_set_itf_close_EP_cb` disarms the ISO IN buffer on alt 0.
  Otherwise the RP2040/RP2350 DCD panics (`ep 81 was already available`)
  on the next alt 1, which Windows triggers when its sound settings open.
- `write()` holds `USB.mutex` and calls `tud_task()`, like `SerialUSB`.
  The core runs `tud_task()` from a 1 ms IRQ that skips while the mutex is
  held, and that IRQ can phase-lock with a 1 ms producer loop.
- `task()` must be called every `loop()` while streaming. The next ISO
  packet is only armed when `tud_task()` handles the previous completion;
  the 1 ms timer alone missed ~4 % of the frames.
- Producers should keep `queued()` near `bufferSize() / 2`: TinyUSB's flow
  control sizes packets around that level, absorbing the clock drift
  between the board and the computer.
