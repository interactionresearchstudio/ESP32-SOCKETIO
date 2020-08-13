void setupPins() {
  pinMode(onBoardLed, OUTPUT);
  pinMode(buttonPin, INPUT);

  pinMode(EXTERNAL_BUTTON, INPUT_PULLUP);
  pinMode(EXTERNAL_LED1, OUTPUT);
  pinMode(EXTERNAL_LED2, OUTPUT);
  pinMode(EXTERNAL_LED3, OUTPUT);
  digitalWrite(EXTERNAL_LED1, 1);
  digitalWrite(EXTERNAL_LED2, 1);
  digitalWrite(EXTERNAL_LED3, 1);
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
    digitalWrite(EXTERNAL_LED3, 0);
  }
}

void led3Handler() {
  if (led3IsPressed == true && millis() - led3PrevTime > LED3TIMEON) {
    led3IsPressed = false;
    digitalWrite(EXTERNAL_LED3, 1);
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
    digitalWrite(onBoardLed, 1);
    digitalWrite(EXTERNAL_LED1, 0);
  }
  if (millis() - blinkTime > blinkDuration && isBlinking == true) {
    digitalWrite(onBoardLed, 0);
    digitalWrite(EXTERNAL_LED1, 1);
    isBlinking = false;
    readyToBlink = false;
  }
}

void blinkOnConnect() {
  byte NUM_BLINKS = 3;
  for (byte i = 0; i < NUM_BLINKS; i++) {
    digitalWrite(onBoardLed, 1);
    delay(100);
    digitalWrite(onBoardLed, 0);
    delay(400);
  }
  delay(600);
}

// button functions

void buttonHandler() {
  bool buttonState = digitalRead(buttonPin);
  bool buttonState2 = digitalRead(EXTERNAL_BUTTON);
  if (!buttonState2) {
    buttonState = false;
  }

  if (!buttonState && buttonDebounce == false) {
    Serial.println("button pressed");
    buttonPressTime = millis();
    buttonDebounce = true;
    socketIO_sendButtonPress();
  }
  if (buttonState && buttonDebounce == true && millis() - buttonPressTime > 500) {
    buttonDebounce = false;
  }
}



//reset functions

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
