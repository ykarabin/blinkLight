#include <Arduino.h>

const int leds[] = {1, 2, 4, 5, 6, 7};
const int ledCount = sizeof(leds) / sizeof(leds[0]);

const float speedMultiplier = 0.5;

void setup() {
  for (int i = 0; i < ledCount; i++) {
    pinMode(leds[i], OUTPUT);
  }
}

int scaledDelay(int delayMs) {
  return delayMs / speedMultiplier;
}

void allOff() {
  for (int i = 0; i < ledCount; i++) {
    digitalWrite(leds[i], LOW);
  }
}

void blinkPair(int led1, int led2, int times, int onTime, int offTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    delay(onTime);

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    delay(offTime);
  }
}

void runningLightForward() {
  for (int i = 0; i < ledCount; i++) {
    allOff();
    digitalWrite(leds[i], HIGH);
    delay(scaledDelay(25));
  }
}

void runningLightBackward() {
  for (int i = ledCount - 1; i >= 0; i--) {
    allOff();
    digitalWrite(leds[i], HIGH);
    delay(scaledDelay(25));
  }
}

void blinkBlue()
{
  blinkPair(leds[4], leds[5], 3, scaledDelay(50), scaledDelay(40));
}

void blinkRed()
{
  blinkPair(leds[0], leds[1], 3, scaledDelay(50), scaledDelay(40));
}

void loop() {
  blinkRed();
  delay(scaledDelay(50));

  blinkBlue();
  delay(scaledDelay(50));

  runningLightForward();
  runningLightBackward();

  allOff();
  delay(scaledDelay(50));



  blinkBlue();
  delay(scaledDelay(50));

  blinkRed();
  delay(scaledDelay(50));

  runningLightBackward();
  runningLightForward();

  allOff();
  delay(scaledDelay(50));
}
