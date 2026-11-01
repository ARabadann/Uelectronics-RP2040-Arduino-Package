/*
  UNIT Electronics - Pulsar RP2350A
  USB Microphone

  Turns the Pulsar into a plug-and-play USB microphone using the
  on-board ICS-41350 PDM microphone. No drivers are needed on
  Windows, macOS, Linux or Android.

  - 16-bit PCM, mono
  - 16, 24, 44.1 or 48 kHz, selected from the computer
    (Windows: Settings > System > Sound > Microphone > Format)

  - Input level from the computer, -20 dB .. +30 dB
    (Windows: Settings > System > Sound > Microphone > Input volume)

  Built-in LED: fast blink = waiting for USB, short flash every
  second = connected, steady = recording.

  Open the Serial Monitor at 115200 baud to see the level meter.
  The peak shown is before the computer's input volume.
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

#include <Arduino.h>
#include <PDM.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include "USBAudio.h"

// ============================================================
// Configuration
// ============================================================

// OpenPDM filter gain. The filter saturates at +/-32700, so too
// much gain makes loud voices sound raspy. Raise it for distant
// sources, lower it if the peak reaches ~99 %.
constexpr int MIC_GAIN = 2;

// The PDM library samples DATA on the falling edge of CLK, which
// suits a LEFT microphone. On the Pulsar, SELECT is pulled high,
// so the mic is RIGHT and drives DATA while CLK is low. Inverting
// the CLK pad moves the sample point to the rising edge.
constexpr bool INVERT_PDM_CLK = true;

// ============================================================
// Ring buffer between the PDM filter and USB
// ============================================================
//
// The PDM library drops a whole block if the previous one has
// not been read in time, so drain it immediately into this ring
// and feed USB from here.
//

constexpr size_t RING_SIZE = 4096;   // power of two

int16_t ring[RING_SIZE];
size_t ringHead = 0;
size_t ringTail = 0;

size_t ringCount() {
    return (ringHead - ringTail) & (RING_SIZE - 1);
}

int16_t block[128];

// ============================================================
// Statistics
// ============================================================

uint32_t micRate = 0;
uint32_t samplesIn = 0;
uint32_t samplesOut = 0;
uint32_t drops = 0;
int16_t peak = 0;
uint32_t lastReport = 0;

// ============================================================
// Status LED
// ============================================================
//
//   fast blink        waiting for the computer (no USB)
//   short flash / s   connected, idle
//   steady on         recording
//   very fast blink   microphone error
//

void updateLed() {

    const uint32_t t = millis();
    bool on;

    if (!USBAudio.mounted()) {
        on = (t % 250) < 125;
    } else if (USBAudio.streaming()) {
        on = true;
    } else {
        on = (t % 1000) < 50;
    }

    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}

// ============================================================
// Microphone
// ============================================================

void startMicrophone(uint32_t rate) {

    PDM.end();

    PDM.setGain(MIC_GAIN);

    if (!PDM.begin(1, rate)) {
        while (true) {
            Serial.println("PDM.begin() failed");
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(100);
        }
    }

    if (INVERT_PDM_CLK) {
        gpio_set_outover(PIN_MIC_CLK, GPIO_OVERRIDE_INVERT);
    }

    // The PIO RX FIFO holds only a few tens of microseconds of PDM
    // bits. The PDM DMA IRQ must preempt the USB task IRQ.
    irq_set_priority(DMA_IRQ_0, PICO_HIGHEST_IRQ_PRIORITY);

    ringTail = ringHead;
    micRate = rate;
}

// ============================================================
// Setup
// ============================================================

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    startMicrophone(USBAudio.sampleRate());

    lastReport = millis();
}

// ============================================================
// Loop
// ============================================================

void loop() {

    // --------------------------------------------------------
    // 1. The computer picked another sample rate
    // --------------------------------------------------------

    if (USBAudio.sampleRateChanged()) {
        startMicrophone(USBAudio.sampleRate());
    }

    // --------------------------------------------------------
    // 2. Drain the PDM filter into the ring
    // --------------------------------------------------------

    while (PDM.available()) {

        const size_t n =
            PDM.read(block, sizeof(block)) / sizeof(int16_t);

        samplesIn += n;

        for (size_t i = 0; i < n; i++) {

            const int16_t s = block[i];
            const int16_t a = s < 0 ? -s : s;

            if (a > peak) {
                peak = a;
            }

            if (ringCount() == RING_SIZE - 1) {
                ringTail = (ringTail + 1) & (RING_SIZE - 1);
                drops++;
            }

            ring[ringHead] = s;
            ringHead = (ringHead + 1) & (RING_SIZE - 1);
        }
    }

    // --------------------------------------------------------
    // 3. Feed USB
    // --------------------------------------------------------
    //
    // task() arms the next USB packet as soon as the previous one
    // left. TinyUSB sizes each packet to keep its FIFO about half
    // full, which absorbs the clock difference with the computer.
    //

    USBAudio.task();

    const size_t target = USBAudio.bufferSize() / 2;
    const size_t queued = USBAudio.queued();

    if (!USBAudio.streaming()) {
        ringTail = ringHead;   // nobody recording
    } else if (queued < target) {

        size_t n = target - queued;
        const size_t avail = ringCount();
        const size_t toEnd = RING_SIZE - ringTail;

        if (n > avail) {
            n = avail;
        }
        if (n > toEnd) {
            n = toEnd;
        }

        if (n) {
            const size_t written = USBAudio.write(ring + ringTail, n);
            ringTail = (ringTail + written) & (RING_SIZE - 1);
            samplesOut += written;
        }
    }

    updateLed();

    // --------------------------------------------------------
    // 4. Once per second: level meter
    // --------------------------------------------------------

    if (millis() - lastReport >= 1000) {
        lastReport = millis();

        const float level = 100.0f * peak / 32768.0f;

        Serial.print(micRate / 1000.0f, 1);
        Serial.print(" kHz  ");
        Serial.print(USBAudio.streaming() ? "REC" : "idle");
        Serial.print("  vol=");
        if (USBAudio.muted()) {
            Serial.print("mute");
        } else {
            Serial.print(USBAudio.volumeDb(), 0);
            Serial.print("dB");
        }
        Serial.print("  in=");
        Serial.print(samplesIn);
        Serial.print("  out=");
        Serial.print(samplesOut);
        Serial.print("  drops=");
        Serial.print(drops);
        Serial.print("  peak=");
        Serial.print(level, 1);
        Serial.print("%  ");

        for (int i = 0; i < 40; i++) {
            Serial.print(i < level * 0.4f ? '#' : '.');
        }
        Serial.println();

        samplesIn = 0;
        samplesOut = 0;
        peak = 0;
    }
}
