/* Sensor read code - Team 17
    Authors: Jordan Alexander, Ben DeWeerd
*/

const int analogInPin = A0;  // Analog input pin that the sensor is attached to
const int resValue = 9870;  // Value of 10kOhm resistor !change this to match your resistor
const float Vref = 1.1;  //This is the voltage of the internal reference
const long int cOff = 0; //286mV offset due to resistor ladder. Try taking the average of a long
//measurement of the counts without a sensor in place. This should give a good Zero.

const float Sf = 2.11; // sensitivity in nA/ppm,
//this is roughly about 1/2 the sensitivity of the barcode on the sensor. Try finding a known
//concentration of CO to measure, or just approximate.

const int extraBit = 256; //Number of samples averaged, like adding 8 bits to ADC

long int sensorValueCumulative = 0;        // value read from the sensor
float sensorValueAverage = 0;
float amountCO = 0;
float current = 0;
float sensorSensitivity = 0;
float PPM = 0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  // !!!set analog reference to 1.1 Volts!!!
  analogReference(EXTERNAL);
}

void loop() {
  // read the analog in value:
  sensorValueCumulative = 0;
  for (int i = 0; i < extraBit; i++) {
    sensorValueCumulative = analogRead(analogInPin) + sensorValueCumulative;
    delay(3);   // needs 2 ms for the analog-to-digital converter to settle after the last reading
  }

  sensorValueAverage = sensorValueCumulative / extraBit - cOff;
  amountCO = sensorValueAverage / 1024;
  current = Vref / resValue;
  sensorSensitivity = Sf * 1000000000;

  PPM = amountCO * current / sensorSensitivity;

  // print the results to the serial monitor:

  Serial.print("\nAvg Sensor Value = ");
  Serial.print( sensorValueAverage );
  Serial.print("\tamountCO = ");
  Serial.print( amountCO );
  Serial.print("\t\tSensitivity Factor = ");
  Serial.print( sensorSensitivity );
  Serial.print("\tCurrent = ");
  Serial.print( current );
  Serial.print("\tPPM = ");
  Serial.print( PPM );
  // Serial.print("\tnA = ");
  // Serial.print( (float) (sensorValue) / extraBit / 1024 * Vref / resValue * 1000000000);
  // Serial.print("\tCounts = " );
  // Serial.println(sensorValue);
  
  //Trying to make each loop 1 second
  delay(218);  //1 second – 3ms*256ms (each adc read)-14ms (for printing)= 218ms
}