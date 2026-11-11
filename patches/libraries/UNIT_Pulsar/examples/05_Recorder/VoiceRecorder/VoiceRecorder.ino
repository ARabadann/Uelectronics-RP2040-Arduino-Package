/*
  UNIT Electronics - Pulsar RP2350A
  Voice Recorder

  Portable recorder: the on-board PDM microphone is recorded to
  the microSD card as WAV files (REC0001.WAV, REC0002.WAV, ...).
  Audio goes through an 8 MB PSRAM buffer, so slow card writes
  never cut the recording.

  BOOT button:
    short press   start / stop recording
    double press  add a marker (REC0001.txt, an Audacity label
                  track: File > Import > Labels)
    hold 2 s      save and sleep / wake up

  NeoPixels:
    blue               level meter, ready
    green-yellow-red   level meter while recording
    dim red dot        recording, silence
    white flash        marker added
    purple fill        holding BOOT towards sleep
    red blink          error (no card, card full, write failed)

  Built-in LED: short flash every second = ready,
  steady = recording, fast blink = error, off = sleeping.

  Without a card it still works as a level meter, and the Serial
  Monitor (115200 baud) shows what is happening.

  Card: microSD formatted FAT32 (or exFAT).

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

#include <Arduino.h>
#include <SD.h>
#include "PdmCapture.h"
#include "WavFile.h"
#include "BootButton.h"
#include "StatusLights.h"
#include "Battery.h"

// ============================================================
// Configuration
// ============================================================

// 16000 (voice, smallest files), 24000, 44100 or 48000 (best).
// 48 kHz uses ~5.5 MB per minute.
constexpr uint32_t SAMPLE_RATE = 48000;

// Digital gain after the microphone. +10 dB matches the USB
// microphone and suits a voice at ~25 cm. Lower it for loud
// sources if the meter turns red.
constexpr float RECORD_GAIN_DB = 10.0f;

// OpenPDM filter gain (see 04_Microphone/USBMicrophone)
constexpr int MIC_GAIN = 2;
constexpr bool INVERT_PDM_CLK = true;

// Battery monitor: -1 = none. Set an analog pin (A0..A3) once a
// 1:2 divider is fitted, see Battery.h. NEVER wire BAT directly.
constexpr int BATTERY_PIN = -1;
constexpr float BATTERY_DIVIDER = 2.0f;

// NeoPixel brightness, 0..255. Keep it low on battery.
constexpr uint8_t PIXEL_BRIGHTNESS = 40;

// Rewrite the WAV header this often, so a power loss keeps the file
constexpr uint32_t SYNC_MS = 5000;

// ============================================================
// State
// ============================================================

enum class State {
    Ready,
    Recording,
    Sleeping
};

State state = State::Ready;

BootButton button;
StatusLights lights;
Battery battery;
WavFile wav;

bool cardMounted = false;
bool micOk = false;
uint16_t nextNumber = 1;
char wavName[16] = "";
char labelName[16] = "";
uint16_t markers = 0;
uint32_t lastSync = 0;

// UI
float meterDb = -90.0f;
uint32_t clipUntil = 0;
uint32_t markerUntil = 0;
uint32_t errorUntil = 0;
const char *lastError = "";

// Card writes go through this SRAM buffer (16 KB, 32 sectors)
constexpr size_t CHUNK = 8192;
int16_t chunk[CHUNK];

// ============================================================
// Helpers
// ============================================================

void showError(const char *message) {

    lastError = message;
    errorUntil = millis() + 3000;

    Serial.print("ERROR: ");
    Serial.println(message);
}

bool mountCard() {

    if (cardMounted) {
        return true;
    }

    SD.end();
    cardMounted = SD.begin(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_DAT0);

    if (cardMounted) {
        Serial.print("microSD: ");
        Serial.print(SD.size64() / (1024.0 * 1024.0 * 1024.0), 1);
        Serial.println(" GB");
    }

    return cardMounted;
}

bool startMicrophone() {

    micOk = PdmCapture::begin(SAMPLE_RATE, MIC_GAIN, RECORD_GAIN_DB, INVERT_PDM_CLK);

    if (!micOk) {
        showError("microphone did not start");
    }

    return micOk;
}

// ============================================================
// Recording
// ============================================================

void startRecording() {

    if (!micOk) {
        showError("microphone not running");
        return;
    }

    if (!mountCard()) {
        showError("no microSD card");
        return;
    }

    while (true) {

        snprintf(wavName, sizeof(wavName), "REC%04u.WAV", nextNumber);

        if (!SD.exists(wavName)) {
            break;
        }

        if (++nextNumber > 9999) {
            showError("too many recordings");
            return;
        }
    }

    snprintf(labelName, sizeof(labelName), "REC%04u.txt", nextNumber);

    if (!wav.open(wavName, SAMPLE_RATE)) {
        cardMounted = false;   // probably removed, remount next time
        showError("cannot create file");
        return;
    }

    // Start with fresh audio
    PdmCapture::discard();
    PdmCapture::overruns = 0;

    markers = 0;
    lastSync = millis();
    state = State::Recording;

    Serial.print("Recording ");
    Serial.println(wavName);
}

// Writes buffered audio to the card. all = flush everything.
void writeAudio(bool all) {

    while (PdmCapture::available() >= CHUNK ||
           (all && PdmCapture::available() > 0)) {

        const size_t n = PdmCapture::read(chunk, CHUNK);

        if (!wav.write(chunk, n)) {
            wav.close();
            cardMounted = false;
            state = State::Ready;
            showError("card full or removed");
            return;
        }

        // One chunk per loop() while recording keeps the button and
        // lights responsive; the ring absorbs the rest.
        if (!all) {
            break;
        }
    }
}

void stopRecording() {

    writeAudio(true);

    const float seconds = wav.seconds();
    wav.close();
    state = State::Ready;
    nextNumber++;

    Serial.print("Saved ");
    Serial.print(wavName);
    Serial.print("  ");
    Serial.print(seconds, 1);
    Serial.print(" s  markers=");
    Serial.print(markers);
    Serial.print("  overruns=");
    Serial.println(PdmCapture::overruns);
}

void addMarker() {

    const float t = wav.seconds() + PdmCapture::available() / float(SAMPLE_RATE);
    markers++;

    // Audacity label track: start <tab> end <tab> text
    File f = SD.open(labelName, FILE_WRITE);

    if (f) {
        f.print(t, 3);
        f.print('\t');
        f.print(t, 3);
        f.print("\tMarker ");
        f.println(markers);
        f.close();
    }

    markerUntil = millis() + 250;

    Serial.print("Marker ");
    Serial.print(markers);
    Serial.print(" at ");
    Serial.print(t, 1);
    Serial.println(" s");
}

// ============================================================
// Sleep
// ============================================================

void goToSleep() {

    if (state == State::Recording) {
        stopRecording();
    }

    PdmCapture::end();
    lights.clear();
    lights.show();
    lights.led(false);

    state = State::Sleeping;
    Serial.println("Sleeping. Hold BOOT 2 s to wake up.");
}

void wakeUp() {

    state = State::Ready;
    startMicrophone();
    Serial.println("Awake.");
}

// ============================================================
// Lights
// ============================================================

void renderLights() {

    const uint32_t now = millis();
    lights.clear();

    // Built-in LED
    bool led;

    if (now < errorUntil) {
        led = (now % 200) < 100;
    } else if (state == State::Recording) {
        led = true;
    } else {
        led = (now % 1000) < 50;
    }

    lights.led(led);

    // NeoPixels, highest priority first
    const float hold = button.holdProgress();

    if (hold > 0.15f) {

        const uint8_t lit = hold * NUM_NEOPIXEL + 0.99f;

        for (uint8_t i = 0; i < lit; i++) {
            lights.setPixel(i, 160, 0, 255);
        }

    } else if (now < errorUntil) {

        if ((now % 300) < 150) {
            lights.fill(255, 0, 0);
        }

    } else if (now < markerUntil) {

        lights.fill(255, 255, 255);

    } else if (now < clipUntil) {

        lights.fill(255, 0, 0);

    } else {

        // Level meter: -45, -30 and -15 dBFS
        static const float steps[NUM_NEOPIXEL] = { -45.0f, -30.0f, -15.0f };
        bool any = false;

        for (uint8_t i = 0; i < NUM_NEOPIXEL; i++) {

            if (meterDb < steps[i]) {
                continue;
            }

            any = true;

            if (state == State::Recording) {
                if (i == 0) {
                    lights.setPixel(i, 0, 255, 0);
                } else if (i == 1) {
                    lights.setPixel(i, 255, 160, 0);
                } else {
                    lights.setPixel(i, 255, 0, 0);
                }
            } else {
                lights.setPixel(i, 0, 60, 255);
            }
        }

        if (state == State::Recording && !any) {
            lights.setPixel(0, 60, 0, 0);
        }

        // Low battery: orange blink on the last pixel while ready
        if (state == State::Ready && battery.low() && (now % 2000) < 150) {
            lights.setPixel(NUM_NEOPIXEL - 1, 255, 80, 0);
        }
    }

    lights.show();
}

void updateMeter() {

    const int32_t peak = PdmCapture::takePeak();
    const float db = peak > 0 ? 20.0f * log10f(peak / 32768.0f) : -90.0f;

    // Fast attack, slow release
    meterDb = db > meterDb ? db : meterDb - 1.5f;

    if (PdmCapture::takeClipped()) {
        clipUntil = millis() + 200;
    }
}

// ============================================================
// Serial report
// ============================================================

void report() {

    Serial.print(state == State::Recording ? "REC   " : "READY ");

    if (state == State::Recording) {
        Serial.print(wavName);
        Serial.print("  ");
        Serial.print(wav.seconds(), 1);
        Serial.print(" s  buffer ");
        Serial.print(100.0f * PdmCapture::available() / PdmCapture::capacity(), 1);
        Serial.print("%  overruns=");
        Serial.print(PdmCapture::overruns);
    } else {
        Serial.print("card=");
        Serial.print(cardMounted ? "OK" : "none");
    }

    Serial.print("  level=");
    Serial.print(meterDb, 0);
    Serial.print(" dBFS");

    if (battery.present()) {
        Serial.print("  battery=");
        Serial.print(battery.volts(), 2);
        Serial.print(" V (");
        Serial.print(battery.percent());
        Serial.print("%)");
    }

    Serial.println();
}

// ============================================================
// Setup
// ============================================================

void setup() {

    Serial.begin(115200);

    lights.begin(PIXEL_BRIGHTNESS);
    battery.begin(BATTERY_PIN, BATTERY_DIVIDER);

    // Startup sweep
    for (uint8_t i = 0; i < NUM_NEOPIXEL; i++) {
        lights.clear();
        lights.setPixel(i, 0, 60, 255);
        lights.show();
        delay(120);
    }

    startMicrophone();
    mountCard();

    Serial.println("UNIT Pulsar Voice Recorder");
    Serial.println("BOOT: short = rec/stop, double = marker, hold 2 s = sleep");
}

// ============================================================
// Loop
// ============================================================

void loop() {

    const uint32_t now = millis();

    // --------------------------------------------------------
    // Button
    // --------------------------------------------------------

    const ButtonEvent event = button.update(PdmCapture::safeToPauseInterrupts());

    if (state == State::Sleeping) {

        if (event == ButtonEvent::Long) {
            wakeUp();
        }

        delay(5);
        return;
    }

    switch (event) {

    case ButtonEvent::Short:
        if (state == State::Recording) {
            stopRecording();
        } else {
            startRecording();
        }
        break;

    case ButtonEvent::Double:
        if (state == State::Recording) {
            addMarker();
        }
        break;

    case ButtonEvent::Long:
        goToSleep();
        return;

    default:
        break;
    }

    // --------------------------------------------------------
    // Audio
    // --------------------------------------------------------

    if (state == State::Recording) {

        writeAudio(false);

        if (state == State::Recording && now - lastSync >= SYNC_MS) {
            lastSync = now;
            wav.sync();
        }

    } else {
        PdmCapture::discard();
    }

    // --------------------------------------------------------
    // Battery
    // --------------------------------------------------------

    static uint32_t lastBattery = 0;

    if (battery.present() && now - lastBattery >= 5000) {

        lastBattery = now;
        battery.update();

        if (battery.critical()) {
            showError("battery empty, saving");
            goToSleep();
            return;
        }
    }

    // --------------------------------------------------------
    // Lights and report
    // --------------------------------------------------------

    static uint32_t lastFrame = 0;

    if (now - lastFrame >= 33) {
        lastFrame = now;
        updateMeter();
        renderLights();
    }

    static uint32_t lastReport = 0;

    if (now - lastReport >= 1000) {
        lastReport = now;
        report();
    }
}
