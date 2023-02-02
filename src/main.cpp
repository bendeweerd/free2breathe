#include <Arduino.h>
#include <FastLED.h>

#include "leds/LEDController.h"
#include "ulp/ULP.h"

/***************************************
 * LEDs
 ***************************************/
#define NUM_LEDS 5
#define LED_DATA_PIN 8
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);

/***************************************
 * Timing
 ***************************************/
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned waitTime = 100;

/***************************************
 * ULP
 ***************************************/
const int C1 = A0;
const int C2 = A1;
const int C3 = A2;
const int C4 = A3;
const int T1 = A4;

// averaging times, keep these low, so that the ADC read does not overflow 32
// bits. For example n = 5 reads ADC 4465 times which could add to 22bit number.
const int n = 2;  // seconds to read gas sensor
const int m = 1;  // seconds to read temperature sensor
const int s = 4;  // seconds to read all sensors, should be greater than n+m+1

// Sensitivities (as shown on sensor barcode, in nA/ppm):
// CO: 4.47 nA / ppm
// SO2: 39.23 nA / ppm
// O3: ~ -60 nA +- 10 / ppm
// NO2: ~ -30 nA +- 10 / ppm
const float Sf1 = 4.47;
const float Sf2 = 39.23;
const float Sf3 = -60;
const float Sf4 = -30;

unsigned long etime;

CO sensor1(C1, T1, Sf1);
SO2 sensor2(C2, T1, Sf2);
O3 sensor3(C3, T1, Sf3);
NO2 sensor4(C4, T1, Sf4);

void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);

  FastLED.addLeds<WS2812B, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();

  LEDController.currentLEDState = f2b::LEDState::PULSE_BLUE;

  sensor1.pVcc = 4.74;  // TODO: update if micro swapped
  sensor1.pVsup = 3.23;

  Serial.begin(9600);

  Serial.println();
  Serial.println("Setting Up.");

  Serial.print("  Vsup for all sensors = ");
  Serial.println(sensor1.pVsup);
  Serial.print("  Vcc for all sensors = ");
  Serial.println(sensor1.pVcc);
  Serial.print("  Vref for sensor 1 = ");
  Serial.println(sensor1.pVref);

  //  Using resistor values from board R1, R2, R3 are for setting pVref and
  //  Bias, while R6 sets the gain If using modified or custom boards set Vref
  //  and Gain like this

  // CO
  long int CO_R2 = 1990;  // Assumes R1 and R3 are 1 MOhms in resistor ladder
  float CO_bias = -3.0;
  sensor1.setVref(CO_bias, CO_R2);
  sensor1.pGain = 100000;  // resistor R6

  // SO2

  // O3

  // NO2

  // if you know the V_ref replace the following code...
  //  Serial.println("Remove CO Sensor.");
  //  if (sensor1.OCzero(n)) {
  //    Serial.print("Vref new = ");
  //    Serial.println(sensor1.pVref_set);
  //  } else {
  //    Serial.println("Recheck Settings, Zero out of range");
  //    while (1) {
  //      Serial.println(analogRead(A0));
  //      delay(1000);
  //    }sensor1
  //  }

  // Serial.println("Finished Setting Up, Replace Sensor Now.\n");
  // Serial.println("T1, mV, nA, PPM");
  // etime = millis();

  //...with this code and your measured value of new Vref
  sensor1.pVref_set = 1613.39;

  etime = millis();
}

void loop() {
  // if zero command is sent, zero the sensor
  while (Serial.available()) {
    if (Serial.read() == 'Z') {
      Serial.println("Zeroing");
      sensor1.zero();
      Serial.print("  sensor1: ");
      Serial.print(sensor1.pIzero);
      Serial.print(", ");
      Serial.println(sensor1.pTzero);

      // sensor2.zero();
      // Serial.print("  sensor2: ");
      // Serial.print(sensor2.pIzero);
      // Serial.print(", ");
      // Serial.println(sensor2.pTzero);

      // Serial.print("  sensor3: ");
      // Serial.print(sensor3.pIzero);
      // Serial.print(", ");
      // Serial.println(sensor3.pTzero);

      // Serial.print("  sensor4: ");
      // Serial.print(sensor4.pIzero);
      // Serial.print(", ");
      // Serial.println(sensor4.pTzero);
    }
  }

  // loop, read sensor values
  if (millis() - etime > (s * 1000)) {
    etime = millis();

    sensor1.getIgas(n);
    sensor1.getTemp(m);
    sensor1.getConc(20);

    // sensor2.getIgas(n);
    // sensor2.getTemp(m);
    // sensor2.getConc(20);

    // sensor3.getIgas(n);
    // sensor3.getTemp(m);
    // sensor3.getConc(20);

    // sensor4.getIgas(n);
    // sensor4.getTemp(m);
    // sensor4.getConc(20);

    Serial.print(sensor1.convertT('C'));  // use 'C' or 'F' for units
    Serial.print(", ");
    Serial.print(sensor1.pVgas);
    Serial.print(", ");
    Serial.print(sensor1.pInA);
    Serial.print(", ");
    Serial.println(sensor1.convertX('M'));
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