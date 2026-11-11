/*
  UNIT Electronics - Pulsar RP2350A
  ==================================
  Example: Native NeoPixel Effects

  Description:
  This example demonstrates how to control the three on-board
  addressable RGB LEDs of the UNIT Pulsar RP2350A.

  The NeoPixels are driven directly by the RP2350A Programmable
  I/O (PIO), using the Raspberry Pi Pico SDK included with the
  Arduino-Pico core. No external NeoPixel library is required.

  The demonstration includes:
    - RGB color cycle
    - Running light
    - RGB chase
    - Scanner effect
    - Breathing effect
    - Pulsar energy effect
    - UNIT startup sequence

  Hardware:
    Board       : UNIT Pulsar RP2350A
    MCU         : Raspberry Pi RP2350A
    NeoPixels   : 3 x WS2812-compatible RGB LEDs
    Data Pin    : GPIO1 (PIN_NEOPIXEL)
    Interface   : RP2350 PIO
    Frequency   : 800 kHz

  Board definitions used:
    PIN_NEOPIXEL
    NUM_NEOPIXEL

  Requirements:
    - UNIT Pulsar RP2350A board selected in Arduino IDE
    - No external libraries required

  Notes:
    The example intentionally uses a reduced LED brightness to
    limit current consumption and provide comfortable visual effects.

  Based on the WS2812 PIO implementation from the Raspberry Pi
  Pico SDK examples.

  Developed for the UNIT Electronics Arduino RP2040/RP2350 Package.

  Copyright (c) 2026 UNIT Electronics
  SPDX-License-Identifier: BSD-3-Clause

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

#include <Arduino.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"

// ============================================================
// WS2812 PIO Program
// Based on the Raspberry Pi Pico SDK WS2812 example
// ============================================================

static const uint16_t ws2812_program_instructions[] = {
  0x6221, // out x, 1 side 0 [2]
  0x1123, // jmp !x, 3 side 1 [1]
  0x1400, // jmp 0 side 1 [4]
  0xa442  // nop side 0 [4]
};

static const struct pio_program ws2812_program = {
  .instructions = ws2812_program_instructions,
  .length = 4,
  .origin = -1
};


// ============================================================
// PIO Configuration
// ============================================================

static inline pio_sm_config ws2812_program_get_default_config(uint offset) {
  pio_sm_config c = pio_get_default_sm_config();

  sm_config_set_wrap(&c, offset + 0, offset + 3);
  sm_config_set_sideset(&c, 1, false, false);

  return c;
}


static inline void ws2812_program_init(
  PIO pio,
  uint sm,
  uint offset,
  uint pin,
  float freq
) {
  pio_gpio_init(pio, pin);

  pio_sm_set_consecutive_pindirs(
    pio,
    sm,
    pin,
    1,
    true
  );

  pio_sm_config c = ws2812_program_get_default_config(offset);

  sm_config_set_sideset_pins(&c, pin);

  sm_config_set_out_shift(
    &c,
    false,
    true,
    24
  );

  sm_config_set_fifo_join(
    &c,
    PIO_FIFO_JOIN_TX
  );

  int cycles_per_bit = 10;

  float divider =
    clock_get_hz(clk_sys) /
    (freq * cycles_per_bit);

  sm_config_set_clkdiv(&c, divider);

  pio_sm_init(
    pio,
    sm,
    offset,
    &c
  );

  pio_sm_set_enabled(
    pio,
    sm,
    true
  );
}


// ============================================================
// WS2812 Helpers
// ============================================================

static PIO pio = pio0;
static uint sm;
static uint offset;


static inline void putPixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(
    pio,
    sm,
    pixel_grb << 8u
  );
}


static inline uint32_t rgbToGRB(
  uint8_t r,
  uint8_t g,
  uint8_t b
) {
  return
    ((uint32_t)g << 16) |
    ((uint32_t)r << 8) |
    (uint32_t)b;
}


void clearPixels() {
  for (int i = 0; i < NUM_NEOPIXEL; i++) {
    putPixel(rgbToGRB(0, 0, 0));
  }

  delayMicroseconds(100);
}


void setAllPixels(
  uint8_t r,
  uint8_t g,
  uint8_t b
) {
  for (int i = 0; i < NUM_NEOPIXEL; i++) {
    putPixel(rgbToGRB(r, g, b));
  }

  delayMicroseconds(100);
}


void setSinglePixel(
  uint8_t pixel,
  uint8_t r,
  uint8_t g,
  uint8_t b
) {
  for (int i = 0; i < NUM_NEOPIXEL; i++) {

    if (i == pixel) {
      putPixel(rgbToGRB(r, g, b));
    } else {
      putPixel(rgbToGRB(0, 0, 0));
    }
  }

  delayMicroseconds(100);
}


// ============================================================
// Arduino Setup
// ============================================================

void setup() {

  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("UNIT Pulsar RP2350A");
  Serial.println("Native NeoPixel Effects");
  Serial.println("--------------------");

  Serial.print("NeoPixel GPIO: ");
  Serial.println(PIN_NEOPIXEL);

  Serial.print("Number of NeoPixels: ");
  Serial.println(NUM_NEOPIXEL);

  sm = pio_claim_unused_sm(
    pio,
    true
  );

  offset = pio_add_program(
    pio,
    &ws2812_program
  );

  ws2812_program_init(
    pio,
    sm,
    offset,
    PIN_NEOPIXEL,
    800000
  );

  clearPixels();
}


// ============================================================
// Arduino Loop
// ============================================================

void loop() {

  // ==========================================================
  // Effect 1: RGB Color Cycle
  // ==========================================================

  Serial.println("Effect 1 - RGB Color Cycle");

  setAllPixels(32, 0, 0);
  delay(400);

  setAllPixels(0, 32, 0);
  delay(400);

  setAllPixels(0, 0, 32);
  delay(400);

  setAllPixels(32, 16, 0);
  delay(400);

  setAllPixels(20, 0, 32);
  delay(400);

  clearPixels();
  delay(300);


  // ==========================================================
  // Effect 2: Running Light
  // ==========================================================

  Serial.println("Effect 2 - Running Light");

  for (int repeat = 0; repeat < 4; repeat++) {

    for (int i = 0; i < NUM_NEOPIXEL; i++) {
      setSinglePixel(i, 0, 20, 32);
      delay(120);
    }

    for (int i = NUM_NEOPIXEL - 2; i > 0; i--) {
      setSinglePixel(i, 0, 20, 32);
      delay(120);
    }
  }

  clearPixels();


  // ==========================================================
  // Effect 3: RGB Chase
  // ==========================================================

  Serial.println("Effect 3 - RGB Chase");

  for (int repeat = 0; repeat < 5; repeat++) {

    putPixel(rgbToGRB(32, 0, 0));
    putPixel(rgbToGRB(0, 32, 0));
    putPixel(rgbToGRB(0, 0, 32));
    delayMicroseconds(100);
    delay(180);

    putPixel(rgbToGRB(0, 0, 32));
    putPixel(rgbToGRB(32, 0, 0));
    putPixel(rgbToGRB(0, 32, 0));
    delayMicroseconds(100);
    delay(180);

    putPixel(rgbToGRB(0, 32, 0));
    putPixel(rgbToGRB(0, 0, 32));
    putPixel(rgbToGRB(32, 0, 0));
    delayMicroseconds(100);
    delay(180);
  }


  // ==========================================================
  // Effect 4: Scanner
  // ==========================================================

  Serial.println("Effect 4 - Scanner");

  for (int repeat = 0; repeat < 6; repeat++) {

    setSinglePixel(0, 32, 0, 8);
    delay(100);

    setSinglePixel(1, 32, 0, 8);
    delay(100);

    setSinglePixel(2, 32, 0, 8);
    delay(100);

    setSinglePixel(1, 32, 0, 8);
    delay(100);
  }

  clearPixels();


  // ==========================================================
  // Effect 5: Breathing
  // ==========================================================

  Serial.println("Effect 5 - Breathing");

  for (int brightness = 0; brightness <= 40; brightness++) {

    setAllPixels(
      0,
      brightness / 3,
      brightness
    );

    delay(20);
  }

  for (int brightness = 40; brightness >= 0; brightness--) {

    setAllPixels(
      0,
      brightness / 3,
      brightness
    );

    delay(20);
  }


  // ==========================================================
  // Effect 6: Pulsar Energy
  // ==========================================================

  Serial.println("Effect 6 - Pulsar Energy");

  for (int repeat = 0; repeat < 4; repeat++) {

    // Energy starts in the center
    setSinglePixel(1, 32, 0, 32);
    delay(120);

    // Expands to both sides
    putPixel(rgbToGRB(20, 0, 32));
    putPixel(rgbToGRB(32, 8, 32));
    putPixel(rgbToGRB(20, 0, 32));

    delayMicroseconds(100);
    delay(150);

    // Flash
    setAllPixels(32, 16, 32);
    delay(80);

    clearPixels();
    delay(120);
  }


  // ==========================================================
  // Effect 7: Startup-style sequence
  // ==========================================================

  Serial.println("Effect 7 - UNIT Startup");

  clearPixels();

  setSinglePixel(0, 0, 0, 32);
  delay(150);

  putPixel(rgbToGRB(0, 0, 32));
  putPixel(rgbToGRB(0, 16, 32));
  putPixel(rgbToGRB(0, 0, 0));
  delayMicroseconds(100);
  delay(150);

  putPixel(rgbToGRB(0, 0, 32));
  putPixel(rgbToGRB(0, 16, 32));
  putPixel(rgbToGRB(16, 0, 32));
  delayMicroseconds(100);

  delay(600);

  // Final white flash
  setAllPixels(32, 32, 32);
  delay(100);

  clearPixels();

  Serial.println("Sequence complete");
  Serial.println();

  delay(1000);
}