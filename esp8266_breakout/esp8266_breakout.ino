#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Stream.h>
#include <WiFiClientSecureBearSSL.h>

// Wifi credential information
const String kSsid = "Pwn135";
const String kPassword = "my.little.pwnies";

// Overall delay, Thingspeak limits to 15 seconds
const int kDelay = 20000;

// Max size of our JSON doc for writing information
const size_t kMaxJsonDoc = JSON_OBJECT_SIZE(4);
StaticJsonDocument<kMaxJsonDoc> doc;

// Init the ESP8266 Wifi Module
ESP8266WiFiMulti WiFiMulti;

// Thingspeak API information
const char* kThingSpeakThumbPrint = "F9 C2 65 6C F9 EF 7F 66 8B F7 35 FE 15 EA 82 9F 5F 55 54 3E";
const String kThingSpeakUri = "https://api.thingspeak.com/update?";
const String kThingSpeakApiKey = "api_key=XXXXXXXXXXXXXXX";
const String kThingSpeakField1 = "&field1=";
const String kThingSpeakField2 = "&field2=";

// Helper buffer for receiving our Serial data
char jsonBuff[0x100];

// the setup function runs once when you press reset or power the board
void setup() {
  // Serial receiving data from 32u4
  Serial.begin(115200);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(kSsid.c_str(), kPassword.c_str());
  
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
  }
  memset(jsonBuff, 0, sizeof(jsonBuff));
}

// Fetches a string response from a remote API endpoint URI
int getApiResponse(const String& uri, String& resp) {
  std::unique_ptr<BearSSL::WiFiClientSecure>secure_client(new BearSSL::WiFiClientSecure);
  
  secure_client->setFingerprint(kThingSpeakThumbPrint);
  secure_client->setTimeout(3000);

  HTTPClient https;
  auto ret = https.begin(*secure_client, uri);

  if (!ret) {
    return 1;
  }
  
  auto status = https.GET();
  if (status != HTTP_CODE_OK) {
    return status;
  }

  resp = https.getString();
  return status;
}

void blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(kDelay);
  digitalWrite(LED_BUILTIN, LOW);
  delay(kDelay);  
}

void loop() {
  // We blink every time we read the temperature for some form of feedback
  //blink();

  // If we've received a message, POST the data to ThingSpeak
  if (Serial.available() > 0) {
    Serial.readBytes(jsonBuff, 0x100);
    auto err = deserializeJson(doc, String(jsonBuff));
    if (err) {
      Serial.print("Failed to parse " + String(jsonBuff));
      Serial.println(" error: " + String(err.c_str()));
      return;
    }
    String uri = kThingSpeakUri + kThingSpeakApiKey + 
                 kThingSpeakField1 + doc["temp"].as<String>() + 
                 kThingSpeakField2 + doc["uptime"].as<String>();
    String resp {""};
    auto ret = getApiResponse(uri, resp);
    Serial.println("POST " + String(jsonBuff) + " got back (" + 
                   String(ret) + ", " + resp + ")");
  } else {
    Serial.println("No data yet, waiting...");
  }

  // Lastly clear out our buffer
  memset(jsonBuff, 0, sizeof(jsonBuff));
  delay(kDelay);
}
