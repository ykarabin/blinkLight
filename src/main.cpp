#include <Arduino.h>

const int leds[] = {1, 2, 4, 5, 6, 7};
const int ledCount = sizeof(leds) / sizeof(leds[0]);

#define BTN_UP 8  
#define BTN_DOWN 0


enum ButtonState {
  IDLE,
  DEBOUNCE,
  PRESSED
};

enum BlinkMode {
  POLICE_MODE,
  WAVE_MODE
};

struct Button {
  uint8_t pin;
  ButtonState state;
  unsigned long changeTime;
  unsigned long pressedAt;
};

struct ButtonEvents {
  bool upClicked;
  bool downClicked;
  bool comboClicked;
};

enum ButtonClickType {
  NO_CLICK,
  SINGLE_CLICK,
  COMBO_CLICK
};

Button upButton = {BTN_UP, IDLE, 0, 0};
Button downButton = {BTN_DOWN, IDLE, 0, 0};

BlinkMode blinkMode = POLICE_MODE;

const unsigned long debounceDelay = 50;
const unsigned long comboWindow = 300; // ms window to treat presses as simultaneous
bool comboHandled = false;

float speedMultiplier = 1.0;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < ledCount; i++) {
    pinMode(leds[i], OUTPUT);
  }
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
}

int scaledDelay(int delayMs) {
  return delayMs / speedMultiplier;
}

ButtonClickType updateButton(Button &button, bool rawState, bool otherRawState, unsigned long otherPressedAt, unsigned long now) {
  switch (button.state) {
    case IDLE:
      if (rawState == LOW) {
        button.state = DEBOUNCE;
        button.changeTime = now;
      }
      break;

    case DEBOUNCE:
      if (now - button.changeTime >= debounceDelay) {
        if (rawState == LOW) {
          button.state = PRESSED;
          button.pressedAt = now;
        } else {
          button.state = IDLE;
        }
      }
      break;

    case PRESSED:
      if (rawState == HIGH) {
        button.state = IDLE;

        if (otherRawState == LOW || (otherPressedAt && (now - otherPressedAt) <= comboWindow)) {
          button.pressedAt = 0;
          return COMBO_CLICK;
        }

        button.pressedAt = 0;
        return SINGLE_CLICK;
      }
      break;
  }

  return NO_CLICK;
}

ButtonEvents readButtons() {
  ButtonEvents events = {false, false, false};
  unsigned long now = millis();

  bool upRaw = digitalRead(upButton.pin);
  bool downRaw = digitalRead(downButton.pin);

  if (comboHandled) {
    if (upRaw == HIGH && downRaw == HIGH) {
      comboHandled = false;
      upButton.state = IDLE;
      downButton.state = IDLE;
      upButton.pressedAt = 0;
      downButton.pressedAt = 0;
    }

    return events;
  }

  ButtonClickType upClick = updateButton(upButton, upRaw, downRaw, downButton.pressedAt, now);
  ButtonClickType downClick = updateButton(downButton, downRaw, upRaw, upButton.pressedAt, now);

  if (upClick == COMBO_CLICK || downClick == COMBO_CLICK) {
    events.comboClicked = true;
    comboHandled = true;
    upButton.state = PRESSED;
    downButton.state = PRESSED;
    return events;
  }

  events.upClicked = upClick == SINGLE_CLICK;
  events.downClicked = downClick == SINGLE_CLICK;

  return events;
}

void changeSpeed(float delta) {
  speedMultiplier += delta;

  if (speedMultiplier > 2.0) {
    speedMultiplier = 0.5;
  }

  if (speedMultiplier < 0.5) {
    speedMultiplier = 2.0;
  }

  Serial.print("Speed: ");
  Serial.println(speedMultiplier);
}

void changeMode() {
  if (blinkMode == POLICE_MODE) {
    blinkMode = WAVE_MODE;
    Serial.println("Mode: Wave");
  } else {
    blinkMode = POLICE_MODE;
    Serial.println("Mode: Police");
  }
}

void handleButtons() {
  ButtonEvents events = readButtons();

  if (events.comboClicked) {
    changeMode();
    return;
  }

  if (events.upClicked) {
    changeSpeed(0.5);
  }

  if (events.downClicked) {
    changeSpeed(-0.5);
  }
}

void smartDelay(int delayMs) {
  unsigned long startMillis = millis();
  while (millis() - startMillis < scaledDelay(delayMs)) {
    handleButtons();
    delay(1);
  }
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
    smartDelay(onTime);

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    smartDelay(offTime);
  }
}

void runningLightForward() {
  for (int i = 0; i < ledCount; i++) {
    allOff();
    digitalWrite(leds[i], HIGH);
    smartDelay(25);
  }
}

void runningLightBackward() {
  for (int i = ledCount - 1; i >= 0; i--) {
    allOff();
    digitalWrite(leds[i], HIGH);
    smartDelay(25);
  }
}

void blinkBlue()
{
  blinkPair(leds[4], leds[5], 3, 50, 40);
}

void blinkRed()
{
  blinkPair(leds[0], leds[1], 3, 50, 40);
}

void blinkWaveMode() {
  for (int i = 0; i < ledCount; i++) {
    allOff();
    digitalWrite(leds[i], HIGH);

    if (i > 0) {
      digitalWrite(leds[i - 1], HIGH);
    }

    smartDelay(60);
  }

  for (int i = ledCount - 1; i >= 0; i--) {
    allOff();
    digitalWrite(leds[i], HIGH);

    if (i < ledCount - 1) {
      digitalWrite(leds[i + 1], HIGH);
    }

    smartDelay(60);
  }
}

void blinkPoliceMode() {
  blinkRed();
  smartDelay(50);

  blinkBlue();
  smartDelay(50);

  runningLightForward();
  runningLightBackward();

  allOff();
  smartDelay(50);

  blinkBlue();
  smartDelay(50);

  blinkRed();
  smartDelay(50);

  runningLightBackward();
  runningLightForward();

  allOff();
  smartDelay(50);
}

void loop() {
  if (blinkMode == POLICE_MODE) {
    blinkPoliceMode();
  } else {
    blinkWaveMode();
  }
}
