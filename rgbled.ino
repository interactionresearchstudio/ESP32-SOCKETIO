void setupPixels() {

  userSat = 255;
  remoteSat = 255;
  userVal = 255;
  remoteVal = 255;
  userHue = 0;
  remoteHue = 0;

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  delay(1000);
  pixels.setPixelColor(USERLED, pixels.ColorHSV(40000, userSat, userVal));
  pixels.show();
  delay(100);
  pixels.setPixelColor(USERLED, pixels.ColorHSV(0, 0, 0));
  pixels.show();
  delay(100);
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
      pixels.setPixelColor(USERLED, pixels.ColorHSV(userHue, userSat, userVal));
      //  pixels.setPixelColor(REMOTELED, pixels.ColorHSV(remoteHue, remoteSat, remoteVal));
      pixels.show();
    }

  }
  if (millisCheck - prevlongPixelMillis > PIXELUPDATETIMELONG) {
    prevlongPixelMillis = millisCheck;
    ledHasUpdated = true;
  }
}

void cycleHue(int led) {
  switch (led) {
    //user led
    case 0:
      if (userHue < 65536) {
        userHue = userHue + 500;
      } else {
        userHue = 0;
      }
      ledHasUpdated = true;
      break;
    // remote led
    case 1:
      if (remoteHue < 65536) {
        remoteHue = remoteHue + 300;
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
