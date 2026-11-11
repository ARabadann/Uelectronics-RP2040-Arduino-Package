/*
  UNIT Electronics - Pulsar RP2350A
  Voice Recorder: WavFile.h

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#pragma once

// ============================================================
// 16-bit mono WAV writer
// ============================================================
//
// The header is rewritten every few seconds (sync()), so if power
// is lost the file still plays up to the last sync.
//

#include <Arduino.h>
#include <SD.h>

class WavFile {

public:

    bool open(const char *path, uint32_t sampleRate) {

        // No O_APPEND: the header must be rewritten in place
        _file = SD.open(path, O_RDWR | O_CREAT | O_TRUNC);

        if (!_file) {
            return false;
        }

        _rate = sampleRate;
        _dataBytes = 0;

        return writeHeader();
    }

    bool isOpen() {
        return static_cast<bool>(_file);
    }

    // Returns false if the card did not take all the data
    bool write(const int16_t *samples, size_t count) {

        const size_t bytes = count * sizeof(int16_t);
        const size_t written =
            _file.write(reinterpret_cast<const uint8_t *>(samples), bytes);

        _dataBytes += written;
        return written == bytes;
    }

    bool sync() {

        const bool ok = writeHeader();
        _file.flush();
        return ok;
    }

    void close() {

        if (!_file) {
            return;
        }

        writeHeader();
        _file.close();
    }

    float seconds() const {
        return _rate ? _dataBytes / 2.0f / _rate : 0.0f;
    }

private:

    File _file;
    uint32_t _rate = 0;
    uint32_t _dataBytes = 0;

    static void put16(uint8_t *p, uint16_t v) {
        p[0] = v;
        p[1] = v >> 8;
    }

    static void put32(uint8_t *p, uint32_t v) {
        p[0] = v;
        p[1] = v >> 8;
        p[2] = v >> 16;
        p[3] = v >> 24;
    }

    bool writeHeader() {

        uint8_t h[44];

        memcpy(h, "RIFF", 4);
        put32(h + 4, 36 + _dataBytes);
        memcpy(h + 8, "WAVEfmt ", 8);
        put32(h + 16, 16);              // fmt chunk size
        put16(h + 20, 1);               // PCM
        put16(h + 22, 1);               // mono
        put32(h + 24, _rate);
        put32(h + 28, _rate * 2);       // bytes per second
        put16(h + 32, 2);               // block align
        put16(h + 34, 16);              // bits per sample
        memcpy(h + 36, "data", 4);
        put32(h + 40, _dataBytes);

        _file.seek(0);
        const bool ok = _file.write(h, sizeof(h)) == sizeof(h);
        _file.seek(sizeof(h) + _dataBytes);

        return ok;
    }
};
