// constant definitions

const int analogInPin = A0;  // Analog input pin that the sensor is attached to
const int refInPin = A1;
const int resValue =
    9870;  // Value of 10kOhm resistor !change this to match your resistor
const float Vref = 1.1;  // This is the voltage of the internal reference

const float Sf = 2.11;  // sensitivity in nA/ppm,
                        // this is roughly about 1/2 the sensitivity of the
                        // barcode on the sensor. Try finding a known
                        // concentration of CO to measure, or just approximate.

const int extraBit =
    256;  // Number of samples averaged, like adding 8 bits to ADC

long int sensorValueCumulative = 0;  // value read from the sensor
long int refValueCumulative = 0;     // value read from reference
float sensorValueAverage = 0;
float refValueAverage = 0;
float amountCO = 0;
float current = 0;
float sensorSensitivity = 0;
float PPM = 0;

void setup() {
  Serial.begin(9600);
  analogReference(INTERNAL);  // set analog reference to 1.1 Volts
}

void loop() {
  // read the analog in value:
  sensorValueCumulative = 0;
  refValueCumulative = 0;
  for (int i = 0; i < extraBit; i++) {
    sensorValueCumulative = analogRead(analogInPin) + sensorValueCumulative;
    refValueCumulative = analogRead(refInPin) + refValueCumulative;
    delay(3);  // needs 2 ms for the analog-to-digital converter to settle after
               // the last reading
  }

  sensorValueCumulative =
      sensorValueCumulative - refValueCumulative;  // subtract reference offset

  sensorValueAverage = (float)sensorValueCumulative / extraBit;
  amountCO = sensorValueAverage / 1024;
  current = Vref / resValue;
  sensorSensitivity = Sf / 1000000000;

  PPM = amountCO * current / sensorSensitivity;

  // print results from tutorial calculations:
  Serial.print("\n\nPPM = ");
  Serial.print(((float)sensorValueCumulative / extraBit / 1024 * Vref /
                resValue * 1000000000) /
               Sf);
  Serial.print("\tnA = ");
  Serial.print((float)(sensorValueCumulative) / extraBit / 1024 * Vref /
               resValue * 1000000000);
  Serial.print("\tCounts = ");
  Serial.print(sensorValueCumulative);
  Serial.print("\tAnalog = ");
  Serial.println(analogRead(analogInPin));

  // print results from our calculations:
  Serial.print("PPM = ");
  Serial.print(PPM);
  Serial.print("\tnA = ");
  Serial.print(current * amountCO * 1000000000);
  Serial.print("\tcumulative = ");
  Serial.print(sensorValueCumulative);
}