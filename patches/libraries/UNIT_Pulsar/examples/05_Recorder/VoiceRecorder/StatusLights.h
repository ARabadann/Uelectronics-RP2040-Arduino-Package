/*
  UNIT Electronics - Pulsar RP2350A
  Voice Recorder: StatusLights.h

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#pragma once

// ============================================================
// Three on-board NeoPixels (PIO) and the built-in LED
// ============================================================
//
// WS2812 driver based on the Raspberry Pi Pico SDK example, the
// same one used by 02_NeoPixel/NeoPixelEffects.
//

#include <Arduino.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>

static const uint16_t ws2812Instructions[] = {
    0x6221, // out x, 1      side 0 [2]
    0x1123, // jmp !x, 3     side 1 [1]
    0x1400, // jmp 0         side 1 [4]
    0xa442  // nop           side 0 [4]
};

static const pio_program_t ws2812Program = {
    .instructions = ws2812Instructions,
    .length = 4,
    .origin = -1
};

class StatusLights {

public:

    bool begin(uint8_t brightness) {

        _brightness = brightness;
        pinMode(LED_BUILTIN, OUTPUT);

        static PIOProgram program(&ws2812Program);
        int sm, offset;

        if (!program.prepare(&_pio, &sm, &offset, PIN_NEOPIXEL)) {
            return false;
        }

        _sm = sm;

        pio_gpio_init(_pio, PIN_NEOPIXEL);
        pio_sm_set_consecutive_pindirs(_pio, _sm, PIN_NEOPIXEL, 1, true);

        pio_sm_config c = pio_get_default_sm_config();
        sm_config_set_wrap(&c, offset, offset + 3);
        sm_config_set_sideset(&c, 1, false, false);
        sm_config_set_sideset_pins(&c, PIN_NEOPIXEL);
        sm_config_set_out_shift(&c, false, true, 24);
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
        sm_config_set_clkdiv(&c, clock_get_hz(clk_sys) / (800000.0f * 10));

        pio_sm_init(_pio, _sm, offset, &c);
        pio_sm_set_enabled(_pio, _sm, true);

        _ready = true;
        clear();
        show();
        return true;
    }

    void setPixel(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {

        if (i >= NUM_NEOPIXEL) {
            return;
        }

        _rgb[i][0] = r;
        _rgb[i][1] = g;
        _rgb[i][2] = b;
    }

    void fill(uint8_t r, uint8_t g, uint8_t b) {
        for (uint8_t i = 0; i < NUM_NEOPIXEL; i++) {
            setPixel(i, r, g, b);
        }
    }

    void clear() {
        fill(0, 0, 0);
    }

    void show() {

        if (!_ready) {
            return;
        }

        for (uint8_t i = 0; i < NUM_NEOPIXEL; i++) {

            const uint32_t r = _rgb[i][0] * _brightness / 255;
            const uint32_t g = _rgb[i][1] * _brightness / 255;
            const uint32_t b = _rgb[i][2] * _brightness / 255;

            pio_sm_put_blocking(_pio, _sm, ((g << 16) | (r << 8) | b) << 8);
        }
    }

    void led(bool on) {
        digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
    }

private:

    PIO _pio = nullptr;
    uint _sm = 0;
    bool _ready = false;
    uint8_t _brightness = 40;
    uint8_t _rgb[NUM_NEOPIXEL][3] = {};
};
