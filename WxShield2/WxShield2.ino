/*
The MIT License (MIT)

Copyright (c) 2016 Nick Potts

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Wire.h>
#include "helpers.h"
#include "w1temp.h"
#include "SparkFunMPL3115A2.h" //Pressure sensor - Search "SparkFun MPL3115" and install from Library Manager
#include "SparkFunHTU21D.h" //Humidity sensor - Search "SparkFun HTU21D" and install from Library Manager

MPL3115A2 barometer;
HTU21D rh;
/*I have a DS18B20 (family 0x28) connected to pin 10 in a full 3 pin (vcc, 1wire, gnd) topology
 * waiting 800ms before retrieving data. (lowest resolution) */
W1temp thermometer1(10, 0x28, 0, 800);
W1temp thermometer2(10, 0x28, 0, 800);

//assigned pins
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;

void setup() {
  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);
  pinMode(BATT, INPUT);
  Serial.begin(57600);
  Serial.println();
  w1();
//  i2c();
}

void w1() {
  Serial.print("Waiting for 1w devices...");
  while(!thermometer1.locate()) {}
  while(!thermometer2.locate(1)) {}
}

void i2c() {
  /*Begin RH*/
  rh.begin();
  /*Setup Barometer*/
  barometer.begin(); // Get sensor online
  barometer.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  barometer.setOversampleRate(7); // Set Oversample to the recommended 128
  barometer.enableEventFlags();
}


struct measurement temp;
void loop() {
  thermometer1.measure(); //prep read
  thermometer2.measure(); //prep read
  delay(1000);
  json();
}
