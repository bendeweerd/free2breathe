#include <Arduino.h>
#include <FastLED.h>
#include <string.h>

#include "leds/LEDController.h"
#define NUM_LEDS 5
#define LED_DATA_PIN 8
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned period = 4000;
bool leds_on = true;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, LED_DATA_PIN>(leds, NUM_LEDS)
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
        LEDController.ChangeState(f2b::LEDState::PULSE_BLUE);
        break;
    }

    previousMillis = currentMillis;
  }

  LEDController.UpdateLEDs();
}