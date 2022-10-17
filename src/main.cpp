#include <Arduino.h>
#include <string.h>
using namespace std;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  int sensorValue = analogRead(A0);
  Serial.println(sensorValue);

  // double voltage = sensorValue * (1024/5);
  // Serial.println("Voltage: " + (voltage);
  delay(500);
}