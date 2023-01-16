#include <Arduino.h>
#include <FastLED.h>

#include "leds/LEDController.h"
#include "sensors/COsensor.h"
#include "sensors/NO2Sensor.h"
#include "sensors/O3sensor.h"
#include "sensors/SO2sensor.h"

#define NUM_LEDS 135
#define LED_DATA_PIN 8
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);
COSensor coSensor(A0, A1);
O3Sensor o3Sensor(A2, A3);
SO2Sensor so2Sensor(A4, A5);
NO2Sensor no2Sensor(A6, A7);

float coPPM = 0;
float o3PPM = 0;
float so2PPM = 0;
float no2PPM = 0;

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
  coPPM = coSensor.read();
  o3PPM = o3Sensor.read();
  so2PPM = so2Sensor.read();
  no2PPM = no2Sensor.read();

  // if (coPPM > 10) {
  //   LEDController.currentLEDState = f2b::LEDState::PULSE_RED;
  // } else if (coPPM > 5) {
  //   LEDController.currentLEDState = f2b::LEDState::PULSE_YELLOW;
  // } else {
  //   LEDController.currentLEDState = f2b::LEDState::LOADING_SPIN;
  // }

  LEDController.UpdateLEDs();
}