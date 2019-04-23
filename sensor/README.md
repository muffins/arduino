## Sensor

This project is for a simple temperature sensor that periodically writes the
current ambient temp, and whatever else I get soldered onto this thing, to an
sdcard. Once an hour the data is aggregated (or will be, when I get an 8266) and
POSTed out to an external graphana endpoint for analysis.


### Extras
Guides for Wiring and Code Samples:
[Good guide for Wiring](https://learn.adafruit.com/using-ds18b20-temperature-sensor-with-circuitpython/)hardware
[Good code samples](https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806)