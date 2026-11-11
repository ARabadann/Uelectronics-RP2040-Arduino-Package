/*
  UNIT Pulsar RP2350A
  Analog Input Test

  A0 -> GPIO28
  A1 -> GPIO29
  A2 -> GPIO27
  A3 -> GPIO26

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
  delay(2000);

  Serial.println();
  Serial.println("UNIT Pulsar RP2350A");
  Serial.println("Analog Input Test");
}

void loop() {
  Serial.print("A0: ");
  Serial.print(analogRead(A0));
  Serial.print(" | A1: ");
  Serial.print(analogRead(A1));
  Serial.print(" | A2: ");
  Serial.print(analogRead(A2));
  Serial.print(" | A3: ");
  Serial.println(analogRead(A3));
  delay(500);
}
