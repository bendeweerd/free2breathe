#include <Arduino.h>
#include <FastLED.h>

#include "leds/LEDController.h"
#include "sensors/COsensor.h"
#include "sensors/NO2Sensor.h"
#include "sensors/O3sensor.h"
#include "sensors/SO2sensor.h"
// #include "sensors/VoltageReader.h"

#define NUM_LEDS 135
#define LED_DATA_PIN 8
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);
COSensor coSensor(A0, A1);
// SO2Sensor so2Sensor(A2, A3);
// O3Sensor o3Sensor(A4, A5);
// NO2Sensor no2Sensor(A6, A7);

// VoltageReader CO(A0, A1);
// VoltageReader SO2(A2, A3);
// VoltageReader O3(A4, A5);
// VoltageReader NO2(A6, A7);

// double COVoltage;
// double SO2Voltage;
// double O3Voltage;
// double NO2Voltage;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned waitTime = 100;

double coPPM = 0;
double o3PPM = 0;
double so2PPM = 0;
double no2PPM = 0;

// unsigned long currentMillis = 0;
// unsigned long previousMillis = 0;

void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);

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
  // o3PPM = o3Sensor.read();
  // so2PPM = so2Sensor.read();
  // no2PPM = no2Sensor.read();

  currentMillis = millis();
  if (currentMillis - previousMillis >= waitTime) {
    Serial.println(coPPM, 10);
    // Serial.print(coPPM);
    // Serial.print(",");
    // Serial.print(so2PPM);
    // Serial.print(",");
    // Serial.print(o3PPM);
    // Serial.print(",");
    // Serial.println(no2PPM);
    previousMillis = currentMillis;
  }

  // if (coPPM > 10) {
  //   LEDController.currentLEDState = f2b::LEDState::PULSE_RED;
  // } else if (coPPM > 5) {
  //   LEDController.currentLEDState = f2b::LEDState::PULSE_YELLOW;
  // } else {
  //   LEDController.currentLEDState = f2b::LEDState::LOADING_SPIN;
  // }

  LEDController.UpdateLEDs();
}