#include <Arduino.h>

#define PIN 4

void setup() {
  pinMode(PIN, OUTPUT);
}

void loop() {
  digitalWrite(PIN, HIGH);
  delay(500);

  digitalWrite(PIN, LOW);
  delay(500);

  pinMode(PIN, INPUT);
  delay(500);
  pinMode(PIN, OUTPUT);
}
