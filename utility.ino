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

  pinMode(EXTERNAL_LED1, OUTPUT);
  pinMode(EXTERNAL_LED2, OUTPUT);
  pinMode(EXTERNAL_LED3, OUTPUT);
  digitalWrite(EXTERNAL_LED1, HIGH);
  digitalWrite(EXTERNAL_LED2, HIGH);
  digitalWrite(EXTERNAL_LED3, HIGH);
}

//external led functions

void Led2Toggle() {
  led2Toggle = !led2Toggle;
  digitalWrite(EXTERNAL_LED2, led2Toggle);
}

void led3LongOn() {
  if (led3IsPressed == false) {
    led3IsPressed = true;
    led3PrevTime = millis();
    digitalWrite(EXTERNAL_LED3, LOW);
  }
}

void led3Handler() {
  if (led3IsPressed == true && millis() - led3PrevTime > LED3TIMEON) {
    led3IsPressed = false;
    digitalWrite(EXTERNAL_LED3, HIGH);
  }
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
    digitalWrite(EXTERNAL_LED1, 0);
  }
  if (millis() - blinkTime > blinkDuration && isBlinking == true) {
    digitalWrite(LED_BUILTIN, 0);
    digitalWrite(EXTERNAL_LED1, 1);
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
      factoryReset();
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
      socketIO_sendColour();
      break;
  }
}

void updateColourSelection() {
  if (isSelectingColour) {
    uint32_t currentTime = millis();
    if (currentTime - prevColourChange >= COLOUR_CHANGE_DELAY) {
      hue += 1;
      if (hue > 360) hue = 0;
      Serial.println(hue);
      prevColourChange = currentTime;
    }
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
