//////////////////////////////////////////////
// Read Pot Value for Physical Dial Mapping //
//////////////////////////////////////////////

// rawPotValues

#include <Arduino.h>

const int potPin = 34;
int potValue;

int sample = 0;
int sampleValue = 0;
int activeStation;
int avgPot;

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  potValue = analogRead(potPin);
  sampleValue = sampleValue + potValue;
  sample++;
  if (sample == 10)
  {
    avgPot = sampleValue / 10;
    Serial.println(avgPot);
    sample = 0;
    sampleValue = 0;
    delay(500);
  }
}