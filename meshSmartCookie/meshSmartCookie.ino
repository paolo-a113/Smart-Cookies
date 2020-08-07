/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include <SPIFFS.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <FS.h>
#endif

#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Adafruit_Si7021.h"



// assign the ESP8266 pins to arduino pins
//#define D1 5
//#define D2 4
//#define D4 2
//#define D3 0

// assign the SPI bus to pins
#define BME_SCK 14
#define BME_MOSI 13
#define BME_CS 5
#define BME_MISO 12

#define SEALEVELPRESSURE_HPA (1013.25)

#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

//Adafruit_BME280 = sensor(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
Adafruit_Si7021 sensor = Adafruit_Si7021();


// If client is connected to this node, set as root
if(WiFi.softAPgetStationNum()){
   mesh.setRoot(true);
  //This and all other mesh should ideally know the mesh contains a root
  mesh.setContainsRoot(true);
}


// Replace with your network credentials
//const char* ssid = "ATTeEPEtxi";
//const char* password = "2s?h7j7sw8j=";

/* Put your SSID & Password */
const char* ssid = "Smart Cookies";  // Enter SSID here
const char* password = "";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readBME280Temperature() {
  // Read temperature as Celsius (the default)
  float t = sensor.readTemperature();
  // Convert temperature to Fahrenheit
  //t = 1.8 * t + 32;
  if (isnan(t) || t==998 ) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
   // Serial.println(t);
    return String(t);
  }
}

String readBME280Humidity() {
  float h = sensor.readHumidity();
  if (isnan(h) || h==998 ) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
   // Serial.println(h);
    return String(h);
  }
}

void sendMessage() ; // Prototype so PlatformIO doesn't complain

//Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );


void sendMessage() {
  String msg = "Node: ";
  msg += mesh.getNodeId();
  msg += " Time: ";
  mesh.sendBroadcast("1");
//  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

void receivedCallback( uint32_t rootNodeID, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  mesh.sendSingle(rootNodeID, msg);

}


void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);


  Serial.println(F("BME280 test"));

    bool status;
    
    // default settings
    status = sensor.begin();
    Serial.println(status);
    Serial.println(SDA);
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    
    Serial.println("-- Default Test --");

    Serial.println();


  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.softAP(ssid);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
//  
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//    Serial.println("Connecting to WiFi..");
//  }
//
//  // Print ESP32 Local IP Address
//  Serial.println(WiFi.localIP());

  // Route for root / web page

    server.on("/js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Chart.min.js", "text/javascript");
  });
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });

//    server.on("/material.cyan-light_blue.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
//    request->send(SPIFFS, "/material.cyan-light_blue.min.css", "text/css");
//  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Temperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Humidity().c_str());
  });


  // Start server
  server.begin();
}
 
void loop(){
  Serial.println(WiFi.softAPgetStationNum());
}
