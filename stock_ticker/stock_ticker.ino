#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <HttpClient.h>

// Wifi credential information
const String kSsid = "Pwn135";
const String kPassword = "my.little.pwnies";

// Yahoo Finance API information
const char* kYahooApi = "https://apidojo-yahoo-finance-v1.p.rapidapi.com/market/get-summary?region=US&lang=en";
const char* kApiHost = "apidojo-yahoo-finance-v1.p.rapidapi.com";
const char* kApiKey = "2822998c09msh41e18f03cdf82dep115067jsndc7284e628ad";

// Max document size that we can parse for JSON
const size_t kMaxJsonDoc = 0x4096;

// Main loop pause
const size_t kMainDelay = 10000;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* kSha1Fingerprint = "01 3E 44 23 62 2D 92 54 6D 75 7B 2C 60 A7 33 A2 FB 60 C1 3E";

// TODO: A logo?
static const unsigned char kOsqueryLogo [] = {
  0x7F, 0x00, 0x80, 0x00, 0x3F, 0x80, 0xC0, 0x01, 0x1F, 0xC0, 0xE0, 0x03, 0x0F, 0xE0, 0xF0, 0x07,
  0x07, 0xF0, 0xF8, 0x0F, 0x03, 0xF8, 0xFC, 0x1F, 0x01, 0xFC, 0xFE, 0x3F, 0x00, 0xFE, 0xFF, 0x7F,
  0x01, 0xFF, 0xFF, 0xFE, 0x03, 0xFC, 0x7F, 0xFC, 0x07, 0xF8, 0x3F, 0xF8, 0x0F, 0xF0, 0x1F, 0xF0,
  0x1F, 0xE0, 0x0F, 0xE0, 0x3F, 0xC0, 0x07, 0xC0, 0x7F, 0x80, 0x03, 0x80, 0xFF, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x01, 0xFF, 0x01, 0xC0, 0x03, 0xFE, 0x03, 0xE0, 0x07, 0xFC, 0x07, 0xF0, 0x0F, 0xF8,
  0x0F, 0xF8, 0x1F, 0xF0, 0x1F, 0xFC, 0x3F, 0xE0, 0x3F, 0xFE, 0x7F, 0xC0, 0x7F, 0xFF, 0xFF, 0x80,
  0xFE, 0xFF, 0x7F, 0x00, 0xFC, 0x7F, 0x3F, 0x80, 0xF8, 0x3F, 0x1F, 0xC0, 0xF0, 0x1F, 0x0F, 0xE0,
  0xE0, 0x0F, 0x07, 0xF0, 0xC0, 0x07, 0x03, 0xF8, 0x80, 0x03, 0x01, 0xFC, 0x00, 0x01, 0x00, 0xFE
};

Adafruit_SSD1306 display = Adafruit_SSD1306();

void setup() {
  Serial.begin(115200);
 
  Serial.println();
  Serial.println("osquery dev board v0.1");
  Serial.println("Connecting to wifi");
  Serial.println(kSsid);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)

  display.print("Connecting to WiFi API: " + kSsid);
  delay(500);
  
  WiFi.begin(kSsid.c_str(), kPassword.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String getMktSummary(DynamicJsonDocument& blob) {

  HTTPClient http;
  http.begin(kYahooApi, kSha1Fingerprint);
  http.addHeader("X-RapidAPI-Host", kApiHost);
  http.addHeader("X-RapidAPI-Key", kApiKey);
  http.GET(kYahooApi);

  Stream& response = http.getStream();
  deserializeJson(blob, response);

  /*
  WiFiClientSecure client;
  if (!client.connect(kApiHost, 443)) {
    Serial.println("connection failed");
    return String();
  }

  if (client.verify(kSha1Fingerprint, kApiHost)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  // TODO: Add headers
  client.print(String("GET ") + kYahooApi + " HTTP/1.1\r\n" +
               "Host: " + kApiHost + "\r\n" +
               "User-Agent: arduino-mkt-summary-fetch-muffins\r\n" +
               "Connection: close\r\n\r\n");
 

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //Serial.println("headers received");
      break;
    }
  }

  String json = client.readStringUntil('}');
  DeserializationError err = deserializeJson(blob, json);
  
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  */
}
 
void loop() {
  
  DynamicJsonDocument doc(kMaxJsonDoc);
  getMktSummary(doc);

  String status = doc["marketSummaryResponse"]["error"];
  display.print("E: " + status);

  /*
  display.clearDisplay();
  display.setCursor(0,0);
  display.drawBitmap(90, 0, kOsqueryLogo, 32, 32, WHITE);
  display.display();
  */
 
  delay(kMainDelay);
}
