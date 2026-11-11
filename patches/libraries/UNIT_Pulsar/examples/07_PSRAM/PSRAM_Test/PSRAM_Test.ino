/*
  UNIT Pulsar RP2350A
  8 MB PSRAM Test

  External PSRAM:
  APS6404L-3SQR-ZR
  CS -> GPIO0
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("UNIT Pulsar RP2350A");
  Serial.println("PSRAM Test");

  Serial.print("Detected PSRAM: ");
  Serial.print(rp2040.getPSRAMSize() / (1024 * 1024));
  Serial.println(" MB");

  Serial.print("Total PSRAM heap: ");
  Serial.print(rp2040.getTotalPSRAMHeap() / 1024);
  Serial.println(" KB");

  Serial.print("Free PSRAM heap before allocation: ");
  Serial.print(rp2040.getFreePSRAMHeap() / 1024);
  Serial.println(" KB");

  const size_t testSize = 1024 * 1024;
  uint8_t *buffer = (uint8_t *)pmalloc(testSize);

  if (!buffer) {
    Serial.println("ERROR: Could not allocate 1 MB in PSRAM.");
    return;
  }

  for (size_t i = 0; i < testSize; i++) {
    buffer[i] = (uint8_t)(i & 0xFF);
  }

  bool ok = true;
  for (size_t i = 0; i < testSize; i++) {
    if (buffer[i] != (uint8_t)(i & 0xFF)) {
      ok = false;
      Serial.print("Memory error at byte ");
      Serial.println(i);
      break;
    }
  }

  Serial.println(ok ? "PSRAM read/write test: PASS" : "PSRAM read/write test: FAIL");

  free(buffer);

  Serial.print("Free PSRAM heap after test: ");
  Serial.print(rp2040.getFreePSRAMHeap() / 1024);
  Serial.println(" KB");
}

void loop() {
}
