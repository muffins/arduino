/*
 * Nick Anderson - 2019
 * 
 * A program to fetch stock prices for various symbols and display them
 * on an Arduino OLED display
 */

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

// AlphaVantage API information, they have free tokens ;)
//const char* kApiUri = "https://canhazip.com";
const String kApiUri = "https://www.alphavantage.co";
const String kApiBaseUri = "https://www.alphavantage.co/query?";
const String kApiStockPrice = "function=GLOBAL_QUOTE&";
const String kApiExchangeRate = "function=CURRENCY_EXCHANGE_RATE&";
const String kApiFromCurrency = "from_currency=BTC&";
// Note: we leave the & off of this one as the last piece is the kApiKey
const String kApiToCurrency = "to_currency=USD";
const String kApiSymbol = "symbol=";
const String kApiKey = "&apikey=NDZC0JRPWV6C3UDA";

// Define additional ticker symbols here
const String kTickerSymbols[] = {
  "FB",   // Facebook
  "MSFT", // Microsoft
  "AAPL", // Apple
  "NFLX", // Netflix
  "PANW", // Palo Alto Networks
  "FEYE", // Fire Eye
  "AMZN", // Amazon
  "AMD",  // AMD
  "BREW"  // Craft Brew Alliance
};

// Max document size that we can parse for JSON
const size_t kMaxJsonDoc = 1024;

// Main loop pause
const size_t kMainDelay = 10000;

// SHA1 fingerprint of the sites certificate
const char* kAlphaVantageFingerprint = "9E 0B E5 F1 F4 1A 2F 29 8A 7A AA 9D B5 30 54 39 4C 20 A1 6C";
//const char* kCanHazFingerprint = "39 11 70 BA 7D 5D D1 E0 3C 80 A8 5B B2 8F 28 57 67 A1 FC 4A";

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

// Init the SSD1306 display
Adafruit_SSD1306 display = Adafruit_SSD1306();

// Init the ESP8266 Wifi Module
ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
 
  Serial.println();
  Serial.println("Stock Ticker Board v0.1");
  Serial.print("Connecting to wifi: ");
  Serial.print(kSsid);

  // initialize with the I2C addr 0x3C (for the 128x32)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  auto msg = "Connecting to " + kSsid;
  display.print(msg);
  display.setCursor(0,0);
  display.display();
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
  display.setCursor(0,15);
  display.print("IP: " + WiFi.localIP().toString());
  display.display();
  delay(2000);
}

// Fetches a string response from a remote API endpoint URI
int getApiResponse(const String& uri, String& resp) {
  Serial.println("Fetching resource from " + uri);
  std::unique_ptr<BearSSL::WiFiClientSecure>secure_client(new BearSSL::WiFiClientSecure);
  
  secure_client->setFingerprint(kAlphaVantageFingerprint);
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

void loop() {
  
  // We fetch stock values in a loop around the ticker symbols, defined above
  for(const auto sym : kTickerSymbols) {
    Serial.println("Fetching price for " + sym);

    // Perform the API request
    String resp;
    String uri = kApiBaseUri + kApiStockPrice + kApiSymbol + sym + kApiKey;
    Serial.println("Calling GET to " + uri);
    auto ret = getApiResponse(uri, resp);
    if (ret == 1) {
      Serial.println("Fetch remote resource failed. Continuing");
      continue;
    }
    Serial.println("Got back:\n" + resp);

    // Parse the response into a JSON object
    StaticJsonDocument<kMaxJsonDoc> doc;
    auto err = deserializeJson(doc, resp);
    if (err) {
      Serial.println("Failed to parse response to JSON with " + String(err.c_str()));
      continue;
    }

    // We display the price
    auto quote = doc["Global Quote"];
    auto quote_msg = sym + ": " + quote["05. price"].as<String>() + ", " + 
                     quote["09. change"].as<String>() + " (" + 
                     quote["10. change percent"].as<String>() + ")";
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(quote_msg);
    display.display();
    delay(kMainDelay);
  }

  // Lastly, fetch the USD -> BTC exchange rate, for funsies
  /*
  String resp;
  String uri = kApiBaseUri + kApiExchangeRate + kApiFromCurrency + 
               kApiToCurrency + kApiKey;
  auto ret = getApiResponse(uri, resp);
  Serial.println("Got back:\n" + resp);

  // Parse the response into a JSON object
  StaticJsonDocument<kMaxJsonDoc> doc;
  auto err = deserializeJson(doc, resp);
  if (err) {
    Serial.println("Failed to parse response to JSON with " + String(err.c_str()));
  } else {
    // We display the price
    auto er = doc["Realtime Currency Exchange Rate"] ;
    auto msg = er["1. From_Currency Code"].as<String>() + " -> " + 
               er["3. To_Currency Code"].as<String>() + ": " + 
               er["5. Exchange Rate"].as<String>();
    Serial.println("Failed to parse response to JSON with " + String(err.c_str()));
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(msg);
    display.display();
    delay(kMainDelay);
  }
  */
}
