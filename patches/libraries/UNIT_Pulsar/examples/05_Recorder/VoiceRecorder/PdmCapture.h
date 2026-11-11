/*
  UNIT Electronics - Pulsar RP2350A
  Voice Recorder: PdmCapture.h

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#pragma once

// ============================================================
// PDM capture into a large ring buffer
// ============================================================
//
// The PDM library keeps only one block (~1.3 ms) and drops audio
// if it is not read in time. A microSD write can block for tens
// of milliseconds, so every block is copied from the PDM
// interrupt into a ring buffer in PSRAM (2 MB, ~21 s at 48 kHz),
// and loop() writes it to the card at its own pace.
//

#include <Arduino.h>
#include <PDM.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/timer.h>

namespace PdmCapture {

// Ring buffer, written by the PDM interrupt, read by loop()
int16_t *ring = nullptr;
uint32_t ringMask = 0;
volatile uint32_t head = 0;
volatile uint32_t tail = 0;
volatile uint32_t overruns = 0;

// Level meter
volatile int32_t peak = 0;
volatile bool clipped = false;

volatile uint32_t lastBlockUs = 0;
int32_t gainQ12 = 4096;
uint32_t sampleRate = 0;
bool running = false;


// Runs in the PDM DMA interrupt for every block
void onData() {

    int16_t block[128];
    const int n = PDM.read(block, sizeof(block)) / sizeof(int16_t);

    lastBlockUs = time_us_32();

    uint32_t h = head;
    int32_t p = peak;

    for (int i = 0; i < n; i++) {

        int32_t v = (block[i] * gainQ12 + 2048) >> 12;

        if (v > 32767) {
            v = 32767;
            clipped = true;
        } else if (v < -32768) {
            v = -32768;
            clipped = true;
        }

        const int32_t a = v < 0 ? -v : v;

        if (a > p) {
            p = a;
        }

        if (h - tail > ringMask) {
            overruns++;   // ring full: the card is too slow
            continue;
        }

        ring[h & ringMask] = static_cast<int16_t>(v);
        h++;
    }

    head = h;
    peak = p;
}


// Allocates the ring once: PSRAM if fitted, else a small SRAM one
bool allocate() {

    if (ring) {
        return true;
    }

    size_t samples = 1u << 20;   // 2 MB

    if (rp2040.getPSRAMSize()) {
        ring = static_cast<int16_t *>(pmalloc(samples * sizeof(int16_t)));
    }

    if (!ring) {
        samples = 1u << 15;      // 64 KB
        ring = static_cast<int16_t *>(malloc(samples * sizeof(int16_t)));
    }

    if (!ring) {
        return false;
    }

    ringMask = samples - 1;
    return true;
}


bool begin(uint32_t rate, int micGain, float gainDb, bool invertClk) {

    if (!allocate()) {
        return false;
    }

    PDM.end();
    PDM.onReceive(onData);
    PDM.setGain(micGain);

    gainQ12 = lroundf(4096.0f * powf(10.0f, gainDb / 20.0f));
    head = tail = 0;
    overruns = 0;
    peak = 0;
    clipped = false;

    if (PDM.begin(1, rate) != 1) {
        return false;
    }

    // The Pulsar's mic is a RIGHT mic (SELECT pulled high): it drives
    // DATA while CLK is low, so sample on the rising edge instead.
    if (invertClk) {
        gpio_set_outover(PIN_MIC_CLK, GPIO_OVERRIDE_INVERT);
    }

    // Restarting the PDM DMA must never wait behind another interrupt
    irq_set_priority(DMA_IRQ_0, PICO_HIGHEST_IRQ_PRIORITY);

    lastBlockUs = time_us_32();
    sampleRate = rate;
    running = true;
    return true;
}


void end() {
    PDM.end();
    running = false;
}


uint32_t available() {
    return head - tail;
}


uint32_t capacity() {
    return ringMask + 1;
}


// Copies up to maxSamples out of the ring
size_t read(int16_t *dst, size_t maxSamples) {

    size_t n = available();

    if (n > maxSamples) {
        n = maxSamples;
    }

    const uint32_t t = tail;

    for (size_t i = 0; i < n; i++) {
        dst[i] = ring[(t + i) & ringMask];
    }

    tail = t + n;
    return n;
}


// Drops everything captured so far
void discard() {
    tail = head;
}


// Peak since the last call, 0..32767
int32_t takePeak() {
    const int32_t p = peak;
    peak = 0;
    return p;
}


bool takeClipped() {
    const bool c = clipped;
    clipped = false;
    return c;
}


// Reading the BOOT button pauses all interrupts for ~30 us. Doing it
// right after a PDM block arrived leaves the whole block period free,
// so the PDM interrupt is never delayed.
bool safeToPauseInterrupts() {
    return !running || (time_us_32() - lastBlockUs) < 400;
}

} // namespace PdmCapture
