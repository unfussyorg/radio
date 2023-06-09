////////////////
// DIAL SETUP //
////////////////

// dialSetup

#include <Arduino.h>

// pot & station mapping
const int potPin = 34; // potentiometer is connected to GPIO 34 (Analog ADC1/6)
int potValue;          // reading from the pot (0 - 4095)
int tallyCount = 0;    // counts 10 pot readings before we take an average
int mappedTally = 0;   // a container for the 10 readings pre-average
int activeStation;     // can be any odd number from 1-11. even nums are deadzone

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  potValue = analogRead(potPin);
  mappedTally = mappedTally + map(potValue, 656, 3480, 1, 23);
  tallyCount++;
  if (tallyCount == 10) // after 10 loops, recalculate activeStation
  {
    activeStation = mappedTally / 10;
    tallyCount = 0;
    mappedTally = 0;
  }

  if (activeStation == 1)
  {
    Serial.println("88");
  }
  else if (activeStation == 2)
  {
    Serial.println("89 dead");
  }
  else if (activeStation == 3)
  {
    Serial.println("90");
  }
  else if (activeStation == 4)
  {
    Serial.println("91 dead");
  }
  else if (activeStation == 5)
  {
    Serial.println("92");
  }
  else if (activeStation == 6)
  {
    Serial.println("93 dead");
  }
  else if (activeStation == 7)
  {
    Serial.println("94");
  }
  else if (activeStation == 8)
  {
    Serial.println("95 dead");
  }
  else if (activeStation == 9)
  {
    Serial.println("96");
  }
  else if (activeStation == 10)
  {
    Serial.println("97 dead");
  }
  else if (activeStation == 11)
  {
    Serial.println("98");
  }
  else if (activeStation == 12)
  {
    Serial.println("99 dead");
  }
  else if (activeStation == 13)
  {
    Serial.println("100");
  }
  else if (activeStation == 14)
  {
    Serial.println("101 dead");
  }
  else if (activeStation == 15)
  {
    Serial.println("102");
  }
  else if (activeStation == 16)
  {
    Serial.println("103 dead");
  }
  else if (activeStation == 17)
  {
    Serial.println("104");
  }
  else if (activeStation == 18)
  {
    Serial.println("105 dead");
  }
  else if (activeStation == 19)
  {
    Serial.println("106");
  }
  else if (activeStation == 20)
  {
    Serial.println("107 dead");
  }
  else if (activeStation == 21)
  {
    Serial.println("108");
  }
  else if (activeStation == 22)
  {
    Serial.println("109 dead");
  }
  else if (activeStation == 23)
  {
    Serial.println("110");
  }
  else
  {
    Serial.println("out of range");
  }
}