# Arduino-ESP8266-RGB-Web-Server
A Wi-Fi web server for the ESP8266 platform driving a WS2812B addressable RGB Led strip - Arduino sketch

Description:
An Arduino sketch for running a Wi-Fi web server on the ESP8266. Designed to drive the WS2812B addressable RGB Led strips using the Adafruit Neopixel Arduino library.

=============================================     Implements the following    ======================================================

Adafruit Neopixel Library:
https://learn.adafruit.com/adafruit-neopixel-uberguide?view=all#arduino-library


ESP8266 Web Server:
https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/examples/AdvancedWebServer/AdvancedWebServer.ino


==============================================        Useful Information       =======================================================

ESP8266 Setup Guide
https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide?_ga=1.192693098.1720005138.1446065268

Future improvements I would like too add.

1) Additional operation mode that allows the user to set a 'sunrise' wake-up. The lamp will switch on at a certain time at zero brightnrss and then gradually get brighter and brighter over a user definable period of time. The color setting should also be user definable. Perhaps implementing an internet time look up to update an on-board clock.

2) Test all this on a standard ESP8266 (around $5) rather than using the Sparkfun ESP8266 Thing.
