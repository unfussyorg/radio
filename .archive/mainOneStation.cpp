////////////////////////////////////
// Internet Radio w/ Analog Tuner //
////////////////////////////////////

// Ring Buffer Version
// V2 - Add pot

#include <Arduino.h>
#include <VS1053.h>
#include <WiFi.h>
#define VS1053_CS 5
#define VS1053_DCS 16
#define VS1053_DREQ 4
#define VOLUME 75
#define VS_BUFF_SIZE 32
#define RING_BUF_SIZE 32000
#define HTTPVER "HTTP/1.1" // use HTTP/1.0 to force not chunked transfer encoding

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
WiFiClient client;

// wifi
const char *ssid = "SSID";
const char *password = "PASSWORD";

// stream URL
const char *host = "kxlu.streamguys1.com";
const char *path = "/kxlu-hi";
int httpPort = 80;

// pot & stations
int radioStation = 0;
int previousradioStation = -1;
const int potPin = 34; // potentiometer is connected to GPIO 34 (Analog ADC1/6)
int potValue = 0;      // init variable to storing pot value
int previouspotValue = -1;

unsigned long logLoopCounter = 0;

uint16_t rcount = 0;
uint8_t *ringbuf;
uint16_t rbwindex = 0;
uint8_t *mp3buff;
uint16_t rbrindex = RING_BUF_SIZE - 1;
uint8_t netbuff[1024];

///////////////////////////
// Independent Functions //
///////////////////////////

void putring(uint8_t b)
{
  *(ringbuf + rbwindex) = b;
  if (++rbwindex == RING_BUF_SIZE)
  {
    rbwindex = 0;
  }
  rcount++;
}
uint8_t getring()
{
  if (++rbrindex == RING_BUF_SIZE)
  {
    rbrindex = 0;
  }
  rcount--;
  return *(ringbuf + rbrindex);
}

void playRing(uint8_t b)
{
  static int bufcnt = 0;
  mp3buff[bufcnt++] = b;
  if (bufcnt == sizeof(mp3buff))
  {
    player.playChunk(mp3buff, bufcnt);
    bufcnt = 0;
  }
}

// read potentiometer & set radioStation number //
void checkPot()
{
  potValue = analogRead(potPin);
  if (potValue < 2000)
  {
    radioStation = 1;
  }
  else
  {
    radioStation = 2;
  }
}

////////////////
// Void Setup //
////////////////

void setup()
{
  Serial.begin(115200);
  mp3buff = (uint8_t *)malloc(VS_BUFF_SIZE);
  ringbuf = (uint8_t *)malloc(RING_BUF_SIZE);
  SPI.begin();
  player.begin();
  player.switchToMp3Mode();
  player.setVolume(VOLUME);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  if (!client.connect(host, httpPort))
  {
    Serial.println("Connection failed");
    return;
  }
  // get station number
  checkPot();
  // start playing
  client.print(String("GET ") + path + " " + HTTPVER + "\r\nHost: " + host + "\r\nConnection: close\r\n\r\n");
  Serial.println("Start playing");
}

///////////////
// Void Loop //
///////////////

void loop()
{
  uint32_t maxfilechunk;
  unsigned long nowMills = millis();
  int resd = 0;
  int cntr = 0;

  checkPot();

  // if client isn't connected, reconnect to DEFAULT STATION -- need to send this through a process to choose the correct station via pot
  if (!client.connected())
  {
    Serial.println("Reconnecting...");
    if (client.connect(host, httpPort))
    {
      client.print(String("GET ") + path + " " + HTTPVER + "\r\nHost: " + host + "\r\nConnection: close\r\n\r\n");
    }
  }

  maxfilechunk = client.available();
  if (maxfilechunk > 0)
  {
    if (maxfilechunk > 1024)
    {
      maxfilechunk = 1024;
    }
    if ((RING_BUF_SIZE - rcount) > maxfilechunk)
    {
      resd = client.read(netbuff, maxfilechunk);
      while ((cntr < resd))
      {
        putring(netbuff[cntr++]);
      }
    }
    yield();
  }
  while (rcount && (player.data_request()))
  {
    playRing(getring());
  }
  if ((nowMills - logLoopCounter) >= 500)
  {
    Serial.printf("Buffer: %d%%\r", rcount * 100 / RING_BUF_SIZE);
    logLoopCounter = nowMills;
    // GET POT VALUES TO SHOW UP IN SERIAL ///////////////////////
    if (potValue < (previouspotValue - 100) || potValue > (previouspotValue + 100))
    {
      Serial.print("                New pot: ");
      Serial.print(potValue);
      previouspotValue = potValue;
    }
    // GET STATION IN SERIAL ///////////////////////
    if (radioStation != previousradioStation)
    {
      Serial.print("                New station: ");
      Serial.print(radioStation);
      previousradioStation = radioStation;
    }
  }
}
