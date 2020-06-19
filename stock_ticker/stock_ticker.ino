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
const String kSsid = "XXXX";
const String kPassword = "XXXXX";

// AlphaVantage API information, they have free tokens ;)
const String kApiUri = "https://www.alphavantage.co";
const String kApiBaseUri = "https://www.alphavantage.co/query?";
const String kApiStockPrice = "function=GLOBAL_QUOTE&";
const String kApiExchangeRate = "function=CURRENCY_EXCHANGE_RATE&";
const String kApiFromCurrency = "from_currency=BTC&";
// Note: we leave the & off of this one as the last piece is the kApiKey
const String kApiToCurrency = "to_currency=USD";
const String kApiSymbol = "symbol=";
const String kApiKey = "&apikey=XXXXXXXXXXXXXXX";

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

// Max document size that we can parse for JSON, resp from AV has about 10 vals
const size_t kMaxJsonDoc = 0x400;
StaticJsonDocument<kMaxJsonDoc> doc;

// Main loop pause
const size_t kDelay = 30000;
// Seconds to wait between POSTs to AlphaVantage
const size_t kStockPause = 15000;
// Blink delay for the lights
const size_t kBlinkDelay = 1000;

// SHA1 fingerprint of the sites certificate
const char* kAlphaVantageFingerprint = "C9 44 AD B9 48 6F 7C D1 64 5F ED 9B 49 53 56 88 DA FD FC 4B";

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
  auto ret = https.begin(*secure_client, uri);

  if (!ret) {
    Serial.println("Failed to connect to URI with " + String(ret));
    return 1;
  }
  
  auto status = https.GET();
  if (status != HTTP_CODE_OK) {
    Serial.println("HTTPS GET failed with code " + String(status));
    return status;
  }

  resp = https.getString();
  return status;
}

void blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(kBlinkDelay);
  digitalWrite(LED_BUILTIN, LOW);
  delay(kBlinkDelay);  
}

void loop() {
  
  // Blink to indicate we're about to fetch new stocks
  blink();

  // We fetch stock values in a loop around the ticker symbols, defined above
  for(const auto sym : kTickerSymbols) {
    Serial.println("Fetching price for " + sym);

    // Perform the API request
    String resp;
    String uri = kApiBaseUri + kApiStockPrice + kApiSymbol + sym + kApiKey;
    auto ret = getApiResponse(uri, resp);
    if (ret == 1) {
      Serial.println("Fetch remote resource failed. Continuing");
      delay(kStockPause);
      continue;
    }

    // Parse the response into a JSON object
    auto err = deserializeJson(doc, resp);
    if (err) {
      Serial.println("Failed to parse response to JSON with " + String(err.c_str()));
      delay(kStockPause);
      continue;
    }

    // We display the price
    auto quote = doc["Global Quote"];
    auto quote_msg = sym + ": " + quote["05. price"].as<String>() + ", " + 
                     quote["09. change"].as<String>() + " (" + 
                     quote["10. change percent"].as<String>() + ")";
    Serial.println(quote_msg);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(quote_msg);
    display.display();
    delay(kStockPause);
  }

  // Lastly, fetch the USD -> BTC exchange rate, for funsies
  String resp;
  String uri = kApiBaseUri + kApiExchangeRate + kApiFromCurrency + 
               kApiToCurrency + kApiKey;
  auto ret = getApiResponse(uri, resp);

  // Parse the response into a JSON object
  auto err = deserializeJson(doc, resp);
  if (err) {
    Serial.println("Failed to parse response to JSON with " + String(err.c_str()));
  } else {
    // We display the price
    auto er = doc["Realtime Currency Exchange Rate"] ;
    auto msg = er["1. From_Currency Code"].as<String>() + " -> " + 
               er["3. To_Currency Code"].as<String>() + ": " + 
               er["5. Exchange Rate"].as<String>();
    Serial.println(msg);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(msg);
    display.display();
    delay(kDelay);
  }
}
