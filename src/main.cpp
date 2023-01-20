#include <Arduino.h>
#include <FastLED.h>

#include "leds/LEDController.h"
// #include "sensors/COsensor.h"
// #include "sensors/NO2Sensor.h"
// #include "sensors/O3sensor.h"
// #include "sensors/SO2sensor.h"
#include "sensors/VoltageReader.h"

#define NUM_LEDS 135
#define LED_DATA_PIN 8
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);
// COSensor coSensor(A0, A1);
// O3Sensor o3Sensor(A2, A3);
// SO2Sensor so2Sensor(A4, A5);
// NO2Sensor no2Sensor(A6, A7);
VoltageReader CO(A0, A1);
VoltageReader SO2(A2, A3);
VoltageReader O3(A4, A5);
VoltageReader NO2(A6, A7);

float COVoltage;
float SO2Voltage;
float O3Voltage;
float NO2Voltage;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned waitTime = 100;

// float coPPM = 0;
// float o3PPM = 0;
// float so2PPM = 0;
// float no2PPM = 0;

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
  COVoltage = CO.read() * 1000;
  SO2Voltage = SO2.read() * 1000;
  O3Voltage = O3.read() * 1000;
  NO2Voltage = NO2.read() * 1000;

  currentMillis = millis();
  if (currentMillis - previousMillis >= waitTime) {
    Serial.print(COVoltage);
    Serial.print(",");
    Serial.print(SO2Voltage);
    Serial.print(",");
    Serial.print(O3Voltage);
    Serial.print(",");
    Serial.println(NO2Voltage);
    previousMillis = currentMillis;
  }

  // coPPM = coSensor.read();
  // o3PPM = o3Sensor.read();
  // so2PPM = so2Sensor.read();
  // no2PPM = no2Sensor.read();

  // if (coPPM > 10) {
  //   LEDController.currentLEDState = f2b::LEDState::PULSE_RED;
  // } else if (coPPM > 5) {
  //   LEDController.currentLEDState = f2b::LEDState::PULSE_YELLOW;
  // } else {
  //   LEDController.currentLEDState = f2b::LEDState::LOADING_SPIN;
  // }

  LEDController.UpdateLEDs();
}