/*
  UNIT Pulsar RP2350A
  Serial2 Loopback Test

  Serial2:
  TX -> GPIO26 / A3 / D17
  RX -> GPIO27 / A2 / D16

  Connect TX to RX for a loopback test.

  Author: Adrián Rabadán Ortiz
  Copyright (c) 2026 UNIT Electronics
  Released under the MIT License: you may copy, modify and use
  this example in your own projects, commercial or not.
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("UNIT Pulsar RP2350A");
  Serial.println("Serial2 Loopback Test");
  Serial.println("TX = GPIO26");
  Serial.println("RX = GPIO27");
}

void loop() {
  static uint32_t counter = 0;

  Serial2.print("Serial2 message: ");
  Serial2.println(counter++);

  delay(100);

  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }

  delay(900);
}
