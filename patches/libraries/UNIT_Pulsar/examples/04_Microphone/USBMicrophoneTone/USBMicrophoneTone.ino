/*
  UNIT Electronics - Pulsar RP2350A
  USB Microphone Tone Test

  Sends a 1 kHz sine wave (-12 dBFS) as a USB microphone. Use it to
  check the USB audio path without the PDM microphone: record with
  Audacity and you should see a clean sine wave.

  - 16-bit PCM, mono
  - 16, 24, 44.1 or 48 kHz, selected from the computer

  Note: Windows voice enhancements ("Voice Clarity", "Voice Focus")
  remove steady tones as noise. Disable them or record with Audacity.

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

#include <Arduino.h>
#include "USBAudio.h"
#include <math.h>

// ============================================================
// Configuration
// ============================================================

constexpr float TONE_FREQ = 1000.0f;
constexpr float AMPLITUDE = 8192.0f;   // 25 % of full scale

// ============================================================
// Oscillator
// ============================================================

float phase = 0.0f;
float phaseStep = 0.0f;

void setRate(uint32_t rate) {
    phaseStep = 2.0f * PI * TONE_FREQ / rate;
}

int16_t nextSample() {

    const int16_t s =
        static_cast<int16_t>(sinf(phase) * AMPLITUDE);

    phase += phaseStep;

    if (phase >= 2.0f * PI) {
        phase -= 2.0f * PI;
    }

    return s;
}

// ============================================================
// Setup
// ============================================================

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);

    setRate(USBAudio.sampleRate());
}

// ============================================================
// Loop
// ============================================================

void loop() {

    if (USBAudio.sampleRateChanged()) {
        setRate(USBAudio.sampleRate());
    }

    // Arm the next USB packet as soon as the previous one left
    USBAudio.task();

    // Keep the USB FIFO about half full. The host's pace sets
    // the rate, so the tone never drifts or overflows.
    const size_t target = USBAudio.bufferSize() / 2;

    int16_t frame[32];

    while (USBAudio.streaming() && USBAudio.queued() < target) {

        size_t n = target - USBAudio.queued();

        if (n > 32) {
            n = 32;
        }

        for (size_t i = 0; i < n; i++) {
            frame[i] = nextSample();
        }

        USBAudio.write(frame, n);
    }

    // LED on = USB connected
    digitalWrite(LED_BUILTIN, USBAudio.mounted() ? HIGH : LOW);
}
