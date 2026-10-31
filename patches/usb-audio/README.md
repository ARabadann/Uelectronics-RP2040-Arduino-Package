# USB Audio patch (UNIT Pulsar RP2350A)

UAC2 microphone, 24 kHz / 16-bit / mono, isochronous IN.

| File | Destination in the core |
|---|---|
| `cores/rp2040/USBAudio.cpp`, `USBAudio.h` | `cores/rp2040/` |
| `include/tusb_config.h` | `include/` (enables `CFG_TUD_AUDIO`) |
| `lib/rp2350/libpico.a` | `lib/rp2350/` |

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

- `tud_audio_set_itf_close_EP_cb` disarms the ISO IN buffer on alt 0.
  Otherwise the RP2040/RP2350 DCD panics (`ep 81 was already available`)
  on the next alt 1, which Windows triggers when its sound settings open.
- `write()` holds `USB.mutex` and calls `tud_task()`, like `SerialUSB`.
  The core runs `tud_task()` from a 1 ms IRQ that skips while the mutex is
  held, and that IRQ can phase-lock with a 1 ms producer loop.
