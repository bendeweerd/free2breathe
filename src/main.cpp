#include <Arduino.h>
#include <FastLED.h>

#include "leds/LEDController.h"
#include "sensors/COsensor.h"

#define NUM_LEDS 135
#define LED_DATA_PIN 8
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);
COSensor sensor0(A0, A1);

float coPPM = 0;

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();

  LEDController.currentLEDState = f2b::LEDState::PULSE_BLUE;

  analogReference(INTERNAL);  // for sensor reading
}

void loop() {
  coPPM = sensor0.read();

  if (coPPM > 10) {
    LEDController.currentLEDState = f2b::LEDState::PULSE_RED;
  } else if (coPPM > 5) {
    LEDController.currentLEDState = f2b::LEDState::PULSE_YELLOW;
  } else {
    LEDController.currentLEDState = f2b::LEDState::LOADING_SPIN;
  }

  LEDController.UpdateLEDs();
}