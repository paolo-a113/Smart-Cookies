#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_Si7021.h"
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

//Adafruit_Si7021 sensor = Adafruit_Si7021();
//Receiver code  
  char cString[20];
  byte chPos = 0;
  byte ch = 0;

  String readString;


/* Put your SSID & Password */
const char* ssid = "Smart Cookies";  // Enter SSID here
const char* password = "";  //Enter Password here

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer webSocket1 = WebSocketsServer(81);

void getData() {

 
   
//  float t = sensor.readTemperature();
//  float h = sensor.readHumidity();
//
//  if (isnan(t) || t==998 || isnan(h) || h==998 ) {    
//    Serial.println("Failed to read from sensor!");
//    return;
//  }
//  else {
////    Serial.println(t);
//    char tBuf[10];
//    char hBuf[10];
//    char msgBuf[10];
//
//    dtostrf(t, 4, 2, tBuf);
//    dtostrf(h, 4, 2, hBuf);
//    strcat(msgBuf,tBuf);
//    strcat(msgBuf," ");
//    strcat(msgBuf,hBuf);
//
//    Serial.println(msgBuf);
    webSocket1.broadcastTXT(cString);
    return;
//}  
}



unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 2000;           // interval at which to blink (milliseconds)


void setup(){
  Serial.begin(115200);
  Serial.println("SMART COOKIE - SI7021");

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  //WiFi.softAP(ssid);
  WiFi.begin("ATTeEPEtxi","2s?h7j7sw8j=");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  delay(100);

//  Serial.println(WiFi.softAPIP());
  Serial.println(WiFi.localIP());

 
  if (!MDNS.begin("smartcookies")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });


  server.begin();
  MDNS.addService("http", "tcp", 80);

  webSocket1.begin();

}
 
void loop(){
  webSocket1.loop();

  MDNS.update();

  unsigned long currentMillis = millis();

//  if (currentMillis - previousMillis >= interval) {
//    previousMillis = currentMillis;
//    getData();
//  }  

if (Serial.available())  {
    char c = Serial.read();  //gets one byte from serial buffer
    if (c == '\n') {  //looks for end of data packet marker
      Serial.read(); //gets rid of following \r
      Serial.println(readString); //prints string to serial port out
      readString.toCharArray(cString, 20);
         webSocket1.broadcastTXT(cString);
      //do stuff with captured readString
     
      readString=""; //clears variable for new input     
     } 
    else {     
      readString += c; //makes the string readString
    }
  }
  
}
