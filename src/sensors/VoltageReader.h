#include <Arduino.h>

class VoltageReader {
 private:
  int pin1;
  int pin2;

  int i = 0;
  const int numPolls = 64;
  bool updateAvg = false;

  long int pin1ReadSum = 0;
  long int pin2ReadSum = 0;
  long int readDiff = 0;
  float diffAvg = 0;
  float voltage = 0;

  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;
  unsigned waitTime = 3;

 public:
  VoltageReader(uint8_t pin1, uint8_t pin2) {
    pin1 = pin1;
    pin2 = pin2;
  }

  float read() {
    currentMillis = millis();
    if (currentMillis - previousMillis >= waitTime) {
      pin1ReadSum += analogRead(pin1);
      pin2ReadSum += analogRead(pin2);
      previousMillis = currentMillis;
      i++;
    }
    if (i >= numPolls) {
      readDiff = pin2ReadSum - pin1ReadSum;
      diffAvg = (float)readDiff / (float)numPolls;

      voltage = (float)diffAvg / 204.6;  // map to 0-5v range

      pin1ReadSum = 0;
      pin2ReadSum = 0;
      i = 0;
    }

    return voltage;

    // int v1 = analogRead(pin1);
    // int v2 = analogRead(pin2);

    // int diff = v2 - v1;            // range 0:1023
    // float voltage = diff / 204.6;  // map to 0-5v range

    // return voltage;
  }
};