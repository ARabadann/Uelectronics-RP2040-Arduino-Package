/*
  UNIT Pulsar RP2350A
  QWIIC I2C Bus Scanner

  QWIIC SDA -> GPIO24
  QWIIC SCL -> GPIO25

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

  Wire1.setSDA(PIN_QWIIC_SDA);
  Wire1.setSCL(PIN_QWIIC_SCL);
  Wire1.begin();

  Serial.println();
  Serial.println("UNIT Pulsar RP2350A");
  Serial.println("QWIIC I2C Scanner");
}

void loop() {
  uint8_t found = 0;

  Serial.println("Scanning QWIIC bus...");

  for (uint8_t address = 1; address < 127; address++) {
    Wire1.beginTransmission(address);
    uint8_t error = Wire1.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      found++;
    }
  }

  if (!found) {
    Serial.println("No QWIIC devices found.");
  } else {
    Serial.print("Devices found: ");
    Serial.println(found);
  }

  Serial.println();
  delay(3000);
}
