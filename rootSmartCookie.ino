/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_Si7021.h"
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include "painlessMesh.h"
#include <WiFiClient.h>

// Include the mDNS library
#define BME_SCK 14
#define BME_MOSI 13
#define BME_CS 5
#define BME_MISO 12

#define SEALEVELPRESSURE_HPA (1013.25)

//Scheduler userScheduler; // to control your personal task
//
//#define   MESH_PREFIX     "whateverYouLike"
//#define   MESH_PASSWORD   "123"
//#define   MESH_PORT       5555
//
//painlessMesh  mesh;

//Adafruit_BME280 = sensor(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
//Adafruit_Si7021 sensor = Adafruit_Si7021();

////// Replace with your network credentials
//const char* ssid = "ATTeEPEtxi";
//const char* password = "2s?h7j7sw8j=";

/* Put your SSID & Password */
const char* ssid = "Smart Cookies";  // Enter SSID here
const char* password = "";  //Enter Password here
//
///* Put IP Address details */
//IPAddress local_ip(192,168,1,1);
//IPAddress gateway(192,168,1,1);
//IPAddress subnet(255,255,255,0);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer webSocket1 = WebSocketsServer(81);
WebSocketsServer webSocket2 = WebSocketsServer(82);
WebSocketsServer webSocket3 = WebSocketsServer(83);

unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 100;           // interval at which to blink (milliseconds)

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("ROOT SMART COOKIE");
  
  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

//   Connect to Wi-Fi
  WiFi.softAP(ssid);
//  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//    Serial.println("Connecting to WiFi..");
//  }


  // Print ESP32 Local IP Address
  Serial.println(WiFi.softAPIP());
//Serial.println(mesh.getStationIP());

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });

  if (!MDNS.begin("smartcookies")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
//
//  mesh.setRoot(true);
//  mesh.setContainsRoot(true);
//  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

//  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
//  mesh.onReceive(&receivedCallback);

  // Start server
  server.begin();
  MDNS.addService("http", "tcp", 80);

  webSocket1.begin();
//  webSocket1.onEvent(webSocket1Event);

  webSocket2.begin();
//  webSocket2.onEvent(webSocket2Event);

  webSocket3.begin();
  Serial.println("websocket 3 start");
  webSocket3.onEvent(webSocket3Event);
}
 
void loop(){
  webSocket1.loop();
  webSocket2.loop();
  webSocket3.loop();
//  mesh.update();
  MDNS.update();
}

void webSocket3Event(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  Serial.println("event fire");
  if(type == WStype_TEXT){
    for(int i=0; i<length; i++){
      Serial.print((char) payload[i]);
    }
    Serial.println();
  }
}
