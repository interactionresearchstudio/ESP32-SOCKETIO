void setupPixels() {

  userSat = 255;
  remoteSat = 255;
  userVal = 180;
  remoteVal = 180;
  userHue = 0;
  remoteHue = 0;

  FastLED.addLeds<WS2811, WS2811PIN, RGB>(leds, NUMPIXELS);
  fill_solid(leds, NUMPIXELS, CRGB(0, 0, 0));
  FastLED.show();
  delay(1000);

}

void pixelUpdate() {
  unsigned long millisCheck = millis();
  if (millisCheck - prevPixelMillis > PIXELUPDATETIME) {
    if (isSelectingColour == true) {
      cycleHue(USERLED);
    }
    prevPixelMillis = millisCheck;
    if (ledHasUpdated) {
      ledHasUpdated = false;
      leds[0] = CHSV(userHue, userSat, userVal);
      FastLED.show();

    }
    if (led2HasChanged) {
      led2HasChanged = false;
      leds[1] = CHSV(remoteHue, remoteSat, remoteVal);
      FastLED.show();
    }

  }
  if (millisCheck - prevlongPixelMillis > PIXELUPDATETIMELONG) {
    prevlongPixelMillis = millisCheck;
    ledHasUpdated = true;
    led2HasChanged = true;
    
  }
}

void cycleHue(int led) {
  switch (led) {
    //user led
    case 0:
      if (userHue < 255) {
        userHue++;
      } else {
        userHue = 0;
      }
      ledHasUpdated = true;
      break;
    // remote led
    case 1:
      if (remoteHue < 255) {
        remoteHue++;
      } else {
        remoteHue = 0;
      }
      ledHasUpdated = true;
      break;
  }
}


void updateRemoteLedFade() {
  if (isRemoteLedFading) {
    int fadeIncrement = REMOTELEDFADE / REMOTELEDPWMSTART;
    if (remoteLedFadeMinutes >= REMOTELEDFADE) {
      isRemoteLedFading = false;
    } else {
      remoteSat = (int)remoteLedFadeMinutes / fadeIncrement;
      if (prevRemoteSat != remoteSat) {
        ledHasUpdated = true;
      }
      prevRemoteSat = remoteSat;
    }
  }
}

uint16_t getUserHue() {
  return userHue;
}
