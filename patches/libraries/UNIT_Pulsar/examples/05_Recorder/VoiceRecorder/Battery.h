/*
  UNIT Electronics - Pulsar RP2350A
  Voice Recorder: Battery.h

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#pragma once

// ============================================================
// Optional LiPo battery monitor
// ============================================================
//
// The Pulsar has no battery sense circuit. A LiPo reaches 4.2 V and
// the ADC tolerates 3.3 V, so NEVER wire BAT straight to an analog
// pin. Use a 1:2 divider:
//
//   BAT --[470k]--+--[470k]-- GND
//                 |
//                 +-- A0..A3,  plus 100 nF from that pin to GND
//
// It draws ~4.5 uA. Set BATTERY_PIN in the sketch to enable it.
//

#include <Arduino.h>

class Battery {

public:

    void begin(int pin, float dividerRatio) {

        _pin = pin;
        _ratio = dividerRatio;

        if (_pin >= 0) {
            analogReadResolution(12);
            update();
        }
    }

    bool present() const {
        return _pin >= 0;
    }

    void update() {

        if (_pin < 0) {
            return;
        }

        uint32_t sum = 0;

        for (int i = 0; i < 16; i++) {
            sum += analogRead(_pin);
        }

        _volts = sum / 16.0f * 3.3f / 4095.0f * _ratio;
    }

    float volts() const {
        return _volts;
    }

    // Rough LiPo estimate: 3.3 V = 0 %, 4.2 V = 100 %
    int percent() const {

        const int p = static_cast<int>((_volts - 3.3f) / 0.9f * 100.0f);
        return p < 0 ? 0 : (p > 100 ? 100 : p);
    }

    bool low() const {
        return present() && _volts < 3.5f;
    }

    // Stop recording and save before the board browns out
    bool critical() const {
        return present() && _volts < 3.35f;
    }

private:

    int _pin = -1;
    float _ratio = 2.0f;
    float _volts = 0.0f;
};
