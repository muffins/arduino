#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include <DallasTemperature.h>
#include <OneWire.h>

#include <Stream.h>
#include <WiFiClientSecure.h>

// Wifi credential information
const String kSsid = "XXXXXXX";
const String kPassword = "XXXXXXXXXXXXXXXX";

// Overall delay, Thingspeak limits to 15 seconds
const int kDelay = 20000;

// Init the ESP32 Wifi Module
WiFiMulti WiFiMulti;

// Thingspeak API information
const char* kThingSpeakThumbPrint = "F9 C2 65 6C F9 EF 7F 66 8B F7 35 FE 15 EA 82 9F 5F 55 54 3E";
const String kThingSpeakUri = "https://api.thingspeak.com/update?";
const String kThingSpeakApiKey = "api_key=XXXXXXXXXXXXXXXXXX";
const String kThingSpeakField1 = "&field1=";
const String kThingSpeakField2 = "&field2=";

size_t loopCount = 0;

// Data wire is plugged into pin 5 on the Arduino 
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices  
OneWire oneWire(ONE_WIRE_BUS); 

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Offset to PST (GMT-8), note, it implicitly minus' for you.
// TODO this is still wrong :(
const long gmtOffset_sec = 28800;
const int daylightOffset_sec = 3600;

// Helper function to sync our internal clock
void setClock() {
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

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
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

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
  sensors.begin();
  setClock();
}

// Fetches a string response from a remote API endpoint URI
int getApiResponse(const String& uri, String& resp) {
  std::unique_ptr<WiFiClientSecure>client(new WiFiClientSecure);
  
  HTTPClient https;
  auto ret = https.begin(*client, uri);

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
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}

// Helper function to print to serial with the current timestamp
void log_to_serial(const String& msg) {
  struct tm ti;
  getLocalTime(&ti);

  auto pre = "[" + String(ti.tm_year + 1900) + "-" + String(ti.tm_mon + 1) + "-" +
             String(ti.tm_mday) + " " + String(ti.tm_hour) + ":" + 
             String(ti.tm_min) + ":" + String(ti.tm_sec) + "] ";
  Serial.print(&ti, "[%Y-%m-%d %H:%M:%S] ");
  Serial.println(msg);
}

void loop() {
  // We blink every time we read the temperature for some form of feedback
  //blink();

  // We post the uptime in seconds as well for record
  auto uptime = millis() / 1000;

  // If we've received a message, POST the data to ThingSpeak
  sensors.requestTemperatures();
  auto temp = sensors.getTempFByIndex(0);

  String uri = kThingSpeakUri + kThingSpeakApiKey +  kThingSpeakField1 + 
               String(temp) + kThingSpeakField2 + String(uptime);

  String resp {""};
  auto ret = getApiResponse(uri, resp);
  if (ret == 200) {
    log_to_serial("Posted " + String(temp) + "F, " + String(uptime) + "s to TS.");
  } else {
    log_to_serial("Failed to post data to TS");
  }
  

  delay(kDelay);
}
