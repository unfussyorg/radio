////////////////
// Tuner Test //
////////////////

// V1 - Using map() to map pot range to radio dial
// V1.2 - Added deadzones on even stations (2, 4, 6...)

#include <Arduino.h>

// pot & stations
const int potPin = 34; // potentiometer is connected to GPIO 34 (Analog ADC1/6)
int potValue;          // reading from the pot (0 - 4095)

int tallyCount = 0;
int mappedTally = 0;
int activeStation; // what is playing
int previousStation = -9;

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  potValue = analogRead(potPin);
  mappedTally = mappedTally + map(potValue, 1000, 3000, 1, 11);
  tallyCount++;

  if (tallyCount == 100) // if we've taken 100 readings
  {
    activeStation = mappedTally / 100;
    tallyCount = 0;
    mappedTally = 0;

    if ((activeStation != previousStation) && ((activeStation % 2) != 0)) // if the mapped station has changed AND it's an odd station
    {
      if (activeStation == 1) // 1
      {
        Serial.println("Connected to 1");
      }

      else if (activeStation == 3) // 3
      {
        Serial.println("Connected to 3");
      }

      else if (activeStation == 5) // 5
      {
        Serial.println("Connected to 5");
      }

      else if (activeStation == 7) // 7
      {
        Serial.println("Connected to 7");
      }

      else if (activeStation == 9) // 9
      {
        Serial.println("Connected to 9");
      }

      else if (activeStation == 11) // 11
      {
        Serial.println("Connected to 11");
      }
      else
      {
        // nothing
      }
      previousStation = activeStation;
    }
  }
}
