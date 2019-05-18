#include <Arduino.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SD.h>

// Data wire is plugged into pin 5 on the Arduino 
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices  
OneWire oneWire(ONE_WIRE_BUS); 

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Pinout that we get SD card info from
const int kCardSelect = 4;

// Overall delay, Thingspeak limits to 15 seconds
const int kDelay = 20000;

// Total filename must be < 8 chars, because FAT
const String kLogSuffix = "temp.log";

// Max size of our JSON doc for writing information
const size_t kMaxJsonDoc = JSON_OBJECT_SIZE(3);
StaticJsonDocument<kMaxJsonDoc> doc;

// the setup function runs once when you press reset or power the board
void setup() {
  // Serial for debug logging
  Serial.begin(115200);
  // Serial for communicating with esp
  Serial1.begin(115200);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Initializing SDcard");
  if (!SD.begin(kCardSelect)) {
    Serial.println("Failed to init SDcard, check that format is FAT16/32");
  }

  sensors.begin();
  Serial.println("Initialization complete");  
}

// Helper function to write a log line to disk
void log_data_to_disk(const String& msg, const String& fname) {
  File dataFile = SD.open(fname.c_str(), FILE_WRITE);
  if (dataFile) {
    dataFile.println(msg);
  } else {
    Serial.println("Failed to open file " + fname + " for writing");
  }
  dataFile.close();
}

// Helper function to POST a log line to our SIEM
void log_data_to_siem(const String& msg) {
  Serial.println("Sending " + msg + " to esp");
  // Send our payload to the ESP8266
  Serial1.print(msg);
}

void blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(kDelay);
  digitalWrite(LED_BUILTIN, LOW);
  delay(kDelay);  
}

void loop() {
  // User a naive hour-based uptime prexif ro our log file
  // TODO: Use the ESP8266 to get the current time from an NTP server or something
  auto uptime = millis();
  auto hour = uptime / 3600000;
  auto fname = String(hour) + "-" + kLogSuffix;
  
  // We blink every time we read the temperature for some form of feedback
  //blink();

  // Send the command to get temperature readings
  sensors.requestTemperatures();
  auto temp = sensors.getTempFByIndex(0);
  doc["uptime"] = uptime;
  doc["temp"] = temp;

  String json {""};
  serializeJson(doc, json);

  Serial.println("Sensor captured: " + json);

  log_data_to_disk(json, fname);

  log_data_to_siem(json);

  delay(kDelay);
}
