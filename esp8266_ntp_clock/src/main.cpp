
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <time.h>

// Wifi credential information
const char* kSsid = "XXXXXXXXX";
const char* kPassword = "XXXXXXXXXX";

// Update the time every 5 seconds
const int kDelay = 5000;

// Constants used to adjust timestamp to PST from UTC
const int kUtcToPstOffset = -28800;
const int kDstSecOffset = 0;

// Global reference to the display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// Global reference to the current time
String kCurrentTime = "";

// Interval for re-syncing against NTP servers
const int kClockRefresh = 5;

// Boolean to toggle NTP syncs;
int kNextSync = 0;

// Helper function to print to serial with the current timestamp
void logToSerial(const String& msg) {

  char time_buff[32] {'\0'};
  const char* time_fmt = "[%Y-%m-%d %H:%M:%S] ";
  
  struct tm * ti;
  time_t ts;
  time(&ts);
  ti = localtime(&ts);
  strftime(time_buff, 32, time_fmt, ti);

  Serial.println(String(time_buff) + msg);
}

// Helper function to sync our internal clock
void syncClock() {
  // DO NOT COMMIT: Change NTP to Googles before making public
  configTime(kUtcToPstOffset, kDstSecOffset, "time.google.com", "time.nist.gov");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  logToSerial("Time synced from NTP: " + String(asctime(&timeinfo)));
}

void getTime() {
  char time_buff[4] {'\0'};
  const char* time_fmt = "%H%M";
  
  struct tm * ti;
  time_t ts;
  time(&ts);
  ti = localtime(&ts);
  strftime(time_buff, 32, time_fmt, ti);

  kCurrentTime = String(time_buff);
}

// Setup our esp8266 breakout
void setup() {
  Serial.begin(9600);

  logToSerial("Firing up the ESP8266 Breakout Clock!");

  logToSerial("Connecting to WiFi");
  
  WiFi.begin(kSsid, kPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1500);
    Serial.print(". ");
  }

  syncClock();
  
  // Address of the HT16K33 (The Display chip)
  logToSerial("Initializing HT16K33");
  alpha4.begin(0x70);
}

void loop() {

  // Update the current time
  getTime();

  // Grab the minutes and display to the console the timestamp
  auto minutes = atoi(&kCurrentTime[2]);
  logToSerial("Time is " + kCurrentTime);

  // Periodically update our clock, do this once every 5 minutes
  if ((minutes % kClockRefresh) == kNextSync) {
    logToSerial(String(kClockRefresh) + " minutes passed, resyncing clock");
    syncClock();
    kNextSync = (kNextSync + 5) % 60;
  }

  // Send the text to be displayed over I2C to the matrix
  for (uint8_t i = 0; i < 4; i++) {
    alpha4.writeDigitAscii(i, kCurrentTime[i]);
  }
  alpha4.writeDisplay();
  delay(kDelay);
}
