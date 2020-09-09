void setupPixels() {

  hue[USERLED] = 0;
  hue[REMOTELED] = 0;
  saturation[USERLED] = 0;
  saturation[REMOTELED] = 0;
  value[USERLED] = 0;
  value[REMOTELED] = 0;

  FastLED.addLeds<WS2811, WS2811PIN, RGB>(leds, NUMPIXELS);
  fill_solid(leds, NUMPIXELS, CRGB(0, 0, 0));
  FastLED.show();
  delay(1000);

}

void rgbLedHandler() {
  unsigned long millisCheck = millis();
  if (millisCheck - prevPixelMillis > PIXELUPDATETIME) {
    if (isSelectingColour == true) {
      cycleHue(USERLED);
    }
    prevPixelMillis = millisCheck;
    if (ledHasUpdated) {
      ledHasUpdated = false;
      saturation[USERLED] = 255;
      value[USERLED] = 255;
      leds[USERLED] = CHSV(hue[USERLED], saturation[USERLED], value[USERLED]);
      FastLED.show();
    }
    if (led2HasChanged) {
      led2HasChanged = false;
      saturation[REMOTELED] = 255;
      value[REMOTELED] = 255;
      fadeRGB(REMOTELED);
      leds[REMOTELED] = CHSV(hue[REMOTELED], saturation[REMOTELED], value[REMOTELED]);
      FastLED.show();
    }
  }
  if (millisCheck - prevlongPixelMillis > PIXELUPDATETIMELONG) {
    prevlongPixelMillis = millisCheck;
    FastLED.show();
  }
  fadeRGBHandler();
}

void cycleHue(int led) {
  switch (led) {
    //user led
    case 0:
      if (hue[USERLED] < 255) {
        hue[USERLED]++;
      } else {
        hue[USERLED] = 0;
      }
      ledHasUpdated = true;
      break;
    // remote led
    case 1:
      if (hue[REMOTELED] < 255) {
        hue[REMOTELED]++;
      } else {
        hue[REMOTELED] = 0;
      }
      ledHasUpdated = true;
      break;
  }
}

uint16_t getUserHue() {
  return hue[USERLED];
}


void blinkRGB() {
  leds[0] = CHSV(hue[USERLED], 0, 0);
  FastLED.show();
  delay(100);
  leds[0] = CHSV(hue[USERLED], saturation[USERLED], value[USERLED]);
  FastLED.show();
}

void animateLed(int led) {
  for (byte i = saturation[REMOTELED]; i > RGBLEDPWMSTART; i--) {
    leds[led] = CHSV(hue[REMOTELED], saturation[REMOTELED], i);
    FastLED.show();
    delay(6);
  }
  value[REMOTELED] = RGBLEDPWMSTART;
}

void fadeRGB(int led) {
  if (readyToFadeRGB[led] == false) {
    readyToFadeRGB[led] = true;
  }
}

void fadeRGBHandler() {
  for (byte i = 0; i < NUMPIXELS; i++) {
    if (readyToFadeRGB[i] == true && isFadingRGB[i] == false) {
      isFadingRGB[i] = true;
      fadeTimeRGB[i] = millis();
      value[i] = 255;
    }
    if (millis() - fadeTimeRGB[i] > RGBFADEMILLIS && isFadingRGB[i] == true) {
      fadeTimeRGB[i] = millis();
      if (value[i] > RGBLEDPWMSTART) {
        value[i]--;
        leds[i] = CHSV(hue[i], saturation[i], value[i]);
        FastLED.show();
      } else {
        isFadingRGB[i] = false;
        readyToFadeRGB[i] = false;
      }
    }
  }
}
