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

// sensor averaging times
const int n = 2;  // seconds to read gas sensor
const int s = 3;  // seconds to read all sensors, should be greater than n+m+1

// Sensitivities (as shown on sensor barcodes)
// CO: 4.47 nA / ppm
// SO2: 39.23 nA / ppm
// O3: -60 nA +- 10 / ppm
// NO2: -30 nA +- 10 / ppm
const float Sf1 = 4.47;
const float Sf2 = 39.23;
const float Sf3 = -60;
const float Sf4 = -30;

// TODO: update from specs
const unsigned coWorryConcentration = 9;        // ppm
const unsigned coDangerConcentration = 200;     // ppm
const unsigned so2WorryConcentration = 100;     // ppb
const unsigned so2DangerConcentration = 10000;  // ppb
const unsigned o3WorryConcentration = 400;      // ppm
const unsigned o3DangerConcentration = 10000;   // ppm
const unsigned no2WorryConcentration = 3;       // ppm
const unsigned no2DangerConcentration = 10000;  // ppm

bool coPresent = false;
bool so2Present = false;
bool o3Present = false;
bool no2Present = false;

CO COsensor(CO_READ_PIN, THERMISTOR_READ_PIN, Sf1);
SO2 SO2sensor(SO2_READ_PIN, THERMISTOR_READ_PIN, Sf2);
O3 O3sensor(O3_READ_PIN, THERMISTOR_READ_PIN, Sf3);
NO2 NO2sensor(NO2_READ_PIN, THERMISTOR_READ_PIN, Sf4);

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
unsigned sensorPollPeriod = 10000;
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

  COsensor.pVcc = 4.993;  // TODO: update if micro or power supply changes
  COsensor.pVsup = 3.293;
  SO2sensor.pVcc = COsensor.pVcc;
  SO2sensor.pVsup = COsensor.pVsup;
  O3sensor.pVcc = COsensor.pVcc;
  O3sensor.pVsup = COsensor.pVsup;
  NO2sensor.pVcc = COsensor.pVcc;
  NO2sensor.pVsup = COsensor.pVsup;

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

  // if you know the V_ref replace the following code...
  // Serial.println("Remove Sensor.");
  // if (SO2sensor.OCzero(n)) {
  //   Serial.print("Vref new = ");
  //   Serial.println(SO2sensor.pVref_set);
  // } else {
  //   Serial.println("Recheck Settings, Zero out of range");
  //   while (1) {
  //     Serial.println(analogRead(A0));
  //     delay(1000);
  //   }
  // }
  // Serial.println("Finished Setting Up, Replace Sensor Now.\n");

  //...with this code and your measured value of new Vref
  COsensor.pVref_set = 1638.31;
  // SO2sensor.pVref_set = ;
  // O3sensor.pVref_set = ;
  // NO2sensor.pVref_set = ;

  Serial.println("\nSetting Up.");

  Serial.print("  Vsup for all sensors = ");
  Serial.println(COsensor.pVsup);
  Serial.print("  Vcc for all sensors = ");
  Serial.println(COsensor.pVcc);
  Serial.print("  Vref for sensor 1 = ");
  Serial.println(COsensor.pVref);

  Serial.println("\nSystem starting up, please wait for stabilization.");
  Serial.println("\n\nData Log:");
  Serial.println("s, temp, PPM");

  sensorPreviousMillis = millis();
  tempPreviousMillis = millis();

  // system startup delay
  for (unsigned i = 0; i <= 25; i++) {
    LEDController.currentLEDState = f2b::LEDState::BLUE;
    LEDController.SetNumLEDs(i);
    LEDController.UpdateLEDs();
    // TODO: update for desired startup time
    delay(1000);
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

    // delay to prevent effect of current draw
    delay(1000);

    COsensor.getIgas(n);
    COsensor.setTemp(temp);
    COsensor.getConc(temp);

    // SO2sensor.getIgas(n);
    // SO2sensor.getTemp(m);
    // SO2sensor.getConc(20);

    // O3sensor.getIgas(n);
    // O3sensor.getTemp(m);
    // O3sensor.getConc(20);

    // NO2sensor.getIgas(n);
    // NO2sensor.getTemp(m);
    // NO2sensor.getConc(20);

    // scale reading based on test data
    // TODO: determine scale for each gas
    float COppm = COsensor.convertX('B') / 1.5;
    float SO2ppm = 0;
    float O3ppm = 0;
    float NO2ppm = 0;

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
    // if (SO2ppm > so2WorryConcentration) {
    //   so2Present = true;
    // }
    // if (O3ppm > o3WorryConcentration) {
    //   o3Present = true;
    // }
    // if (no2Present > no2WorryConcentration) {
    //   no2Present = true;
    // }

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
    Serial.println(COppm);
  }

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
    digitalWrite(CO_STATUS_LED, HIGH);

  } else {
    analogWrite(ALARM_PIN, 0);
    digitalWrite(CO_STATUS_LED, LOW);
  }

  // LEDController.UpdateLEDs();
}