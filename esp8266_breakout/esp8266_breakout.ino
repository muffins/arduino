#include <Arduino.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <Stream.h>

#include <WiFiClientSecureBearSSL.h>
// I2C lib
#include <Wire.h>

// Wifi credential information
const String kSsid = "Pwn135";
const String kPassword = "my.little.pwnies";

// Overall delay
const int kDelay = 1000;

// Max size of our JSON doc for writing information
const size_t kMaxJsonDoc = JSON_OBJECT_SIZE(4);

// Init the ESP8266 Wifi Module
ESP8266WiFiMulti WiFiMulti;

// URI for remote resource we're grabbing
const String kApiUri = "https://canhazip.com";
const String kApiHost = "https://canhazip.com";
const String kApiPort = "80";
const char* kCanHazThumbprint = "39 11 70 BA 7D 5D D1 E0 3C 80 A8 5B B2 8F 28 57 67 A1 FC 4A";

// Thingspeak API information
const char* kThingSpeakThumbPrint = "F9 C2 65 6C F9 EF 7F 66 8B F7 35 FE 15 EA 82 9F 5F 55 54 3E";
const String kThingSpeakUri = "https://api.thingspeak.com/update?";
const String kThingSpeakApiKey = "api_key=2RJ5MY0HKIFM286P";
const String kThingSpeakField1 = "&field1=";
const String kThingSpeakField2 = "field2=";

// Global values used to receive data from i2c
#define SDA_PIN 4
#define SCL_PIN 5

const int16_t I2C_MASTER = 0x42;
const int16_t I2C_SLAVE = 0x08;

char jsonBuff[0x100];
boolean receiveFlag { false };


// the setup function runs once when you press reset or power the board
void setup() {
  // Configure I2C
  Wire.begin(SDA_PIN, SCL_PIN, I2C_SLAVE);

  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);

  // Serial for debug logging
  Serial.begin(115200);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(kSsid.c_str(), kPassword.c_str());
  
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    //Serial.print(" .");
  }

  //Serial.println();
  //Serial.print("Connected to ");
  //Serial.print(kSsid);
  //Serial.print(" with IP address: ");
  //Serial.println(WiFi.localIP());
  memset(jsonBuff, 0, sizeof(jsonBuff));
}

// Our handler for receiving data via i2c
void receiveEvent(size_t numBytes) {
  int read = Wire.readBytes(jsonBuff, numBytes);
  //Serial.println("Read bytes: " + String(read));
  receiveFlag = true;
}

// Fetches a string response from a remote API endpoint URI
int getApiResponse(const String& uri, String& resp) {
  //Serial.println("Fetching resource from " + uri);
  std::unique_ptr<BearSSL::WiFiClientSecure>secure_client(new BearSSL::WiFiClientSecure);
  
  secure_client->setFingerprint(kCanHazThumbprint);
  secure_client->setTimeout(3000);

  HTTPClient https;
  //Serial.println("Beginning connection to secure API endpoint");
  auto ret = https.begin(*secure_client, uri);

  if (!ret) {
    //Serial.println("Failed to connect to URI with " + String(ret));
    return 1;
  }
  
  //Serial.println("Sending HTTP GET request");
  auto status = https.GET();
  if (status != HTTP_CODE_OK) {
    //Serial.println("HTTPS GET failed with code " + String(status));
    return status;
  }

  resp = https.getString();
  //Serial.println("Successfully retrieved data from " + kApiUri);
  //Serial.println(resp);

  return status;
}

void loop() {
  StaticJsonDocument<kMaxJsonDoc> doc;
  
  // We blink every time we read the temperature for some form of feedback
  digitalWrite(LED_BUILTIN, HIGH);
  delay(kDelay);
  digitalWrite(LED_BUILTIN, LOW);
  delay(kDelay);

  // If we've received a message, make an HTTPS request
  //Serial.println("Checking if data has been received...");
  Serial.readBytes(jsonBuff, 0x100);
  //if (receiveFlag) {
    //Serial.println("[+] Received buffer from 32u4: " + String(buff));

    //String resp {""};
    //auto ret = getApiResponse(kApiUri, resp);
    //if (ret == 1) {
      //Serial.println("Fetch remote resource failed. Continuing");
    //} else {
      //Serial.println("STUN IP: " + resp);
  Serial.print("Received data");

  memset(jsonBuff, 0, sizeof(jsonBuff));
      
    //}
  //receiveFlag = false;
  //}
  delay(kDelay);
}
