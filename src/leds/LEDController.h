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
  int pulseBrightness = 0;
  int pulseDirection = 1;
  unsigned pulseWaitTime = 1;

  bool pulse = false;

  unsigned int r = 0;
  unsigned int g = 0;
  unsigned int b = 0;

 public:
  LEDState currentLEDState;
  int num_leds = 0;
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
   * @brief Change LED state
   * @param newState: new state
   */
  void ChangeState(LEDState newState) { currentLEDState = newState; }

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
        pulse = true;
        break;
      case PULSE_GREEN:
        r = 0;
        g = 255;
        b = 0;
        pulse = true;
        break;
      case SOLID_RED:
        r = 255;
        g = 0;
        b = 0;
        pulse = false;
        break;
      case PULSE_RED:
        r = 255;
        g = 0;
        b = 0;
        pulse = true;
        break;
      case SOLID_YELLOW:
        r = 255;
        g = 255;
        b = 0;
        pulse = false;
      case PULSE_YELLOW:
        r = 255;
        g = 255;
        b = 0;
        pulse = true;
    }

    if (pulse) {
      applyPulse();
    }

    for (int i = 0; i < num_leds; i++) {
      leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
  };  // updateLEDs()

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
  }
};
}  // namespace f2b
