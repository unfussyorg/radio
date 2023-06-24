////////////////////////////////////
// Internet Radio w/ Analog Tuner //
////////////////////////////////////

// Ring Buffer Version
// V2 - Add pot check into loop
// V2.1 - Add station check into loop
// V2.2 - Change station with map instead of raw pot values, remove setup connect

#include <Arduino.h>
#include <VS1053.h>
#include <WiFi.h>
#include <config.h>
#include <time.h>
#define VS1053_CS 5
#define VS1053_DCS 16
#define VS1053_DREQ 4
#define VOLUME 100
#define VS_BUFF_SIZE 32
#define RING_BUF_SIZE 32000

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
WiFiClient client;

// get time for packets
const long gmtOffset_sec = -28800;
const int daylightOffset_sec = 3600;
const char *ntpServer = "pool.ntp.org";

// wifi credentials defined in Config.h
const char *ssid = SSID;
const char *password = SSID_PASS;
const uint8_t bssid[6] = {ACCESS_POINT}; // this can be removed if you aren't targeting a specific access point/router

// 1 - 88FM KXLU
char *host1 = "kxlu.streamguys1.com";
char *path1 = "/kxlu-hi";
int httpPort1 = 80;
char *httpver1 = "HTTP/1.1"; // 1.1 is default, but some streams work better in 1.0 (not chunked)
// 2 — Deadzone
// 3 - 90FM KEXP
char *host3 = "kexp-mp3-128.streamguys1.com";
char *path3 = "/kexp128.mp3";
int httpPort3 = 80;
char *httpver3 = "HTTP/1.1";
// 4 — Deadzone
// 5 - 92FM KUSF
char *host5 = "104.236.145.45";
char *path5 = "/stream";
int httpPort5 = 8000;
char *httpver5 = "HTTP/1.1";
// 6 — Deadzone
// 7 - 94 "95B" Auckland
char *host7 = "streams.95bfm.com";
char *path7 = "/stream128";
int httpPort7 = 80;
char *httpver7 = "HTTP/1.1";
// 8 — Deadzone
// 9 - 96 WNYU
char *host9 = "cinema.acs.its.nyu.edu";
char *path9 = "/wnyu128.mp3";
int httpPort9 = 8000;
char *httpver9 = "HTTP/1.1";
// 10 — Deadzone
// 11 - 98 KFJC Los Altos Hills
char *host11 = "netcast.kfjc.org";
char *path11 = "/kfjc-320k-aac";
int httpPort11 = 80;
char *httpver11 = "HTTP/1.0"; // 1.1 caused stuttering
// 12 - Deadzone
// 13 - 100 KSCU Santa Clara
char *host13 = "kscu.streamguys1.com";
char *path13 = "/live?kscu-site=";
int httpPort13 = 80;
char *httpver13 = "HTTP/1.1";
// 14 - Deadzone
// 15 - 102 Radio K Minneapolis
char *host15 = "radiokstreams.cce.umn.edu";
char *path15 = "/";
int httpPort15 = 8128;
char *httpver15 = "HTTP/1.1";
// 16 - Deadzone
// 17 - 104 Corridos Y No Fantasias
char *host17 = "stream-150.zeno.fm";
char *path17 = "/7xv04r3wzp8uv?zs=IJNAlzc7Sa-y6rlnOKJQ2Q";
int httpPort17 = 80;
char *httpver17 = "HTTP/1.1";
// 18 - Deadzone
// 19 - 106 WVFS Tallahassee
char *host19 = "voice.wvfs.fsu.edu";
char *path19 = "/stream";
int httpPort19 = 8000;
char *httpver19 = "HTTP/1.1";
// 20 - Deadzone
// 21 - 108 Psyched! SF
char *host21 = "us3.internet-radio.com";
char *path21 = "/proxy/psychedradiosf/stream";
int httpPort21 = 80;
char *httpver21 = "HTTP/1.0"; // 1.1 caused stuttering
// 22 - Deadzone
// 23 - 110 KZSC Santa Cruz
char *host23 = "kzscfms1-geckohost.radioca.st";
char *path23 = "/kzschigh";
int httpPort23 = 80;
char *httpver23 = "HTTP/1.0"; // 1.1 caused stuttering

// pot & station mapping
const int potPin = 34;    // potentiometer is connected to GPIO 34 (Analog ADC1/6)
int potValue;             // reading from the pot (0 - 4095)
int tallyCount = 0;       // counts 10 pot readings before we take an average
int mappedTally = 0;      // a container for the 10 readings pre-average
int activeStation;        // can be any odd number from 1-11. even nums are deadzone
int previousStation = -9; // impossible value makes sure it runs through connection flow first time

// haptic
// const int hapticPin = 21; // vibration motor board is controlled by GPIO 21

unsigned long logLoopCounter = 0;

uint16_t rcount = 0;
uint8_t *ringbuf;
uint16_t rbwindex = 0;
uint8_t *mp3buff;
uint16_t rbrindex = RING_BUF_SIZE - 1;
uint8_t netbuff[1024];

///////////////////////////
// Ring Buffer Functions //
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

////////////////
// Void Setup //
////////////////

void setup()
{
  // Serial.begin(115200);
  // pinMode(hapticPin, OUTPUT); // define pin as output
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // get time and hopefully fix packet loss
  mp3buff = (uint8_t *)malloc(VS_BUFF_SIZE);
  ringbuf = (uint8_t *)malloc(RING_BUF_SIZE);
  SPI.begin();
  player.begin();
  player.switchToMp3Mode();
  player.setVolume(VOLUME);
  WiFi.begin(ssid, password, 0, bssid);
  /*while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }*/
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

  potValue = analogRead(potPin);
  mappedTally = mappedTally + map(potValue, 656, 3480, 1, 23);
  tallyCount++;
  if (tallyCount == 10) // after 10 loops, recalculate activeStation
  {
    activeStation = mappedTally / 10;
    tallyCount = 0;
    mappedTally = 0;
  }

  if ((!client.connected() || (activeStation != previousStation)) && ((activeStation % 2) != 0)) // if (disconnected OR new station) AND (odd station)
  {
    if (activeStation == 1) // 1
    {
      // Serial.println("Loop connect 1");
      if (client.connect(host1, httpPort1))
      {
        client.print(String("GET ") + path1 + " " + httpver1 + "\r\nHost: " + host1 + "\r\nConnection: close\r\n\r\n");
        // digitalWrite(hapticPin, HIGH); // vibrate
        // delay(200);
        // digitalWrite(hapticPin, LOW); // stop
      }
    }
    else if (activeStation == 3) // 3
    {
      // Serial.println("Loop connect 3");
      if (client.connect(host3, httpPort3))
      {
        client.print(String("GET ") + path3 + " " + httpver3 + "\r\nHost: " + host3 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 5) // 5
    {
      // Serial.println("Loop connect 5");
      if (client.connect(host5, httpPort5))
      {
        client.print(String("GET ") + path5 + " " + httpver5 + "\r\nHost: " + host5 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 7) // 7
    {
      // Serial.println("Loop connect 7");
      if (client.connect(host7, httpPort7))
      {
        client.print(String("GET ") + path7 + " " + httpver7 + "\r\nHost: " + host7 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 9) // 9
    {
      // Serial.println("Loop connect 9");
      if (client.connect(host9, httpPort9))
      {
        client.print(String("GET ") + path9 + " " + httpver9 + "\r\nHost: " + host9 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 11) // 11
    {
      // Serial.println("Loop connect 11");
      if (client.connect(host11, httpPort11))
      {
        client.print(String("GET ") + path11 + " " + httpver11 + "\r\nHost: " + host11 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 13) // 13
    {
      // Serial.println("Loop connect 13");
      if (client.connect(host13, httpPort13))
      {
        client.print(String("GET ") + path13 + " " + httpver13 + "\r\nHost: " + host13 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 15) // 15
    {
      // Serial.println("Loop connect 15");
      if (client.connect(host15, httpPort15))
      {
        client.print(String("GET ") + path15 + " " + httpver15 + "\r\nHost: " + host15 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 17) // 17
    {
      // Serial.println("Loop connect 17");
      if (client.connect(host17, httpPort17))
      {
        client.print(String("GET ") + path17 + " " + httpver17 + "\r\nHost: " + host17 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 19) // 19
    {
      // Serial.println("Loop connect 19");
      if (client.connect(host19, httpPort19))
      {
        client.print(String("GET ") + path19 + " " + httpver19 + "\r\nHost: " + host19 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 21) // 21
    {
      // Serial.println("Loop connect 21");
      if (client.connect(host21, httpPort21))
      {
        client.print(String("GET ") + path21 + " " + httpver21 + "\r\nHost: " + host21 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else if (activeStation == 23) // 23
    {
      // Serial.println("Loop connect 23");
      if (client.connect(host23, httpPort23))
      {
        client.print(String("GET ") + path23 + " " + httpver23 + "\r\nHost: " + host23 + "\r\nConnection: close\r\n\r\n");
      }
    }
    else
    {
      // nothing
    }
    previousStation = activeStation;
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
    // Serial.printf("Buffer: %d%%\r", rcount * 100 / RING_BUF_SIZE);
    logLoopCounter = nowMills;
  }
}
