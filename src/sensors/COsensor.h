#include <Arduino.h>

class COSensor {
 private:
  const int resValue = 9870;
  const float Vref = 1.1;
  const float Sf = 2.11;

  int i = 0;
  const int numPolls = 256;
  bool updateAvg = false;

  long int sensorReadSum = 0;
  long int refReadSum = 0;
  float sensorValAvg = 0;
  float refValAvg = 0;
  float amountCO = 0;
  float current = 0;
  float sensorSensitivity = 0;
  float PPM = 0;

  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;
  unsigned waitTime = 3;

 public:
  int analogInPin;
  int refInPin;

  COSensor(uint8_t analogInPin, uint8_t refInPin) {
    analogInPin = analogInPin;
    refInPin = refInPin;
  }

  float read() {
    currentMillis = millis();
    if (currentMillis - previousMillis >= waitTime) {
      sensorReadSum += analogRead(analogInPin);
      refReadSum += analogRead(refInPin);
      previousMillis = currentMillis;
      i++;
    }
    if (i >= numPolls) {
      sensorReadSum -= refReadSum;
      sensorValAvg = (float)sensorReadSum / numPolls;
      amountCO = sensorValAvg / 1024;
      current = Vref / resValue;
      sensorSensitivity = Sf / 1000000000;

      PPM = amountCO * current / sensorSensitivity;

      // print results from tutorial calculations:
      Serial.print("\n\nPPM = ");
      Serial.print(((float)sensorReadSum / numPolls / 1024 * Vref / resValue *
                    1000000000) /
                   Sf);
      Serial.print("\tnA = ");
      Serial.print((float)(sensorReadSum) / numPolls / 1024 * Vref / resValue *
                   1000000000);
      Serial.print("\tCounts = ");
      Serial.println(sensorReadSum);
      // Serial.print("\tAnalog = ");
      // Serial.println(analogRead(analogInPin));

      // print results from our calculations:
      Serial.print("PPM = ");
      Serial.print(PPM);
      Serial.print("\tnA = ");
      Serial.print(current * amountCO * 1000000000);
      Serial.print("\tCounts = ");
      Serial.println(sensorReadSum);
      // Serial.print("\tAnalog = ");
      // Serial.println(analogRead(analogInPin));

      sensorReadSum = 0;
      refReadSum = 0;
      i = 0;
    }

    return PPM;
  }
};