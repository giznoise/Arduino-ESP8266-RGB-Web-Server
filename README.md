# Arduino-ESP8266-RGB-Web-Server
A Wi-Fi web server for the ESP8266 platform driving a WS2812B addressable RGB Led strip - Arduino sketch

Description:
An Arduino sketch for running a Wi-Fi web server on the ESP8266-12F. Designed to drive the WS2812B addressable RGB Led strips using the Adafruit Neopixel Arduino library.

Implements the following:

Adafruit Neopixel Library:
https://learn.adafruit.com/adafruit-neopixel-uberguide?view=all#arduino-library


ESP8266 Web Server:
https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/examples/AdvancedWebServer/AdvancedWebServer.ino


Useful Information:

ESP8266 Setup Guide:
https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide?_ga=1.192693098.1720005138.1446065268

Future improvements I would like too add.

1) Additional operation mode that allows the user to set a 'sunrise' wake-up alarm. The lamp will switch on at a certain time at zero brightness and then gradually get brighter and brighter over a user definable period of time. The color setting should also be user definable. Perhaps implementing an internet time look up to update an on-board clock.

2) Test all this on a standard ESP8266-12F (around $5) rather than using the 'Sparkfun ESP8266 Thing'. - Done, it works Ok.

3) Allow the user to set the transition time (via the html menu system), between colors for the color fade mode and rainbow mode.

4) Add more modes - Candle (Neo Candle), Strobe, Disco, Breathing

5) Start the RGB Lamp in AP Mode first so that a user can connect directly and determine the I.P. Address it occupies on the local network. Maybe give 20 seconds for an incoming AP connection, then switch to server mode / normal operation. 

The only way to determine the IP address of the lamp at the moment is to either connect to the serial port and monitor the output on start-up or log into your router. You can either tell the router to allocate an IP address based on the MAC address of your ESP8266 or just look to see what is connected and on what IP in your router logs.
