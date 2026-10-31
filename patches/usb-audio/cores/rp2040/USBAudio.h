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


    // --------------------------------------------------------
    // PCM Audio
    // --------------------------------------------------------

    size_t write(
        const int16_t *samples,
        size_t sampleCount
    );


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