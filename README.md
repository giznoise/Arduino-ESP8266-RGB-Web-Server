# Arduino-ESP8266-RGB-Web-Server
A Wi-Fi web server for the ESP8266 platform driving a WS2812B addressable RGB Led strip - Arduino sketch

Description:
An Arduino sketch for running a Wi-Fi web server on the ESP8266-12F. Designed to drive the WS2812B addressable RGB Led strips using the Adafruit Neopixel Arduino library.

Construction Notes:
The software has been written to give the best effects with a 60 LED WS2812 RGB LED strip but will work with any. The number of LED's in your strip can be set in code.

To get the best effects (especially in candle mode) wrap the strip around a tube approx. 1.5 inches dia. to form a cylinder of light. Place this in an opaque container to obfuscate the individual LED's to give an overall 'glow'.

This ensures that the rainbow mode is effective and also the candle mode appears to be a small tea light or candle at the bottom of the container.

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

2) Test all this on a standard ESP8266-12F (around $5) rather than using the 'Sparkfun ESP8266 Thing'. - UPDATE: Done, it works Ok.

3) Allow the user to set the transition time (via the html menu system), between colors for the color fade mode and rainbow mode.

4) Update 21/07/2016 - Candle mode added. Experiment with '#define CANDLEPIXELS XX' (default 30) to get the best effect for your set up. Use more or less pixels to suit how bright you want the candle to be. Tweak the code if you want to try and make a better candle effect. Brightness and Colour set mode do not function when in candle mode because colour and brightness level are fundamental parts of the candle effect.

5) Add more modes - Strobe, Disco, Breathing 

6) Start the RGB Lamp in AP Mode first so that a user can connect directly and determine the I.P. Address it occupies on the local network. Maybe give 20 seconds for an incoming AP connection, then switch to server mode / normal operation. 

The only way to determine the IP address of the lamp at the moment is to either connect to the serial port and monitor the output on start-up or log into your router. You can either tell the router to allocate an IP address based on the MAC address of your ESP8266 or just look to see what is connected and on what IP in your router logs.
