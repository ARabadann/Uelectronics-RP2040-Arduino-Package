/*
  UNIT Pulsar RP2350A
  microSD SDIO Read/Write Test

  SDIO:
  CLK  -> GPIO2
  CMD  -> GPIO3
  DAT0 -> GPIO4
  DAT1 -> GPIO5
  DAT2 -> GPIO6
  DAT3 -> GPIO7

  Arduino-Pico SDIO uses CLK, CMD, and DAT0.
  DAT1..DAT3 must be connected consecutively after DAT0.
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

#include <SD.h>

const char *testFile = "/pulsar_test.txt";

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("UNIT Pulsar RP2350A");
  Serial.println("microSD SDIO Read/Write Test");
  Serial.println("Initializing microSD...");

  if (!SD.begin(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_DAT0)) {
    Serial.println("ERROR: microSD initialization failed.");
    Serial.println("Check that a formatted card is inserted.");
    return;
  }

  Serial.println("microSD initialized.");

  File file = SD.open(testFile, FILE_WRITE);
  if (!file) {
    Serial.println("ERROR: Could not open test file for writing.");
    return;
  }

  file.println("UNIT Pulsar RP2350A microSD test");
  file.println("SDIO interface OK");
  file.close();

  Serial.println("Write: PASS");

  file = SD.open(testFile, FILE_READ);
  if (!file) {
    Serial.println("ERROR: Could not open test file for reading.");
    return;
  }

  Serial.println("File contents:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();

  Serial.println();
  Serial.println("Read: PASS");

  Serial.print("Card size: ");
  Serial.print((double)SD.size64() / (1024.0 * 1024.0 * 1024.0), 2);
  Serial.println(" GB");
}

void loop() {
}
