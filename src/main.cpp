#include <Arduino.h>
#include <FastLED.h>

#include "leds/LEDController.h"
#include "ulp/ULP.h"

/***************************************
 * Pin Assignments
 ***************************************/

#define CO_READ_PIN A0
#define SO2_READ_PIN A1
#define O3_READ_PIN A2
#define NO2_READ_PIN A3
#define THERMISTOR_READ_PIN A4

#define CO_STATUS_LED 13
#define SO2_STATUS_LED 12
#define O3_STATUS_LED 11
#define NO2_STATUS_LED 10

#define LED_DATA_PIN 8

#define ALARM_PIN 9

/***************************************
 * Sensors
 ***************************************/

const int sensorReadTime = 2;  // seconds to read gas sensor

// Sensitivities (as shown on sensor barcodes)
// CO: 4.47 nA / ppm
// SO2: 39.23 nA / ppm
// O3: -60 nA +- 10 / ppm
// NO2: -30 nA +- 10 / ppm
const float coSensitivityFactor = 4.47;
const float so2SensitivityFactor = 39.23;
const float o3SensitivityFactor = -60;
const float no2SensitivityFactor = -30;

const float coWorryConcentration = 9;     // ppm
const float coDangerConcentration = 200;  // ppm
const float so2WorryConcentration = 2;    // ppm
const float so2DangerConcentration = 5;   // ppm
const float o3WorryConcentration = 0.1;   // ppm
const float o3DangerConcentration = 0.3;  // ppm
const float no2WorryConcentration = 0.2;  // ppm
const float no2DangerConcentration = 1;   // ppm

bool coPresent = false;
bool so2Present = false;
bool o3Present = false;
bool no2Present = false;

CO COsensor(CO_READ_PIN, THERMISTOR_READ_PIN, coSensitivityFactor);
SO2 SO2sensor(SO2_READ_PIN, THERMISTOR_READ_PIN, so2SensitivityFactor);
O3 O3sensor(O3_READ_PIN, THERMISTOR_READ_PIN, o3SensitivityFactor);
NO2 NO2sensor(NO2_READ_PIN, THERMISTOR_READ_PIN, no2SensitivityFactor);

/***************************************
 * LEDs
 ***************************************/

#define NUM_LEDS 25
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);

/***************************************
 * Thermistor
 ***************************************/
const unsigned long thermistorNominalRes = 100000;
const unsigned long thermistorSeriesResistor = 100000;
const unsigned thermistorTempNominal = 25;
const unsigned thermistorNumSamples = 50;
const unsigned thermistorBetaCoefficient = 3760;

unsigned i;
float thAverage;
long int thSampleSum;
float temp = 20.0;

/***************************************
 * Timing
 ***************************************/
double runTime = 0.0;
unsigned long sensorPreviousMillis = 0;
unsigned sensorPollPeriod = 20000;
unsigned long tempPreviousMillis = 0;
unsigned tempPeriod = 50;
unsigned long buzzerPreviousMillis = 0;
unsigned buzzerPeriod = 250;
bool alarm = false;
bool buzzerOn = false;

void setup() {
  pinMode(CO_READ_PIN, INPUT);
  pinMode(SO2_READ_PIN, INPUT);
  pinMode(O3_READ_PIN, INPUT);
  pinMode(NO2_READ_PIN, INPUT);
  pinMode(THERMISTOR_READ_PIN, INPUT);

  pinMode(CO_STATUS_LED, OUTPUT);
  pinMode(SO2_STATUS_LED, OUTPUT);
  pinMode(O3_STATUS_LED, OUTPUT);
  pinMode(NO2_STATUS_LED, OUTPUT);

  pinMode(LED_DATA_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);

  digitalWrite(CO_STATUS_LED, LOW);
  digitalWrite(SO2_STATUS_LED, LOW);
  digitalWrite(O3_STATUS_LED, LOW);
  digitalWrite(NO2_STATUS_LED, LOW);

  FastLED.addLeds<WS2812B, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  LEDController.currentLEDState = f2b::LEDState::OFF;
  LEDController.UpdateLEDs();

  // bitmasking to set buzzer PWM frequency
  TCCR2B = ((TCCR2B & B11111000) | B00000011);

  // update if micro or power supply changes
  COsensor.pVcc = SO2sensor.pVcc = O3sensor.pVcc = NO2sensor.pVcc = 4.993;
  COsensor.pVsup = SO2sensor.pVsup = O3sensor.pVsup = NO2sensor.pVsup = 3.293;

  // CO
  long int CO_R2 = 1999;  // spec 2k
  float CO_bias = 3.0;
  COsensor.setVref(CO_bias, CO_R2);
  COsensor.pGain = 99900;  // resistor R6, spec 100k

  // SO2
  long int SO2_R2 = 134100;  // spec 143k
  float SO2_bias = 217.0;
  SO2sensor.setVref(SO2_bias, SO2_R2);
  SO2sensor.pGain = 99600;  // spec 100k

  // O3
  long int O3_R2 = 16000;  // spec 16.2k
  float O3_bias = -25;
  O3sensor.setVref(O3_bias, O3_R2);
  O3sensor.pGain = 498000;  // spec 499k

  // NO2
  long int NO2_R2 = 16070;  // spec 16.2k
  float NO2_bias = -25;
  NO2sensor.setVref(NO2_bias, NO2_R2);
  NO2sensor.pGain = 497000;  // spec 499k

  Serial.begin(9600);

  // if V_ref is known replace the following code...

  // Serial.println("Remove Sensor.");
  // if (SO2sensor.OCzero(n)) {
  //   Serial.print("Vref new = ");
  //   Serial.println(SO2sensor.pVref_set);
  // } else {
  //   Serial.println("Recheck Settings, Zero out of range");
  //   while (1) {
  //     Serial.println(analogRead(SO2_READ_PIN));
  //     delay(1000);
  //   }
  // }
  // Serial.println("Finished Setting Up, Replace Sensor Now.\n");

  //...with the measured value of new Vref
  COsensor.pVref_set = 1638.31;
  SO2sensor.pVref_set = 1525.92;
  O3sensor.pVref_set = 1657.67;
  NO2sensor.pVref_set = 1657.12;

  Serial.println("\nSetting Up.");

  Serial.print("  Vsup for all sensors = ");
  Serial.println(COsensor.pVsup);
  Serial.print("  Vcc for all sensors = ");
  Serial.println(COsensor.pVcc);

  Serial.println("\nSystem starting up, please wait for stabilization.");
  Serial.println("\nData Log:");
  Serial.println("s, temp, CO PPM, SO2 PPM, O3 PPM, NO2 PPM");

  sensorPreviousMillis = millis();
  tempPreviousMillis = millis();

  // system startup delay
  for (unsigned i = 0; i <= 25; i++) {
    LEDController.currentLEDState = f2b::LEDState::BLUE;
    LEDController.SetNumLEDs(i);
    LEDController.UpdateLEDs();
    // TODO: update for desired startup time
    delay(50);
  }
}

void loop() {
  runTime = millis() / 1000.0;

  // update temperature reading
  if (millis() - tempPreviousMillis > tempPeriod) {
    thSampleSum += analogRead(THERMISTOR_READ_PIN);
    i++;

    if (i >= thermistorNumSamples) {
      thAverage = thSampleSum / i;

      // convert to resistance
      thAverage = 1023 / thAverage - 1;
      thAverage = thermistorSeriesResistor / thAverage;

      // convert resistance to temperature - see
      // https://learn.adafruit.com/thermistor/using-a-thermistor
      float steinhart;
      steinhart = thAverage / thermistorNominalRes;         // (R/Ro)
      steinhart = log(steinhart);                           // ln(R/Ro)
      steinhart /= thermistorBetaCoefficient;               // 1/B * ln(R/Ro)
      steinhart += 1.0 / (thermistorTempNominal + 273.15);  // + (1/To)
      steinhart = 1.0 / steinhart;                          // Invert
      steinhart -= 273.15;  // convert absolute temp to C

      temp = steinhart;

      thSampleSum = 0;
      thAverage = 0;
      i = 0;
    }
    tempPreviousMillis = millis();
  }

  // update sensor readings
  if (millis() - sensorPreviousMillis > (sensorPollPeriod)) {
    sensorPreviousMillis = millis();

    // turn off lights and alarm to reduce noise while reading
    LEDController.currentLEDState = f2b::LEDState::OFF;
    LEDController.UpdateLEDs();
    digitalWrite(CO_STATUS_LED, LOW);
    digitalWrite(SO2_STATUS_LED, LOW);
    digitalWrite(O3_STATUS_LED, LOW);
    digitalWrite(NO2_STATUS_LED, LOW);
    analogWrite(ALARM_PIN, 0);
    delay(1000);

    COsensor.getIgas(sensorReadTime);
    COsensor.setTemp(temp);
    COsensor.getConc(temp);

    SO2sensor.getIgas(sensorReadTime);
    SO2sensor.setTemp(temp);
    SO2sensor.getConc(temp);

    O3sensor.getIgas(sensorReadTime);
    O3sensor.setTemp(temp);
    O3sensor.getConc(temp);

    NO2sensor.getIgas(sensorReadTime);
    NO2sensor.setTemp(temp);
    NO2sensor.getConc(temp);

    // scale reading based on test data
    float COppm = COsensor.convertX('B') / 1.5;
    float SO2ppm = SO2sensor.convertX('B') / 1.5;
    float O3ppm = O3sensor.convertX('B') / 1.5;
    float NO2ppm = NO2sensor.convertX('B') / 1.5;

    alarm = false;
    if (COppm >= coDangerConcentration || SO2ppm >= so2DangerConcentration ||
        O3ppm >= o3DangerConcentration || NO2ppm >= no2DangerConcentration) {
      alarm = true;
    }

    coPresent = false;
    so2Present = false;
    o3Present = false;
    no2Present = false;

    if (COppm > coWorryConcentration) {
      coPresent = true;
    }
    if (SO2ppm > so2WorryConcentration) {
      so2Present = true;
    }
    if (O3ppm > o3WorryConcentration) {
      o3Present = true;
    }
    if (NO2ppm > no2WorryConcentration) {
      no2Present = true;
    }

    if (alarm) {
      LEDController.currentLEDState = f2b::LEDState::RED;
    } else if (coPresent || so2Present || o3Present || no2Present) {
      LEDController.currentLEDState = f2b::LEDState::YELLOW;
    } else {
      LEDController.currentLEDState = f2b::LEDState::GREEN;
    }

    digitalWrite(CO_STATUS_LED, coPresent);
    digitalWrite(SO2_STATUS_LED, so2Present);
    digitalWrite(O3_STATUS_LED, o3Present);
    digitalWrite(NO2_STATUS_LED, no2Present);

    LEDController.UpdateLEDs();

    Serial.print(runTime);
    Serial.print(", ");
    Serial.print(temp);
    Serial.print(", ");
    Serial.print(COppm);
    Serial.print(", ");
    Serial.print(SO2ppm);
    Serial.print(", ");
    Serial.print(O3ppm);
    Serial.print(", ");
    Serial.println(NO2ppm);
  }

  // sound alarm if appropriate
  if (alarm) {
    if (millis() - buzzerPreviousMillis >= buzzerPeriod) {
      buzzerOn = !buzzerOn;
      buzzerPreviousMillis = millis();
    }
    if (buzzerOn) {
      analogWrite(ALARM_PIN, 128);
    } else {
      analogWrite(ALARM_PIN, 0);
    }
  } else {
    analogWrite(ALARM_PIN, 0);
  }
}