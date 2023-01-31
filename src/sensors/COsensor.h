#include <Arduino.h>

class COSensor {
 private:
  int refPin;  // reference pin, TP2 on schematic
  int subPin;  // subtraction pin, TP1 on schematic

  int i = 0;
  const int numPolls = 64;
  bool updateAvg = false;

  long int refPinReadSum = 0;
  long int subPinReadSum = 0;
  long int readDiff = 0;
  double diffAvg = 0;
  double voltage = 0;

  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;
  unsigned waitTime = 3;

  double resValue = 10.0;
  double amps = 0.0;
  double nA = 0.0;
  double ppm = 0.0;

  // Sensitivities:
  // CO: 4.47 nA / ppm
  // SO2: 39.23 nA / ppm
  // O3: ~ -60 nA +- 10 / ppm
  // NO2: ~ -30 nA +- 10 / ppm

  float sensitivity = 4.75;

 public:
  COSensor(uint8_t refPin, uint8_t subPin) {
    refPin = refPin;
    subPin = subPin;
  }

  double read() {
    currentMillis = millis();
    if (currentMillis - previousMillis >= waitTime) {
      refPinReadSum += analogRead(refPin);
      subPinReadSum += analogRead(subPin);
      previousMillis = currentMillis;
      i++;
    }
    if (i >= numPolls) {
      readDiff = refPinReadSum - subPinReadSum;
      diffAvg = (double)readDiff / (double)numPolls;

      voltage = (double)diffAvg / 204.6;  // map from 0-1023 to 0-5v range

      // map voltage to nA
      amps = (double)(voltage) / resValue;

      nA = amps * 1000000000.0;

      // map nA to PPM
      ppm = nA / sensitivity;

      refPinReadSum = 0;
      subPinReadSum = 0;
      i = 0;
    }

    return voltage;
    // return ppm;
  }
};