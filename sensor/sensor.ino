#include <Arduino.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>
//#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
//#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <SD.h>
//#include <WiFiClientSecureBearSSL.h>


// Wifi credential information
const String kSsid = "Pwn135";
const String kPassword = "my.little.pwnies";

// Data wire is plugged into pin 5 on the Arduino 
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices  
OneWire oneWire(ONE_WIRE_BUS); 

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Pinout that we get SD card info from
const int kCardSelect = 4;

// Overall delay
const int kSensorDelay = 1000;

// Total filename must be < 8 chars, because FAT
const String kLogSuffix = "temp.log";

// Max size of our JSON doc for writing information
const size_t kMaxJsonDoc = JSON_OBJECT_SIZE(3);

// Init the ESP8266 Wifi Module
//ESP8266WiFiMulti WiFiMulti;

// URI for remote resource we're grabbing
const String kApiUri = "https://canhazip.com";
const char* kCanHazThumbprint = "39 11 70 BA 7D 5D D1 E0 3C 80 A8 5B B2 8F 28 57 67 A1 FC 4A";

// Thingspeak API information
const char* kThingSpeakThumbPrint = "F9 C2 65 6C F9 EF 7F 66 8B F7 35 FE 15 EA 82 9F 5F 55 54 3E";
const String kThingSpeakUri = "https://api.thingspeak.com/update?";
const String kThingSpeakApiKey = "api_key=2RJ5MY0HKIFM286P";
const String kThingSpeakField1 = "&field1=";
const String kThingSpeakField2 = "field2=";

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Initializing SDcard");
  if (!SD.begin(kCardSelect)) {
    Serial.println("Failed to init SDcard, check that format is FAT16/32");
  }

  sensors.begin();

  /*
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(kSsid.c_str(), kPassword.c_str());
  
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(" .");
  }


  Serial.println();
  Serial.print("Connected to ");
  Serial.print(kSsid);
  Serial.print(" with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Initialization done.");
  */
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
  
}

// Fetches a string response from a remote API endpoint URI
/*
int getApiResponse(const String& uri, String& resp) {
  Serial.println("Fetching resource from " + uri);
  std::unique_ptr<BearSSL::WiFiClientSecure>secure_client(new BearSSL::WiFiClientSecure);
  
  secure_client->setFingerprint(kCanHazThumbprint);
  secure_client->setTimeout(3000);

  HTTPClient https;
  Serial.println("Beginning connection to secure API endpoint");
  auto ret = https.begin(*secure_client, uri);

  if (!ret) {
    Serial.println("Failed to connect to URI with " + String(ret));
    return 1;
  }
  
  Serial.println("Sending HTTP GET request");
  auto status = https.GET();
  if (status != HTTP_CODE_OK) {
    Serial.println("HTTPS GET failed with code " + String(status));
    return status;
  }

  resp = https.getString();
  Serial.println("Successfully retrieved data from " + kApiUri);
  Serial.println(resp);

  return status;
}
*/

void loop() {
  
  StaticJsonDocument<kMaxJsonDoc> doc;
  
  // User a naive hour-based uptime prexif ro our log file
  // TODO: Use the ESP8266 to get the current time from an NTP server or something
  auto uptime = millis();
  auto hour = uptime / 3600000;
  auto fname = String(hour) + "-" + kLogSuffix;
  
  // We blink every time we read the temperature for some form of feedback
  digitalWrite(LED_BUILTIN, HIGH);
  delay(kSensorDelay);
  digitalWrite(LED_BUILTIN, LOW);
  delay(kSensorDelay);

  // Getting resource from a remote location
  /*
  String resp {""};
  auto ret = getApiResponse(kApiUri, resp);
  if (ret == 1) {
    Serial.println("Fetch remote resource failed. Continuing");
  } else {
    Serial.println("STUN IP: " + resp);
  }
  */

  // Send the command to get temperature readings
  sensors.requestTemperatures();
  auto temp = sensors.getTempFByIndex(0);
  doc["uptime"] = uptime;
  doc["temp"] = temp;

  String json {""};
  serializeJson(doc, json);

  //Serial.println("Sensor captured: " + json);

  log_data_to_disk(json, fname);

  delay(kSensorDelay);
}
