/*
    USB Audio Class 2.0 microphone for Arduino-Pico (RP2350)

    Copyright (c) 2026 UNIT Electronics
    Author: Adrián Rabadán Ortiz

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

#pragma once

#include <Arduino.h>

#if !defined(USE_TINYUSB) && !defined(NO_USB)

// ============================================================
// UNIT Electronics USB Audio Class
// ============================================================

class USBAudioClass {

public:

    // Constructor
    USBAudioClass();


    // --------------------------------------------------------
    // Initialization
    // --------------------------------------------------------

    void begin();


    // --------------------------------------------------------
    // USB State
    // --------------------------------------------------------

    bool mounted();
    bool ready();

    // True while the host is recording (streaming interface open).
    // When false nothing drains the FIFO, so producers can skip.
    bool streaming();


    // --------------------------------------------------------
    // Input level chosen by the host (applied in write())
    // --------------------------------------------------------

    bool muted();

    // -20 dB .. +30 dB, 0 dB = samples sent unchanged.
    // Starts at +10 dB until the host sets its own level.
    float volumeDb();


    // --------------------------------------------------------
    // PCM Audio
    // --------------------------------------------------------

    size_t write(
        const int16_t *samples,
        size_t sampleCount
    );

    // Samples that fit in the USB IN FIFO right now.
    // The FIFO overwrites old data when full, so producers
    // that run ahead of the host should check this first.
    size_t availableForWrite();

    // Samples waiting in the USB IN FIFO.
    size_t queued();

    // Total USB IN FIFO size in samples. TinyUSB's flow control
    // sizes each packet to keep the FIFO about half full, so
    // producers should aim for bufferSize() / 2 queued.
    size_t bufferSize();

    // --------------------------------------------------------
    // Sample rate (16000, 24000, 44100 or 48000, chosen by the host)
    // --------------------------------------------------------

    uint32_t sampleRate();

    // True once after the host selected a different rate.
    // Restart the audio source at sampleRate() when it fires.
    bool sampleRateChanged();

    // Service the USB stack now. The ISO IN packet for the next
    // frame is only armed when tud_task() handles the previous
    // completion; the core's 1 ms timer alone misses frames.
    // Call this every loop() while streaming.
    void task();


    // --------------------------------------------------------
    // USB resources assigned by the Arduino-Pico core
    // --------------------------------------------------------

    uint8_t interfaceID() const {
        return _interfaceID;
    }

    uint8_t endpointIn() const {
        return _endpointIn;
    }

    uint8_t stringID() const {
        return _stringID;
    }


private:

    // Audio Control interface number
    uint8_t _interfaceID;

    // Isochronous Audio IN endpoint
    uint8_t _endpointIn;

    // USB interface string index
    uint8_t _stringID;


    // --------------------------------------------------------
    // USB Descriptor Callback
    // --------------------------------------------------------

    static void descriptorCallback(
        int interfaceNumber,
        uint8_t *dst,
        int len,
        void *param
    );
};


// ============================================================
// Global USB Audio Object
// ============================================================

extern USBAudioClass USBAudio;

#endif