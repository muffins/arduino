
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>

// Wifi credential information
const String kSsid = "Pwn135";
const String kPassword = "my.little.pwnies";

// Yahoo Finance API information
const char* kYahooApi = "/market/get-summary?region=US&lang=en";
const char* kApiHost = "apidojo-yahoo-finance-v1.p.rapidapi.com";
//const char* kApiUri = "https://apidojo-yahoo-finance-v1.p.rapidapi.com/market/get-summary?region=US&lang=en";
const char* kApiUri = "https://canhazip.com";
const uint16_t kApiPort = 443;
const char* kApiKey = "2822998c09msh41e18f03cdf82dep115067jsndc7284e628ad";
const char* kComodoRootCa = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF2DCCA8CgAwIBAgIQTKr5yttjb+Af907YWwOGnTANBgkqhkiG9w0BAQwFADCB\n" \
"hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n" \
"A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n" \
"BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAwMTE5\n" \
"MDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n" \
"EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n" \
"Q09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNh\n" \
"dGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCR\n" \
"6FSS0gpWsawNJN3Fz0RndJkrN6N9I3AAcbxT38T6KhKPS38QVr2fcHK3YX/JSw8X\n" \
"pz3jsARh7v8Rl8f0hj4K+j5c+ZPmNHrZFGvnnLOFoIJ6dq9xkNfs/Q36nGz637CC\n" \
"9BR++b7Epi9Pf5l/tfxnQ3K9DADWietrLNPtj5gcFKt+5eNu/Nio5JIk2kNrYrhV\n" \
"/erBvGy2i/MOjZrkm2xpmfh4SDBF1a3hDTxFYPwyllEnvGfDyi62a+pGx8cgoLEf\n" \
"Zd5ICLqkTqnyg0Y3hOvozIFIQ2dOciqbXL1MGyiKXCJ7tKuY2e7gUYPDCUZObT6Z\n" \
"+pUX2nwzV0E8jVHtC7ZcryxjGt9XyD+86V3Em69FmeKjWiS0uqlWPc9vqv9JWL7w\n" \
"qP/0uK3pN/u6uPQLOvnoQ0IeidiEyxPx2bvhiWC4jChWrBQdnArncevPDt09qZah\n" \
"SL0896+1DSJMwBGB7FY79tOi4lu3sgQiUpWAk2nojkxl8ZEDLXB0AuqLZxUpaVIC\n" \
"u9ffUGpVRr+goyhhf3DQw6KqLCGqR84onAZFdr+CGCe01a60y1Dma/RMhnEw6abf\n" \
"Fobg2P9A3fvQQoh/ozM6LlweQRGBY84YcWsr7KaKtzFcOmpH4MN5WdYgGq/yapiq\n" \
"crxXStJLnbsQ/LBMQeXtHT1eKJ2czL+zUdqnR+WEUwIDAQABo0IwQDAdBgNVHQ4E\n" \
"FgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB\n" \
"/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAArx1UaEt65Ru2yyTUEUAJNMnMvl\n" \
"wFTPoCWOAvn9sKIN9SCYPBMtrFaisNZ+EZLpLrqeLppysb0ZRGxhNaKatBYSaVqM\n" \
"4dc+pBroLwP0rmEdEBsqpIt6xf4FpuHA1sj+nq6PK7o9mfjYcwlYRm6mnPTXJ9OV\n" \
"2jeDchzTc+CiR5kDOF3VSXkAKRzH7JsgHAckaVd4sjn8OoSgtZx8jb8uk2Intzna\n" \
"FxiuvTwJaP+EmzzV1gsD41eeFPfR60/IvYcjt7ZJQ3mFXLrrkguhxuhoqEwWsRqZ\n" \
"CuhTLJK7oQkYdQxlqHvLI7cawiiFwxv/0Cti76R7CZGYZ4wUAc1oBmpjIXUDgIiK\n" \
"boHGhfKppC3n9KUkEEeDys30jXlYsQab5xoq2Z0B15R97QNKyvDb6KkBPvVWmcke\n" \
"jkk9u+UJueBPSZI9FoJAzMxZxuY67RIuaTxslbH9qh17f4a+Hg4yRvv7E491f0yL\n" \
"S0Zj/gA0QHDBw7mh3aZw4gSzQbzpgJHqZJx64SIDqZxubw5lT2yHh17zbqD5daWb\n" \
"QOhTsiedSrnAdyGN/4fy3ryM7xfft0kL0fJuMAsaDk527RH89elWsn2/x20Kk4yl\n" \
"0MC2Hb46TpSi125sC8KKfPog88Tk5c0NqMuRkrF8hey1FGlmDoLnzc7ILaZRfyHB\n" \
"NVOFBkpdn627G190\n" \
"-----END CERTIFICATE-----\n";

// Max document size that we can parse for JSON
const size_t kMaxJsonDoc = 0x4096;

// Main loop pause
const size_t kMainDelay = 10000;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
//const char* kSha1Fingerprint = "01 3E 44 23 62 2D 92 54 6D 75 7B 2C 60 A7 33 A2 FB 60 C1 3E";
const char* kCanHazFingerprint = "39 11 70 BA 7D 5D D1 E0 3C 80 A8 5B B2 8F 28 57 67 A1 FC 4A";

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

ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
 
  Serial.println();
  Serial.println("Stock Ticker Board v0.1");
  Serial.print("Connecting to wifi: ");
  Serial.print(kSsid);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  delay(500);
  
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
}

int getMktSummary(DynamicJsonDocument& blob) {

  // bool begin(WiFiClient &client, String host, uint16_t port, String uri = "/", bool https = false);

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setFingerprint(kCanHazFingerprint);

  HTTPClient https;
  Serial.println("Beginning connection to API endpoint");
  auto ret = https.begin(*client, kApiUri);

  if (!ret) {
    Serial.println("Failed to connect to URI with " + String(ret));
    return 1;
  }

  Serial.println("Sending HTTP GET request");
  auto status = https.GET();
  if (status != HTTP_CODE_OK) {
    Serial.println("HTTPS GET failed with code " + String(status));
    return 1;
  }

  String payload = https.getString();
  Serial.println("Successfully connected to " + String(kApiHost));
  Serial.println(payload);

  /*
  String json = client.readStringUntil('}');
  DeserializationError err = deserializeJson(blob, json);
  
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  */
  return status;
}
 
void loop() {
  
  DynamicJsonDocument doc(kMaxJsonDoc);
  Serial.println("Fetching Market Summary");
  auto ret = getMktSummary(doc);

  //auto status = doc["marketSummaryResponse"]["error"];
  display.clearDisplay();
  display.setCursor(0,0);
  auto msg = "Connection to API: " + String(ret);
  display.print(msg);

  /*
  display.drawBitmap(90, 0, kOsqueryLogo, 32, 32, WHITE);
  display.display();
  */
 
  delay(kMainDelay);
}
