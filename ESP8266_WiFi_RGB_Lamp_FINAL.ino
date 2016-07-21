/*ESP8266WiFiWebServer
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 */

/*The following project leans heavily on the NeoPixel library from Adafruit */
/* Many thanks to Adafruit for the NeoPixel library and NeoCandle code. */
/* https://www.adafruit.com/ */


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "html_pages.h"

#define PIN 0
#define NUMPIXELS 60                 // Number of LED's in your strip

// START OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////
#define CANDLEPIXELS 30      //This is how may led's will represent your 'wick'. With a 60 LED strip, 30 gives the effect of a tealight or small candle when in an opaque container.
// END OF CANDLE MODE RELATED STUFF //////////////////////////////////////////////////


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// START OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////
Adafruit_NeoPixel *wick;
byte state;
unsigned long flicker_msecs;
unsigned long flicker_start;
byte index_start;
byte index_end;
// END OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////////

const char *ssid = "<Put your own SSID here";
const char *password = "<Put your Password here>";
MDNSResponder mdns;

ESP8266WebServer server ( 80 );

const int delayval = 500;
String rgb_now = "#0000ff";    //global rgb state values for use in various html pages
long red_int = 0;
long green_int = 0;
long blue_int = 255;
int brightness = 255;
int mode_flag = 1;

// START OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////
#define WICK_PIN                PIN
// Any unconnected pin, to try to generate a random seed
#define UNCONNECTED_PIN         2
 
// The LED can be in only one of these states at any given time
#define BRIGHT                  0
#define UP                      1
#define DOWN                    2
#define DIM                     3
#define BRIGHT_HOLD             4
#define DIM_HOLD                5
 
// Percent chance the LED will suddenly fall to minimum brightness
#define INDEX_BOTTOM_PERCENT    10
// Absolute minimum red value (green value is a function of red's value)
#define INDEX_BOTTOM            128
// Minimum red value during "normal" flickering (not a dramatic change)
#define INDEX_MIN               192
// Maximum red value
#define INDEX_MAX               255
 
// Decreasing brightness will take place over a number of milliseconds in this range
#define DOWN_MIN_MSECS          20
#define DOWN_MAX_MSECS          250
// Increasing brightness will take place over a number of milliseconds in this range
#define UP_MIN_MSECS            20
#define UP_MAX_MSECS            250
// Percent chance the color will hold unchanged after brightening
#define BRIGHT_HOLD_PERCENT     20
// When holding after brightening, hold for a number of milliseconds in this range
#define BRIGHT_HOLD_MIN_MSECS   0
#define BRIGHT_HOLD_MAX_MSECS   100
// Percent chance the color will hold unchanged after dimming
#define DIM_HOLD_PERCENT        5
// When holding after dimming, hold for a number of milliseconds in this range
#define DIM_HOLD_MIN_MSECS      0
#define DIM_HOLD_MAX_MSECS      50
 
#define MINVAL(A,B)             (((A) < (B)) ? (A) : (B))
#define MAXVAL(A,B)             (((A) > (B)) ? (A) : (B))
 
// END OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////////

void handleIndex();
void handleNotFound();
void handleSwitchOn();
void handleSwitchOff();
void handleSetColour();
void handleColour();
void handleSetBrightness();
void handleBrightness();
void handleSelectMode();
void handle_mode1();
void handle_mode2();
void handle_mode3();
void handle_mode4();
void set_color(byte);
void light_up_all();
void turn_off_all();
uint32_t Wheel(byte);

void setup ( void ) {
	Serial.begin ( 115200 );
	WiFi.begin ( ssid, password );
	Serial.println ( "" );

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
		delay ( 500 );
		Serial.print ( "." );                              
	}

  EEPROM.begin(5);                           // set up EEPROM storage space for config values

	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.println ( ssid );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );

	if ( mdns.begin ( "esp8266", WiFi.localIP() ) ) {
		Serial.println ( "MDNS responder started" );
	}

  Serial.println ( "HTTP server started" );

  server.on ( "/", handleIndex );
  server.onNotFound ( handleNotFound );
  
  server.on ( "/switch_on", handleSwitchOn);
  server.on ( "/switch_off", handleSwitchOff);
  server.on ( "/set_colour", handleSetColour);
  server.on ( "/set_colour_hash", handleColour );
  server.on ( "/set_brightness", handleSetBrightness);
  server.on ( "/set_bright_val", handleBrightness);
  server.on ( "/select_mode", handleSelectMode);
  server.on ( "/set_mode1", handle_mode1); 
  server.on ( "/set_mode2", handle_mode2);
  server.on ( "/set_mode3", handle_mode3);
  server.on ( "/set_mode4", handle_mode4);

//EEPROM Memory//
//Mem Location ||--0--||--1--||--2--||--3--||--4--||--5--||--6--||
//                red   green  blue   bright  mode

  red_int = EEPROM.read(0);       //restore colour to last used value. Ensures RGB lamp is same colour as when last switched off
  green_int = EEPROM.read(1);
  blue_int = EEPROM.read(2);
  brightness = EEPROM.read(3);
  mode_flag = EEPROM.read(4);
	
	server.begin();
  strip.begin();


// START OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////
  // There is no good source of entropy to seed the random number generator,
  // so we'll just read the analog value of an unconnected pin.  This won't be
  // very random either, but there's really nothing else we can do.
  //
  // True randomness isn't strictly necessary, we just don't want a whole
  // string of these things to do exactly the same thing at the same time if
  // they're all powered on simultaneously.
  randomSeed(analogRead(UNCONNECTED_PIN));
 
 
  wick = new Adafruit_NeoPixel(CANDLEPIXELS, WICK_PIN, NEO_GRB + NEO_KHZ800);
  wick->begin();
  wick->show();
 
  set_color(255);
  index_start = 255;
  index_end = 255;
  state = BRIGHT;
// END OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////////

  handleSwitchOn();
}

void loop ( void ) {
	mdns.update();
	server.handleClient();


// START OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////
// The candle mode related stuff is here to ensure it has the fastest response time
// All the other modes have their own functions outside of the main loop

  if (mode_flag==4){
 
  unsigned long current_time; 
  current_time = millis();

//Serial.println("In Loop");
//Serial.println(state);
  
  switch (state)
    {
    case BRIGHT:   
    // Serial.println("Bright"); 
      mdns.update();
      server.handleClient();
      flicker_msecs = random(DOWN_MAX_MSECS - DOWN_MIN_MSECS) + DOWN_MIN_MSECS;
      flicker_start = current_time;
      index_start = index_end;
      if ((index_start > INDEX_BOTTOM) &&
          (random(100) < INDEX_BOTTOM_PERCENT))
        index_end = random(index_start - INDEX_BOTTOM) + INDEX_BOTTOM;
      else
        index_end = random(index_start - INDEX_MIN) + INDEX_MIN;
 
      state = DOWN;
      break;  
      
    case DIM:
    // Serial.println("Dim");
      mdns.update();
      server.handleClient();
      flicker_msecs = random(UP_MAX_MSECS - UP_MIN_MSECS) + UP_MIN_MSECS;
      flicker_start = current_time;
      index_start = index_end;
      index_end = random(INDEX_MAX - index_start) + INDEX_MIN;
      state = UP;
      break;
    
    case BRIGHT_HOLD:  
    case DIM_HOLD:
   //  Serial.println("DIM Hold");
      mdns.update();
      server.handleClient();
      if (current_time >= (flicker_start + flicker_msecs))
        state = (state == BRIGHT_HOLD) ? BRIGHT : DIM; 
      break;
   
   case UP:
   case DOWN:
  //  Serial.println("Down");
     mdns.update();
     server.handleClient();
      if (current_time < (flicker_start + flicker_msecs))
        set_color(index_start + ((index_end - index_start) * (((current_time - flicker_start) * 1.0) / flicker_msecs)));
      else
        {
        set_color(index_end);
 
        if (state == DOWN)
          {
          if (random(100) < DIM_HOLD_PERCENT)
            {
            flicker_start = current_time;
            flicker_msecs = random(DIM_HOLD_MAX_MSECS - DIM_HOLD_MIN_MSECS) + DIM_HOLD_MIN_MSECS;
            state = DIM_HOLD;
            }
          else
            state = DIM;
          }
        else
          {
          if (random(100) < BRIGHT_HOLD_PERCENT)
            {
            flicker_start = current_time;
            flicker_msecs = random(BRIGHT_HOLD_MAX_MSECS - BRIGHT_HOLD_MIN_MSECS) + BRIGHT_HOLD_MIN_MSECS;
            state = BRIGHT_HOLD;
            }
          else
            state = BRIGHT;
          }
        }
 
      break; 
    } } 
     // END OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////////

  return;
  }
 
void handleIndex(){
Serial.println ( "Request for index page received" );
server.send ( 200, "text/html", page_contents);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

 void handleSwitchOn() {
  mode_flag = EEPROM.read(4);                       // start-up in last saved mode
  delay(100);
  switch(mode_flag){
      case 1:handle_mode1();
      break;
      case 2:handle_mode2();
      break;
      case 3:handle_mode3();
      break;
      case 4:handle_mode4();
      break;
      default:                  
           light_up_all();                          //Default to fixed colour should the EEProm become corrupted
      break;
  }
  server.send ( 200, "text/html", "<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>" );
  };  
  
 void handleSwitchOff() {
  mode_flag=1;                                       //go to default fixed color mode and turn off all pixels
  delay(100);
  turn_off_all();
  server.send ( 200, "text/html", "<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>" );
  }
  
void handleSetColour() {     
        server.send ( 200, "text/html", colour_picker);
  }
 
  
void handleSetBrightness(){
   server.send ( 200, "text/html", bright_set);
  }  

    
void handleSelectMode(){
  server.send ( 200, "text/html", mode_page );
//  Serial.println ( "Mode select page" );
  }


void handleColour(){
  char buf_red[3];                               //char buffers to hold 'String' value converted to char array
  char buf_green[3];                       
  char buf_blue[3];                       
  String message = server.arg(0);                //get the 1st argument from the url which is the hex rgb value from the colour picker ie. #rrggbb (actually %23rrggbb)
  rgb_now = message; 
  rgb_now.replace("%23", "#");                   // change %23 to # as we need this in one of html pages
  String red_val = rgb_now.substring(1,3);       //extract the rgb values
  String green_val = rgb_now.substring(3,5); 
  String blue_val = rgb_now.substring(5,7);

  mode_flag=1;                                   //get to fixed colour mode if not already

  red_val.toCharArray(buf_red,3);                //convert hex 'String'  to Char[] for use in strtol() 
  green_val.toCharArray(buf_green,3);           
  blue_val.toCharArray(buf_blue,3);             

  red_int = gamma_adjust[strtol( buf_red, NULL, 16)];          //convert hex chars to ints and apply gamma adjust
  green_int = gamma_adjust[strtol( buf_green, NULL, 16)];
  blue_int = gamma_adjust[strtol( buf_blue, NULL, 16)];

 EEPROM.write(0,red_int);                  //write the colour values to EEPROM to be restored on start-up
 EEPROM.write(1,green_int);
 EEPROM.write(2,blue_int);
 EEPROM.commit();

   light_up_all(); 
   String java_redirect = "<SCRIPT language='JavaScript'>window.location='/set_colour?";
          java_redirect += message;                                                 //send hash colour value in URL to update the colour picker control
          java_redirect += "';</SCRIPT>";
   server.send ( 200, "text/html", java_redirect );                                 // all done! - take user back to the colour picking page                                                                                          
  }

 void handleBrightness(){
  String message = server.arg(0);                //get the 1st argument from the url which is the brightness level set by the slider
  String bright_val = message.substring(0,3);    //extract the brightness value from the end of the argument in the URL
  brightness =  bright_val.toInt();
  EEPROM.write(3,brightness);                    //write the brightness value to EEPROM to be restored on start-up
  EEPROM.commit();

   String java_redirect = "<SCRIPT language='JavaScript'>window.location='/set_brightness?";
          java_redirect += brightness;                                              //send brightness value in URL to update the slider control
          java_redirect += "';</SCRIPT>";
   server.send ( 200, "text/html", java_redirect);                                  // all done! - take user back to the brightness selection page                                                                                          
  }

  void light_up_all(){
  for(int i=0;i<NUMPIXELS;i++){      
   strip.setPixelColor(i, strip.Color(brightness*red_int/255,brightness*green_int/255,brightness*blue_int/255));                            // Set colour with gamma correction and brightness adjust value. 
   strip.show();}                                                                                                                            
  }

  void turn_off_all(){
   mode_flag=999;                                       //go to non-existent mode and turn off all pixels
  for(int i=0;i<NUMPIXELS;i++){                                                                                                              // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
   strip.setPixelColor(i, strip.Color(0,0,0));                                                                                               // Turn off led strip
   strip.show();}                                                                                                                            // This sends the updated pixel color to the hardware.
  }  

void handle_mode1(){                                  //fixed colour mode
  mode_flag = 1;
  EEPROM.write(4,mode_flag);                          //write mode to EEProm so can be restored on start-up
  EEPROM.commit();
  server.send ( 200, "text/html","<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>");
                                   
  while(mode_flag==1){                                // Check the mode hasn't been changed whilst we wait, if so - leave immediately
            light_up_all();                           //set mode to default state - all led's on, fixed colour. This loop will service any brightness changes
            loop();                                   // Not much to do except service the main loop       
  }return;          
}


void handle_mode2(){                                 //colour fade mode
  mode_flag = 2;
  EEPROM.write(4,mode_flag);                         //write mode to EEProm so can be restored on start-up
  EEPROM.commit();
  server.send ( 200, "text/html","<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>");
  uint16_t i, j, k;
  int wait = 10;  //DON'T ever set this more than '10'. Use the 'k' value in the loop below to increase delays. This prevents the watchdog timer timing out on the ESP8266

 while(mode_flag==2){
  for(j=0; j<256; j++) {
   loop();
    for(i=0; i<NUMPIXELS; i++) {
      loop();
      strip.setPixelColor(i, Wheel((j) & 255));
      strip.show();
    } loop(); 
      
     for(k=0; k < 200; k++){                          // Do ten loops of the 'wait' and service loop routine inbetween. Total wait = 10 x 'wait'. This prevents sluggishness in the browser html front end menu.
      delay(wait);
      loop();
     }loop();}
     return;}
}

void handle_mode3(){                                //rainbow mode
  mode_flag = 3;
  EEPROM.write(4,mode_flag);                        //write mode to EEProm so can be restored on start-up
  EEPROM.commit();
  server.send ( 200, "text/html","<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>");
  uint16_t i, j, k;
  int wait = 10;  //DON'T ever set this more than '10'. Use the 'k' value in the loop below to increase delays. This prevents the watchdog timer timing out on the ESP8266
  
 while(mode_flag==3){                               // do this indefenitely or until mode changes
  for(j=0; j < 256*5; j++) {                        // 5 cycles of all colors on wheel
   loop();
    for(i=0; i < NUMPIXELS; i++) {
      loop();
      strip.setPixelColor(i,Wheel(((i * 256 / NUMPIXELS) + j) & 255));
      if(mode_flag!=3){return;}                    //the mode has been changed - get outta here!
    }
    strip.show();

    for(k=0; k < 50; k++){                         // Do ten loops of the 'wait' and service loop routine inbetween. Total wait = 10 x 'wait'. This prevents sluggishness in the browser html front end menu.
     delay(wait);
     loop();                    
    }
 }}return;
 }


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    loop();
    return strip.Color(brightness*(255 - WheelPos * 3)/255, 0, brightness*(WheelPos * 3)/255);  //scale the output values by a factor of global 'brightness' so that the brightness remains as set
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    loop();
    return strip.Color(0,brightness*(WheelPos * 3)/255, brightness*(255 - WheelPos * 3)/255);
  }
  WheelPos -= 170;
  loop();
  return strip.Color(brightness*(WheelPos * 3)/255, brightness*(255 - WheelPos * 3)/255, 0);
}


void handle_mode4(){                                //Candle mode
  mode_flag = 4;
  EEPROM.write(4,mode_flag);                        //write mode to EEProm so can be restored on start-up
  EEPROM.commit();
  server.send ( 200, "text/html","<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>");
  turn_off_all();
  mode_flag = 4;
  loop();
  }



void set_color(byte index)
  {
  int i; 
  index = MAXVAL(MINVAL(index, INDEX_MAX), INDEX_BOTTOM);
  if (index >= INDEX_MIN){
    for(i=0;i<CANDLEPIXELS;i++){
      wick->setPixelColor(i, index, (index * 3) / 8, 0);      
    }}
  else if (index < INDEX_MIN){
    for(i=0;i<CANDLEPIXELS;i++){
      wick->setPixelColor(i, index, (index * 3.25) / 8, 0);
    }}
 
  wick->show();
  return;
  }

  


  

