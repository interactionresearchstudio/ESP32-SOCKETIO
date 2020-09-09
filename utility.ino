void setupPins() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(BUTTON_BUILTIN, INPUT);
  pinMode(EXTERNAL_BUTTON, INPUT_PULLUP);

  ButtonConfig* buttonConfigBuiltIn = buttonBuiltIn.getButtonConfig();
  buttonConfigBuiltIn->setEventHandler(handleButtonEvent);
  buttonConfigBuiltIn->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfigBuiltIn->setLongPressDelay(longButtonPressDelay);

  ButtonConfig* buttonExternalConfig = buttonExternal.getButtonConfig();
  buttonExternalConfig->setEventHandler(handleButtonEvent);

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
  Serial.println(button->getId());

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
      // TODO also hold the LED at the colour for a little bit
      break;
    case AceButton::kEventReleased:
      Serial.println("TOUCH: released");
      isSelectingColour = false;
      break;
    case AceButton::kEventClicked:
      Serial.println("TOUCH: clicked");
      ledChanged[USERLED] = true;
      fadeRGB(USERLED);
      socketIO_sendColour();
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

void softReset() {
  if (isResetting == false) {
    isResetting = true;
    resetTime = millis();
  }
}

void checkReset() {
  if (isResetting) {
    if (millis() - resetTime > resetLength) {
      ESP.restart();
    }
  }
}

String generateID() {
  uint64_t chipID = ESP.getEfuseMac();
  String out = String((uint32_t)chipID);
  return  out;
}
