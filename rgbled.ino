void setupPixels() {

  hue[USERLED] = 0;
  hue[REMOTELED] = 0;
  saturation[USERLED] = 0;
  saturation[REMOTELED] = 0;
  value[USERLED] = 0;
  value[REMOTELED] = 0;

  FastLED.addLeds<WS2812, WS2812PIN, RGB>(leds, NUMPIXELS);
  fill_solid(leds, NUMPIXELS, CRGB(0, 0, 0));
  FastLED.show();
  delay(1000);

  blinkRGB();

}

void rgbLedHandler() {
  unsigned long millisCheck = millis();
  if (millisCheck - prevPixelMillis > PIXELUPDATETIME) {
    if (isSelectingColour == true) {
      cycleHue(USERLED);
    }
    prevPixelMillis = millisCheck;
    if (ledChanged[USERLED]) {
      ledChanged[USERLED] = false;
      saturation[USERLED] = 255;
      value[USERLED] = 255;
      leds[USERLED] = CHSV(hue[USERLED], saturation[USERLED], value[USERLED]);
      FastLED.show();
    }
    if (ledChanged[REMOTELED]) {
      ledChanged[REMOTELED] = false;
      saturation[REMOTELED] = 255;
      //  value[REMOTELED] = 255;
      leds[REMOTELED] = CHSV(hue[REMOTELED], saturation[REMOTELED], value[REMOTELED]);
      FastLED.show();
    }
    //long fade on remote led
    longFadeHandler();
  }
  //updating every 5 seconds to make sure the leds dont lose their colours
  if (millisCheck - prevlongPixelMillis > PIXELUPDATETIMELONG) {
    prevlongPixelMillis = millisCheck;
    FastLED.show();
  }
  //short fade on remote led receive or user led trigger
  fadeRGBHandler();
}

void cycleHue(int led) {
  if (hue[led] < 255) {
    hue[led]++;
  } else {
    hue[led] = 0;
  }
  ledChanged[led] = true;
}

uint16_t getUserHue() {
  return hue[USERLED];
}


void blinkRGB() {
  leds[0] = CHSV(1, 255, 255);
  leds[1] = CHSV(127, 255, 255);
  FastLED.show();
  delay(100);
  leds[0] = CHSV(1, 0, 0);
  leds[1] = CHSV(127, 0, 0);
  FastLED.show();
}

void fadeRGB(int led) {
  if (readyToFadeRGB[led] == false) {
    readyToFadeRGB[led] = true;
  }
}

void fadeRGBHandler() {
  for (byte i = 0; i < NUMPIXELS; i++) {
    if (readyToFadeRGB[i] == true && isFadingRGB[i] == false) {
      ledChanged[i] = true;
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

void startLongFade() {
  isLongFade = true;
  longFadeMinutes = LONGFADEMINUTESMAX;
  prevLongFadeVal = 0;
}

void longFadeHandler() {
  if (isLongFade) {
    if (millis() - prevLongFadeMillis > LONGFADECHECKMILLIS) {
      prevLongFadeMillis = millis();
      longFadeMinutes--;
      unsigned long currLongFadeVal = longFadeMinutes / (LONGFADEMINUTESMAX / RGBLEDPWMSTART);
      if (currLongFadeVal != prevLongFadeVal) {
        prevLongFadeVal = currLongFadeVal;
        currLongFadeVal = currLongFadeVal - 1;
        value[REMOTELED] = (byte)fscale(0,RGBLEDPWMSTART,0,RGBLEDPWMSTART,currLongFadeVal,-3);
        ledChanged[REMOTELED] = true;
        Serial.println(value[REMOTELED]);
      }
      if (longFadeMinutes <= 0) {
        isLongFade = false;
      }
    }
  }
}


int fscale(float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve) {
  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;
  // condition curve parameter
  // limit range
  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;
  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function
  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin) {
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }
  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float
  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0) {
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return (int)rangedValue;
}
