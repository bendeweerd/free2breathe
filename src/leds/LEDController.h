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
};

class LEDController {
 private:
  unsigned maxBrightness = 255;
  unsigned currentBrightness = 0;
  unsigned pulsePeriod = maxBrightness * 2;
  bool brighter = true;

  unsigned long currentMillis = 0;
  unsigned long previousMillis = 0;

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
    switch (currentLEDState) {
      case PULSE_BLUE:
        r = 0;
        g = 0;
        b = 255;
        break;
      case PULSE_GREEN:
        r = 0;
        g = 255;
        b = 0;
        break;
      case SOLID_RED:
        r = 255;
        g = 0;
        b = 0;
        break;
      case PULSE_RED:
        r = 255;
        g = 0;
        b = 0;
        break;
    }
    for (int i = 0; i < num_leds; i++) {
      leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
  };
};
}  // namespace f2b