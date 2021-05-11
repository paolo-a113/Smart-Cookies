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
#include <ESP8266httpUpdate.h>


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>


#include <Wire.h>
#include <Adafruit_BMP280.h>

#define TRUC_VERSION "0_0_5"
#define SPIFFS_VERSION "0_5_0"
#define THIS_DEVICE "working"
#define REMOTE_IP "34.208.195.177"
const char * updateUrl = "http://"REMOTE_IP":1880/update/"THIS_DEVICE;

bool actualUpdate(bool sketch = false) {
  Serial.println("CHECKING FOR UPDATE...");
  String msg;
  t_httpUpdate_return ret;

  ESPhttpUpdate.rebootOnUpdate(false);
  if (sketch) {
    ret = ESPhttpUpdate.update(updateUrl, TRUC_VERSION);  // **************** This is the line that "does the business"
  }
  else {
    //    ret = ESPhttpUpdate.httpUpdateSPIFFS(updateUrl, SPIFFS_VERSION);
  }
  if (ret != HTTP_UPDATE_NO_UPDATES) {
    if (ret == HTTP_UPDATE_OK) {

      Serial.printf("UPDATE SUCCESSFUL");
      return true;
    }
    else {
      if (ret == HTTP_UPDATE_FAILED) {

        Serial.printf("UPDATE FAILED");
      }
    }
  }
  return false;
}


//Adafruit_Si7021 sensor = Adafruit_Si7021();
//Receiver code
char cString[40];
char sensorString[40];

byte chPos = 0;
byte ch = 0;

String readString;

/* Put your SSID & Password */
const char* ssid = "Smart Cookies";  // Enter SSID here
const char* password = "123";  //Enter Password here

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer webSocket1 = WebSocketsServer(81);

unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 100;           // interval at which to blink (milliseconds)

Adafruit_BMP280 bmp; // I2C

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
  WiFi.mode(WIFI_STA);
  long timewait = 0;
  long timeout = 10000;

  WiFi.begin("ATTeEPEtxi", "2s?h7j7sw8j=");
  while (WiFi.status() != WL_CONNECTED && timewait < timeout) {
    delay(500);
    timewait = timewait + 500;
    Serial.print(".");
  }
  if (actualUpdate(false)) ESP.restart();
  delay(100);
  if (actualUpdate(true)) ESP.restart();

  WiFi.disconnect();

  Serial.println("SMART COOKIE - SI7021");
  Serial.print("FIRMWARE VERSION: ");
  Serial.println(TRUC_VERSION);
  Serial.print("WEB APP VERSION: ");
  Serial.println(SPIFFS_VERSION );
  pinMode(LED_BUILTIN, OUTPUT);

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */


  // Initialize SPIFFS
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }


  // PRODUCTION MODE
  WiFi.softAP(ssid, password);

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

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    char buff[50];


    //  strcpy(sensorString,"$,0,");
    //  strcat(sensorString,bmp.readTemperature());
    //  strcat(sensorString,",null,");
    //  strcat(sensorString,bmp.readPressure());
    sprintf(buff, "$,0,%f,null,%f", bmp.readTemperature(), bmp.readPressure() / 100.00);
    webSocket1.broadcastTXT(buff);
  }

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
