# UNIT Pulsar RP2350A Examples v1.3.0

Included examples:

- 01_Basics/Blink
- 01_Basics/Serial2
- 01_Basics/AnalogRead
- 02_NeoPixel/NeoPixelEffects
- 03_I2C/I2CScanner
- 03_I2C/QWIICScanner
- 04_Microphone/USBMicrophone
- 04_Microphone/USBMicrophoneTone
- 05_Recorder/VoiceRecorder
- 06_MicroSD/MicroSD_ReadWrite
- 07_PSRAM/PSRAM_Test

Notes:
- NeoPixelEffects requires the Adafruit NeoPixel library.
- USBMicrophone turns the board into a driverless USB microphone
  (on-board ICS-41350 PDM mic). The computer selects 16, 24, 44.1 or 48 kHz
  and sets the input level (-20 dB .. +30 dB).
- USBMicrophoneTone sends a 1 kHz test tone through the same USB path.
- VoiceRecorder records the microphone to microSD as WAV files, buffered
  in PSRAM. BOOT: short = rec/stop, double = marker, hold 2 s = sleep.
  NeoPixels show the level and status; an optional battery divider can
  be enabled in the sketch.
- MicroSD_ReadWrite uses Arduino-Pico SDIO mode.
- Examples are intended for the UNIT Pulsar RP2350A board definition.
