/*
  UNIT Pulsar RP2350A
  Basic LED Blink

  LED_BUILTIN:
  Arduino D13
  RP2350A GPIO22
*/

#ifndef ARDUINO_UNIT_PULSAR_RP2350A
#error "This example is intended for the UNIT Pulsar RP2350A."
#endif

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
