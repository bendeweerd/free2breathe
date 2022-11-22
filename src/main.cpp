#include <Arduino.h>
#include <FastLED.h>

#include "leds/LEDController.h"
#define NUM_LEDS 135
#define LED_DATA_PIN 8
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned period = 4000;
bool leds_on = true;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();

  LEDController.currentLEDState = f2b::LEDState::PULSE_BLUE;
}

void loop() {
  currentMillis = millis();

  // loop through LED states
  if (currentMillis - previousMillis > period) {
    // int sensorValue = analogRead(A0);
    // Serial.println(sensorValue);

    // rotate through available LED states
    switch (LEDController.currentLEDState) {
      case f2b::LEDState::PULSE_BLUE:
        LEDController.ChangeState(f2b::LEDState::PULSE_GREEN);
        break;
      case f2b::LEDState::PULSE_GREEN:
        LEDController.ChangeState(f2b::LEDState::SOLID_RED);
        break;
      case f2b::LEDState::SOLID_RED:
        LEDController.ChangeState(f2b::LEDState::PULSE_RED);
        break;
      case f2b::LEDState::PULSE_RED:
        LEDController.ChangeState(f2b::LEDState::SOLID_YELLOW);
        break;
      case f2b::LEDState::SOLID_YELLOW:
        LEDController.ChangeState(f2b::LEDState::PULSE_YELLOW);
        break;
      case f2b::LEDState::PULSE_YELLOW:
        LEDController.ChangeState(f2b::LEDState::LOADING_SPIN);
        break;
      case f2b::LEDState::LOADING_SPIN:
        LEDController.ChangeState(f2b::LEDState::PULSE_BLUE);
        break;
    }

    previousMillis = currentMillis;
  }

  // LEDController.ChangeState(f2b::LEDState::LOADING_SPIN);

  LEDController.UpdateLEDs();
}