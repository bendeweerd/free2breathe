#include <Arduino.h>
#include <FastLED.h>

#include "leds/LEDController.h"
#include "ulp/ULP.h"

/***************************************
 * Sensors
 ***************************************/
#define C1 A0
#define C2 A1
#define C3 A2
#define C4 A3
#define T1 A4

#define ALARM_PIN 9

// sensor averaging times, keep these low, so that the ADC read does not
// overflow 32 bits.
const int n = 2;  // seconds to read gas sensor
const int s = 3;  // seconds to read all sensors, should be greater than n+m+1

// Sensitivities (as shown on sensor barcodes)
// CO: 4.47 nA / ppm
// SO2: 39.23 nA / ppm
// O3: ~ -60 nA +- 10 / ppm
// NO2: ~ -30 nA +- 10 / ppm
const float Sf1 = 4.47;
const float Sf2 = 39.23;
const float Sf3 = -60;
const float Sf4 = -30;

CO sensor1(C1, T1, Sf1);
SO2 sensor2(C2, T1, Sf2);
O3 sensor3(C3, T1, Sf3);
NO2 sensor4(C4, T1, Sf4);

#define CO_WORRY 9
#define CO_DANGER 200

/***************************************
 * LEDs
 ***************************************/
#define STATUS_LED_1 13
#define STATUS_LED_2 12
#define STATUS_LED_3 11
#define STATUS_LED_4 10

#define NUM_LEDS 25
#define LED_DATA_PIN 8
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

f2b::LEDController LEDController = f2b::LEDController(leds, NUM_LEDS);

/***************************************
 * Thermistor
 ***************************************/
#define TH_NOMINAL_RES 100000
#define TH_TEMP_NOMINAL 25
#define TH_NUM_SAMPLES 50
#define TH_B_COEFF 3760
#define TH_SERIES_RES 100000
uint8_t i;
float th_average;
long int th_sample_sum;
float temp = 20.0;

/***************************************
 * Timing
 ***************************************/
unsigned long sensorPreviousMillis = 0;
unsigned long tempPreviousMillis = 0;
double runTime = 0.0;
unsigned long buzzerPreviousMillis = 0;
unsigned buzzerPeriod = 250;
bool alarm = false;
bool buzzerOn = false;

void setup() {
  pinMode(C1, INPUT);
  pinMode(C2, INPUT);
  pinMode(C3, INPUT);
  pinMode(C4, INPUT);
  pinMode(T1, INPUT);

  pinMode(STATUS_LED_1, OUTPUT);
  pinMode(STATUS_LED_2, OUTPUT);
  pinMode(STATUS_LED_3, OUTPUT);
  pinMode(STATUS_LED_4, OUTPUT);

  pinMode(LED_DATA_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);

  digitalWrite(STATUS_LED_1, LOW);
  digitalWrite(STATUS_LED_2, LOW);
  digitalWrite(STATUS_LED_3, LOW);
  digitalWrite(STATUS_LED_4, LOW);

  FastLED.addLeds<WS2812B, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(20);
  LEDController.currentLEDState = f2b::LEDState::OFF;
  LEDController.UpdateLEDs();

  // bitmasking to set buzzer PWM frequency
  TCCR2B = TCCR2B & B11111000 | B00000011;

  sensor1.pVcc = 4.993;  // TODO: update if micro or power supply changes
  sensor1.pVsup = 3.293;
  sensor2.pVcc = sensor1.pVcc;
  sensor2.pVsup = sensor1.pVsup;
  sensor3.pVcc = sensor1.pVcc;
  sensor3.pVsup = sensor1.pVsup;
  sensor4.pVcc = sensor1.pVcc;
  sensor4.pVsup = sensor1.pVsup;

  // CO
  long int CO_R2 = 1999;  // spec 2k
  float CO_bias = 3.0;
  sensor1.setVref(CO_bias, CO_R2);
  sensor1.pGain = 99900;  // resistor R6, spec 100k

  // SO2
  long int SO2_R2 = 134100;  // spec 143k
  float SO2_bias = 217.0;
  sensor2.setVref(SO2_bias, SO2_R2);
  sensor2.pGain = 99600;  // spec 100k

  // O3
  long int O3_R2 = 16000;  // spec 16.2k
  float O3_bias = -25;
  sensor3.setVref(O3_bias, O3_R2);
  sensor3.pGain = 498000;  // spec 499k

  // NO2
  long int NO2_R2 = 16070;  // spec 16.2k
  float NO2_bias = -25;
  sensor4.setVref(NO2_bias, NO2_R2);
  sensor4.pGain = 497000;  // spec 499k

  Serial.begin(9600);

  // if you know the V_ref replace the following code...
  // Serial.println("Remove Sensor.");
  // if (sensor2.OCzero(n)) {
  //   Serial.print("Vref new = ");
  //   Serial.println(sensor2.pVref_set);
  // } else {
  //   Serial.println("Recheck Settings, Zero out of range");
  //   while (1) {
  //     Serial.println(analogRead(A0));
  //     delay(1000);
  //   }
  // }
  // Serial.println("Finished Setting Up, Replace Sensor Now.\n");2

  //...with this code and your measured value of new Vref
  sensor1.pVref_set = 1638.31;
  // sensor2.pVref_set = ;
  // sensor3.pVref_set = ;
  // sensor4.pVref_set = ;

  Serial.println("\nSetting Up.");

  Serial.print("  Vsup for all sensors = ");
  Serial.println(sensor1.pVsup);
  Serial.print("  Vcc for all sensors = ");
  Serial.println(sensor1.pVcc);
  Serial.print("  Vref for sensor 1 = ");
  Serial.println(sensor1.pVref);

  Serial.println("\nSystem starting up, please wait for stabilization.");
  Serial.println("\n\nData Log:");
  Serial.println("s, temp, PPM");

  sensorPreviousMillis = millis();
  tempPreviousMillis = millis();

  // system startup delay
  for (unsigned i = 0; i <= 25; i++) {
    LEDController.currentLEDState = f2b::LEDState::SOLID_BLUE;
    LEDController.SetNumLEDs(i);
    LEDController.UpdateLEDs();
    delay(1000);
  }
}

void loop() {
  runTime = millis() / 1000.0;

  // update temperature, sample every 50 milliseconds
  if (millis() - tempPreviousMillis > 50) {
    th_sample_sum += analogRead(T1);
    i++;

    if (i >= TH_NUM_SAMPLES) {
      th_average = th_sample_sum / i;

      // convert to resistance
      th_average = 1023 / th_average - 1;
      th_average = TH_SERIES_RES / th_average;

      float steinhart;
      steinhart = th_average / TH_NOMINAL_RES;        // (R/Ro)
      steinhart = log(steinhart);                     // ln(R/Ro)
      steinhart /= TH_B_COEFF;                        // 1/B * ln(R/Ro)
      steinhart += 1.0 / (TH_TEMP_NOMINAL + 273.15);  // + (1/To)
      steinhart = 1.0 / steinhart;                    // Invert
      steinhart -= 273.15;  // convert absolute temp to C

      temp = steinhart;

      th_sample_sum = 0;
      i = 0;
      th_average = 0;
    }
    tempPreviousMillis = millis();
  }

  // read sensor values
  if (millis() - sensorPreviousMillis > (10000)) {
    // flash status LEDs
    // digitalWrite(STATUS_LED_1, !digitalRead(STATUS_LED_1));
    // digitalWrite(STATUS_LED_2, !digitalRead(STATUS_LED_2))2;
    // digitalWrite(STATUS_LED_3, !digitalRead(STATUS_LED_3));
    // digitalWrite(STATUS_LED_4, !digitalRead(STATUS_LED_4));

    sensorPreviousMillis = millis();

    LEDController.currentLEDState = f2b::LEDState::OFF;
    LEDController.UpdateLEDs();
    digitalWrite(STATUS_LED_1, LOW);
    analogWrite(ALARM_PIN, 0);

    sensor1.getIgas(n);
    sensor1.setTemp(temp);
    sensor1.getConc(temp);

    // scale reading based on test data
    float ppm = sensor1.convertX('B') / 1.5;

    if (ppm > CO_DANGER) {
      LEDController.currentLEDState = f2b::LEDState::SOLID_RED;
      alarm = true;
    } else if (ppm > CO_WORRY) {
      LEDController.currentLEDState = f2b::LEDState::SOLID_YELLOW;
      alarm = true;
    } else {
      LEDController.currentLEDState = f2b::LEDState::SOLID_GREEN;
      alarm = false;
    }
    // LEDController.SetNumLEDs(map((long)ppm, -10, 200, 0, 25));

    LEDController.UpdateLEDs();

    // sensor2.getIgas(n);
    // sensor2.getTemp(m);
    // sensor2.getConc(20);

    // sensor3.getIgas(n);
    // sensor3.getTemp(m);
    // sensor3.getConc(20);

    // sensor4.getIgas(n);
    // sensor4.getTemp(m);
    // sensor4.getConc(20);

    Serial.print(runTime);
    Serial.print(", ");
    // Serial.print(analogRead(A0));
    // Serial.print(", ");
    Serial.print(temp);
    Serial.print(", ");
    // Serial.print(sensor1.pVgas);
    // Serial.print(", ");
    // Serial.print(sensor1.pInA);
    // Serial.print(", ");
    Serial.println(ppm);
    // Serial.print(sensor1.convertX('B'));
    // Serial.print(", ");
    // Serial.println(sensor1.convertX('M'));

    // delay(1000);
    // digitalWrite(STATUS_LED_1, HIGH);
    // digitalWrite(STATUS_LED_2, HIGH);
    // digitalWrite(STATUS_LED_3, HIGH);
    // digitalWrite(STATUS_LED_4, HIGH);
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
    digitalWrite(STATUS_LED_1, HIGH);

  } else {
    analogWrite(ALARM_PIN, 0);
    digitalWrite(STATUS_LED_1, LOW);
  }

  // LEDController.UpdateLEDs();
}