void setupPins() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(BUTTON_BUILTIN, INPUT);
  pinMode(EXTERNAL_BUTTON, INPUT_PULLUP);

  pinMode(FADE_3, INPUT_PULLUP);
  pinMode(FADE_1, INPUT_PULLUP);

  ButtonConfig* buttonConfigBuiltIn = buttonBuiltIn.getButtonConfig();
  buttonConfigBuiltIn->setEventHandler(handleButtonEvent);
  buttonConfigBuiltIn->setFeature(ButtonConfig::kFeatureClick);
  buttonConfigBuiltIn->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfigBuiltIn->setLongPressDelay(LONG_TOUCH);

  touchConfig.setFeature(ButtonConfig::kFeatureClick);
  touchConfig.setFeature(ButtonConfig::kFeatureLongPress);
  touchConfig.setEventHandler(handleTouchEvent);
  touchConfig.setLongPressDelay(LONG_TOUCH);
}

//internal led functions

void blinkDevice() {
  if (readyToBlink == false) {
    readyToBlink = true;
  }
}

void ledHandler() {
  if (readyToBlink == true && isBlinking == false) {
    isBlinking = true;
    blinkTime = millis();
    digitalWrite(LED_BUILTIN, 1);
  }
  if (millis() - blinkTime > blinkDuration && isBlinking == true) {
    digitalWrite(LED_BUILTIN, 0);
    isBlinking = false;
    readyToBlink = false;
  }
}

void blinkOnConnect() {
  byte NUM_BLINKS = 3;
  for (byte i = 0; i < NUM_BLINKS; i++) {
    digitalWrite(LED_BUILTIN, 1);
    delay(100);
    digitalWrite(LED_BUILTIN, 0);
    delay(400);
  }
  delay(600);
}

// button functions
void handleButtonEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.print("ID IS: ");
  Serial.println(button->getPin());
  switch (button->getPin()) {
    case 0:
      switch (eventType) {
        case AceButton::kEventPressed:
          break;
        case AceButton::kEventReleased:
          if (currentSetupStatus == setup_finished) socketIO_sendButtonPress();
          break;
        case AceButton::kEventLongPressed:
#ifdef DEV
          factoryReset();
#endif
          break;
        case AceButton::kEventRepeatPressed:
          break;
      }
      break;
    case EXTERNAL_BUTTON:
      switch (eventType) {
        case AceButton::kEventPressed:
          Serial.println("TOUCH: pressed");
          break;
        case AceButton::kEventLongPressed:
          Serial.println("TOUCH: Long pressed");
          isSelectingColour = true;
          ledChanged[USERLED] = true;
          // TODO also hold the LED at the colour for a little bit
          break;
        case AceButton::kEventReleased:
          Serial.println("TOUCH: released");
          if (isSelectingColour == true) {
            ledChanged[USERLED] = true;
            fadeRGB(USERLED);
          } else {
            ledChanged[USERLED] = true;
            fadeRGB(USERLED);
            socketIO_sendColour();
          }
          isSelectingColour = false;
          break;
        case AceButton::kEventClicked:
          Serial.println("TOUCH: clicked");
          break;
      }
      break;
  }
}

// button functions
void handleTouchEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println(button->getId());

  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("TOUCH: pressed");
      break;
    case AceButton::kEventLongPressed:
      Serial.println("TOUCH: Long pressed");
      isSelectingColour = true;
      ledChanged[USERLED] = true;
      // TODO also hold the LED at the colour for a little bit
      break;
    case AceButton::kEventReleased:
      Serial.println("TOUCH: released");
      if (isSelectingColour == true) {
        ledChanged[USERLED] = true;
        fadeRGB(USERLED);
      } else {
        ledChanged[USERLED] = true;
        fadeRGB(USERLED);
        isFadingRGB[USERLED] = false;
        startLongFade(USERLED);
        socketIO_sendColour();
      }
      isSelectingColour = false;
      break;
    case AceButton::kEventClicked:
      Serial.println("TOUCH: clicked");
      break;
  }
}

//reset functions
void factoryReset() {
  Serial.println("factoryReset");

  preferences.begin("scads", false);
  preferences.clear();
  preferences.end();
  currentSetupStatus = setup_pending;

  ESP.restart();
}

void softReset(int delayMs) {
  if (isResetting == false) {
    isResetting = true;
    resetTime = millis() + delayMs;
  }
}

void checkReset() {
  if (isResetting) {
    if (millis() > resetTime + resetDurationMs) {
      ESP.restart();
    }
  }
}

String generateID() {
  //https://github.com/espressif/arduino-esp32/issues/3859#issuecomment-689171490
  uint64_t chipID = ESP.getEfuseMac();
  uint32_t low = chipID % 0xFFFFFFFF;
  uint32_t high = (chipID >> 32) % 0xFFFFFFFF;
  String out = String(low);
  return  out;
}

void setupCapacitiveTouch() {
  int touchAverage = 0;
  for (byte i = 0; i < 10; i++) {
    touchAverage = touchAverage + touchRead(CAPTOUCH);
    delay(100);
  }
  touchAverage = touchAverage / 10;
  TOUCH_THRESHOLD = touchAverage - TOUCH_HYSTERESIS;
  Serial.print("Touch threshold is:");
  Serial.println(TOUCH_THRESHOLD);
  touchConfig.setThreshold(TOUCH_THRESHOLD);
}

long checkFadingLength() {
  if (digitalRead(FADE_3) == 0 && digitalRead(FADE_1) == 1) {
    Serial.println("Your fade time is 3 hours");
    return 180;
  } else if (digitalRead(FADE_1) == 0 && digitalRead(FADE_3) == 1) {
    Serial.println("Your fade time is 1 hour");
    return 60;
  } else if (digitalRead(FADE_3) == 0 && digitalRead(FADE_1) == 0) {
    Serial.println("Your fade time is 9 hours");
    return 540;
  } else {
    Serial.println("Your fade time is 6 hours");
    return 360;
  }
}
