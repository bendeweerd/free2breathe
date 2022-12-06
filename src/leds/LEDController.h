#include <Arduino.h>

#include "FastLED.h"

namespace f2b {

/**
 * @brief Available LED states
 */
enum LEDState {
  PULSE_BLUE,
  PULSE_GREEN,
  SOLID_RED,
  PULSE_RED,
  SOLID_YELLOW,
  PULSE_YELLOW,
  LOADING_SPIN,
};

/**
 * @brief Available LED effects
 */
enum LEDEffect {
  PULSE,
  SOLID,
  LOADING,
};

/*
colors:
  - red
  - green
  - yellow
*/

class LEDController {
 private:
  unsigned long currentMillis = 0;
  unsigned long previousMillis = 0;
  unsigned pulseBrightness = 0;
  unsigned pulseDirection = 1;
  unsigned pulseWaitTime = 8;

  unsigned loadingWaitTime = 150;
  unsigned loadingLeadDot = 0;

  bool pulse = false;
  bool loading = false;

  unsigned int r = 0;
  unsigned int g = 0;
  unsigned int b = 0;

 public:
  LEDState currentLEDState;
  LEDEffect currentLEDEffect;
  unsigned num_leds = 0;
  CRGB* leds = 0;

  /**
   * @brief LEDController constructor
   * @param ledArray: pointer to CRGB led array
   * @param numLeds: number of LEDs in strip
   **/
  LEDController(CRGB* ledArray, int numLeds) {
    leds = ledArray;
    num_leds = numLeds;
  }

  /**
   * @brief Update LED strip with current state and brightness
   */
  void UpdateLEDs() {
    currentMillis = millis();

    switch (currentLEDState) {
      case PULSE_BLUE:
        r = 0;
        g = 0;
        b = 255;
        currentLEDEffect = PULSE;
        break;
      case PULSE_GREEN:
        r = 0;
        g = 255;
        b = 0;
        currentLEDEffect = PULSE;
        break;
      case SOLID_RED:
        r = 255;
        g = 0;
        b = 0;
        currentLEDEffect = SOLID;
        break;
      case PULSE_RED:
        r = 255;
        g = 0;
        b = 0;
        currentLEDEffect = PULSE;
        break;
      case SOLID_YELLOW:
        r = 255;
        g = 255;
        b = 0;
        currentLEDEffect = SOLID;
        break;
      case PULSE_YELLOW:
        r = 255;
        g = 255;
        b = 0;
        currentLEDEffect = PULSE;
        break;
      case LOADING_SPIN:
        r = 0;
        g = 0;
        b = 255;
        currentLEDEffect = LOADING;
        break;
    }

    switch (currentLEDEffect) {
      case PULSE:
        applyPulse();
        break;
      case LOADING:
        applyLoading();
        break;
      default:
        for (unsigned i = 0; i < num_leds; i++) {
          leds[i] = CRGB(r, g, b);
        }
        FastLED.show();
        break;
    }
  };  // updateLEDs()

  /**
   * @brief Helper method to apply pulse effect
   */
  void applyPulse() {
    if (currentMillis - previousMillis >= pulseWaitTime) {
      if (pulseDirection == 1) {
        pulseBrightness += 3;
      } else if (pulseDirection == 0) {
        pulseBrightness -= 3;
      }
      previousMillis = currentMillis;
    }

    if (pulseBrightness <= 0) {
      pulseBrightness = 0;
      pulseDirection = 1;
    } else if (pulseBrightness >= 255) {
      pulseBrightness = 255;
      pulseDirection = 0;
    }

    r = map(r, 0, 255, 0, pulseBrightness);
    g = map(g, 0, 255, 0, pulseBrightness);
    b = map(b, 0, 255, 0, pulseBrightness);

    for (unsigned i = 0; i < num_leds; i++) {
      leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
  }

  /**
   * @brief Helper method to apply loading effect
   */
  void applyLoading() {
    if (currentMillis - previousMillis >= loadingWaitTime) {
      if (loadingLeadDot >= num_leds) {
        loadingLeadDot = 0;
      }

      leds[loadingLeadDot] = CRGB(r, g, b);
      FastLED.show();
      loadingLeadDot++;

      for (unsigned i = 0; i < num_leds; i++) {
        leds[i].fadeToBlackBy(64);
      }

      previousMillis = currentMillis;
    }
  }
};
}  // namespace f2b
