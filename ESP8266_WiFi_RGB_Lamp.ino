/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "html_pages.h"

#define PIN 0
#define NUMPIXELS 60
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char *ssid = "SKY5BD72";
const char *password = "CFTRWQBD";
MDNSResponder mdns;

ESP8266WebServer server ( 80 );

const int led = 5;
const int delayval = 500;
String rgb_now = "#0000ff";    //global rgb state values for use in various html pages
long red_int = 0;
long green_int = 0;
long blue_int = 255;
int brightness = 255;
int mode_flag = 1;




void setup ( void ) {
	pinMode ( led, OUTPUT );
	digitalWrite ( led, 0 );
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

  handleSwitchOn();
}

void loop ( void ) {
	mdns.update();
	server.handleClient();
}

void handleIndex(){
Serial.println ( "Request for index page received" );
server.send ( 200, "text/html", page_contents);
}

void handleNotFound() {
  digitalWrite ( led, 1 );
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
  digitalWrite ( led, 0 );
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
 //  light_up_all();
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
   mode_flag=1;                                       //go to default fixed color mode and turn off all pixels
  for(int i=0;i<NUMPIXELS;i++){                                                                                                              // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
   strip.setPixelColor(i, strip.Color(0,0,0));                                                                                               // Turn off led strip
   strip.show();}                                                                                                                            // This sends the updated pixel color to the hardware.
  }  

void handle_mode1(){                                  //fixed colour mode
  mode_flag = 1;
  EEPROM.write(4,mode_flag);                          //write mode to EEProm so can be restored on start-up
  EEPROM.commit();
  server.send ( 200, "text/html","<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>");
  handleSelectMode();
  light_up_all();                                     //set mode to default state - all led's on, fixed colour
}


void handle_mode2(){                                 //colour fade mode
  mode_flag = 2;
  EEPROM.write(4,mode_flag);                         //write mode to EEProm so can be restored on start-up
  EEPROM.commit();
  server.send ( 200, "text/html","<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>");
  handleSelectMode();
  uint16_t i, j, k;
  int wait = 10;  //DON'T ever set this more than '10'. Use the 'k' value in the loop below to increase delays. This prevents the watchdog timer timing out on the ESP8266

 while(1){
  for(j=0; j<256; j++) {
   loop();
    for(i=0; i<NUMPIXELS; i++) {
      loop();
      strip.setPixelColor(i, Wheel((j) & 255));
      if(mode_flag!=2){return;}                      //the mode has been changed - get outta here!
      strip.show();
    } loop(); 
      
     for(k=0; k < 200; k++){                          // Do ten loops of the 'wait' and service loop routine inbetween. Total wait = 10 x 'wait'. This prevents sluggishness in the browser html front end menu.
      delay(wait);
      loop();
      if(mode_flag!=2){return;}}                     // Check the mode hasn't been changed whilst we wait, if so - leave immediately
     }loop();
     }
}

void handle_mode3(){                                //rainbow mode
  mode_flag = 3;
  EEPROM.write(4,mode_flag);                        //write mode to EEProm so can be restored on start-up
  EEPROM.commit();
  server.send ( 200, "text/html","<SCRIPT language='JavaScript'>window.location='/';</SCRIPT>");
  handleSelectMode();
  uint16_t i, j, k;
  int wait = 10;  //DON'T ever set this more than '10'. Use the 'k' value in the loop below to increase delays. This prevents the watchdog timer timing out on the ESP8266
  
 while(1){                                          // do this indefenitely or until mode changes
  for(j=0; j < 256*5; j++) {                        // 5 cycles of all colors on wheel
   loop();
    for(i=0; i < NUMPIXELS; i++) {
      loop();
      strip.setPixelColor(i,Wheel(((i * 256 / NUMPIXELS) + j) & 255));
      if(mode_flag!=3){return;}                    //the mode has been changed - get outta here!
    }
    strip.show();

    for(k=0; k < 50; k++){                         // Do ten loops of the 'wai't and service loop routine inbetween. Total wait = 10 x 'wait'. This prevents sluggishness in the browser html front end menu.
     delay(wait);
     loop();
     if(mode_flag!=3){return;}}                    // Check the mode hasn't been changed whilst we wait, if so - leave immediately
    }
 }}


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
  

