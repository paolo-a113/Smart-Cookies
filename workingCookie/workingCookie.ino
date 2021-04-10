#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <FS.h>
#include "LittleFS.h" // LittleFS is declared

#include <Adafruit_Sensor.h>
#include "Adafruit_Si7021.h"
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <AsyncElegantOTA.h>


//Adafruit_Si7021 sensor = Adafruit_Si7021();
//Receiver code
char cString[40];
byte chPos = 0;
byte ch = 0;

String readString;

/* Put your SSID & Password */
const char* ssid = "Smart Cookies";  // Enter SSID here
const char* password = "";  //Enter Password here

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer webSocket1 = WebSocketsServer(81);

//unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
//const long interval = 2000;           // interval at which to blink (milliseconds)


void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length)
{

  if (type == WStype_TEXT)
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
    }
    Serial.println();
    //
    //     char bin[16];
    //strcpy(bin, (char *)(payload)); // skip first char
    //Serial.print(bin);
  }


}

void setup() {
  Serial.begin(115200);
  Serial.println("SMART COOKIE - SI7021");
  Serial.println("VERSION 1.1");
  pinMode(LED_BUILTIN, OUTPUT);


  // Initialize SPIFFS
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }


  // PRODUCTION MODE
  WiFi.softAP(ssid);

  // TESTING MODE
  //  WiFi.begin("ATTeEPEtxi","2s?h7j7sw8j=");
  //  while (WiFi.status() != WL_CONNECTED) {
  //    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  //    delay(250);
  //    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  //    delay(250);
  //    Serial.println("Connecting...");
  //  }

  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  Serial.println(WiFi.softAPIP());
  //  Serial.println(WiFi.localIP());


  if (!MDNS.begin("smartcookies")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/index.html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/style.css", "text/css");
  });
  server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/bootstrap.min.js", "text/js");
  });
  server.on("/jquery.easing.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/jquery.easing.min.js", "text/js");
  });
  server.on("/jquery-3.6.0.slim.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/jquery-3.6.0.slim.min.js", "text/js");
  });

  //    AsyncElegantOTA.begin(&server);    // Start ElegantOTA



  server.begin();
  MDNS.addService("http", "tcp", 80);

  webSocket1.begin();
  webSocket1.onEvent(webSocketEvent);

}

void loop() {
  //    AsyncElegantOTA.loop();

  webSocket1.loop();

  MDNS.update();

  //  unsigned long currentMillis = millis();

  //  if (currentMillis - previousMillis >= interval) {
  //    previousMillis = currentMillis;
  //    getData();
  //  }

  if (Serial.available())  {
    char c = Serial.read();  //gets one byte from serial buffer
    if (c == '\n') {  //looks for end of data packet marker
      Serial.read(); //gets rid of following \r
      if (readString.startsWith("$") || readString.startsWith("*") || readString.startsWith("?")) {
        Serial.println(readString); //prints string to serial port out
        readString.toCharArray(cString, 40); //was 40
        webSocket1.broadcastTXT(cString);
      }

      //do stuff with captured readString

      readString = ""; //clears variable for new input
    }
    else {
      readString += c; //makes the string readString
    }
  }

}
