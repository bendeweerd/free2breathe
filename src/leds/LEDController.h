#include <Arduino.h>

#include "FastLED.h"

namespace f2b {

/**
 * @brief Available LED states
 */
enum LEDState {
  RED,
  YELLOW,
  GREEN,
  BLUE,
  WHITE,
  OFF,
};

struct rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

class LEDController {
 private:
  rgb colors;

 public:
  LEDState currentLEDState;
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
   * @brief Set the number of LEDs to illuminate
   */
  void SetNumLEDs(unsigned num) { num_leds = num; }

  /**
   * @brief Update LED strip based on current state
   */
  void UpdateLEDs() {
    switch (currentLEDState) {
      case WHITE:
        colors = {255, 255, 255};
        break;
      case RED:
        colors = {255, 0, 0};
        break;
      case YELLOW:
        colors = {255, 255, 0};
        break;
      case GREEN:
        colors = {0, 255, 0};
        break;
      case BLUE:
        colors = {0, 0, 255};
        break;
      case OFF:
        colors = {0, 0, 0};
        break;
    }

    for (unsigned i = 0; i < num_leds; i++) {
      leds[i] = CRGB(colors.r, colors.g, colors.b);
    }
    FastLED.show();

  };  // updateLEDs()
};
}  // namespace f2b
