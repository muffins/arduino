#include <OneWire.h>
#include <DallasTemperature.h>
//#include <SD.h>

/********************************************************************/
// Data wire is plugged into pin 5 on the Arduino 
#define ONE_WIRE_BUS 5
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  /*
  Serial.println("Initializing SDcard");
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
  }
  */

  Serial.println("Temperature sensor initialized");
  sensors.begin();
}

void log_data(String log) {
  
}

void loop() {
  // We blink every time we read the temperature for some form of feedback
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second

  // Send the command to get temperature readings
  sensors.requestTemperatures();
  
  Serial.println("Temperature is: "); 
  Serial.println(sensors.getTempFByIndex(0));
}
