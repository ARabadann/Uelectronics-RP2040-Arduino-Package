/*
  UNIT Pulsar RP2350A
  Main I2C Bus Scanner

  SDA -> GPIO8
  SCL -> GPIO9

  The on-board BMI270 is connected to this bus.

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(2000);

  Wire.setSDA(PIN_IMU_SDA);
  Wire.setSCL(PIN_IMU_SCL);
  Wire.begin();

  Serial.println();
  Serial.println("UNIT Pulsar RP2350A");
  Serial.println("Main I2C Scanner");
}

void loop() {
  uint8_t found = 0;

  Serial.println("Scanning...");

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      found++;
    }
  }

  if (!found) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.print("Devices found: ");
    Serial.println(found);
  }

  Serial.println();
  delay(3000);
}
