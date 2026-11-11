/*
  UNIT Electronics - Pulsar RP2350A
  Voice Recorder: BootButton.h

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#pragma once

// ============================================================
// BOOT button: short, double and long press
// ============================================================
//
// BOOT shares the flash chip-select, so it can only be read by
// briefly pausing flash access and interrupts (BOOTSEL). update()
// reads it only when the caller says it is safe.
//

#include <Arduino.h>

enum class ButtonEvent {
    None,
    Short,
    Double,
    Long
};

class BootButton {

public:

    static constexpr uint32_t POLL_MS = 20;
    static constexpr uint32_t DOUBLE_MS = 350;
    static constexpr uint32_t LONG_MS = 2000;

    ButtonEvent update(bool safeToRead) {

        const uint32_t now = millis();
        ButtonEvent event = ButtonEvent::None;

        if (safeToRead && now - _lastPoll >= POLL_MS) {

            _lastPoll = now;
            const bool down = BOOTSEL;

            if (down && !_down) {

                _down = true;
                _pressAt = now;
                _longFired = false;

                if (_pending && now - _releaseAt < DOUBLE_MS) {
                    _pending = false;
                    _second = true;
                }

            } else if (!down && _down) {

                _down = false;

                if (!_longFired) {
                    if (_second) {
                        _second = false;
                        event = ButtonEvent::Double;
                    } else {
                        _pending = true;
                        _releaseAt = now;
                    }
                }
            }
        }

        if (_down && !_longFired && now - _pressAt >= LONG_MS) {
            _longFired = true;
            _pending = false;
            _second = false;
            event = ButtonEvent::Long;
        }

        // A single press is only reported once no second one followed
        if (event == ButtonEvent::None &&
            _pending && !_down && now - _releaseAt >= DOUBLE_MS) {
            _pending = false;
            event = ButtonEvent::Short;
        }

        return event;
    }

    // 0..1 while the button is held towards a long press
    float holdProgress() const {

        if (!_down || _longFired) {
            return 0.0f;
        }

        const float p = (millis() - _pressAt) / static_cast<float>(LONG_MS);
        return p > 1.0f ? 1.0f : p;
    }

private:

    bool _down = false;
    bool _pending = false;
    bool _second = false;
    bool _longFired = false;
    uint32_t _lastPoll = 0;
    uint32_t _pressAt = 0;
    uint32_t _releaseAt = 0;
};
