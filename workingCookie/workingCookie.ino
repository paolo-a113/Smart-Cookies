#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
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

#define TRUC_VERSION "0_0_9"
#define SPIFFS_VERSION "0_5_0"
#define THIS_DEVICE "working"
#define REMOTE_IP "34.208.195.177"
const char * updateUrl = "http://"REMOTE_IP":1880/update/"THIS_DEVICE;

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

bool actualUpdate(bool sketch = false) {
  Serial.println("CHECKING FOR UPDATE...");
  WiFiClient client;

  // The line below is optional. It can be used to blink the LED on the board during flashing
  // The LED will be on during download of one buffer of data from the network. The LED will
  // be off during writing that buffer to flash
  // On a good connection the LED should flash regularly. On a bad connection the LED will be
  // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
  // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

  // Add optional callback notifiers
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);

  String msg;
  t_httpUpdate_return ret;
  
  ESPhttpUpdate.rebootOnUpdate(false);

  if (sketch) {
    ret = ESPhttpUpdate.update(client, updateUrl, TRUC_VERSION);  // **************** This is the line that "does the business"
  }
  else {
//    ret = ESPhttpUpdate.updateFS(client, updateUrl, SPIFFS_VERSION);
  }

  //    t_httpUpdate_return ret = ESPhttpUpdate.update(client, updateUrl, TRUC_VERSION);
  // Or:
  //t_httpUpdate_return ret = ESPhttpUpdate.update(client, "server", 80, "file.bin");
 
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
//  if (actualUpdate(true)) ESP.restart();

  //  WiFi.disconnect();

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
  //  byte best_channel = channel_chooser();
  //
  //  while (best_channel == 255) {         // while the scan isn't finished
  //    best_channel = channel_chooser();
  //    delay(0);                           // needed to avoid a WDT trigger, yields the process time to background esp tasks
  //  }
  //
  //  Serial.printf("Choosing WiFi Channel %d\n", best_channel);
  //
  //  WiFi.softAP(ssid, password, best_channel, false, 1);

  // TESTING MODE
  //  WiFi.begin("ATTeEPEtxi", "2s?h7j7sw8j=");
  //  while (WiFi.status() != WL_CONNECTED) {
  //    //      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  //    //      delay(250);
  //    //      digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  //    //      delay(250);
  //    Serial.println("Connecting...");
  //  }

  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  //  Serial.println(WiFi.softAPIP());
  //    Serial.println(WiFi.localIP());


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

byte channel_chooser() {
  int scan_result = WiFi.scanComplete();

  if (scan_result < 0) {       //  if not scanning return 255 and launch scan
    WiFi.scanNetworks(true);
    return 255;

  } else if (scan_result == 0) { // return 1 if found no networks

    return 1;

  } else {

    // create the list of channel weights

    int channels[14][2];

    for (byte a = 1; a < 14; a++) {
      channels[a][0] = a;
      channels[a][1] = 0;
    }

    for (byte a = 0; a < scan_result; a++) {
      channels[WiFi.channel(a)][0] = WiFi.channel(a);
      channels[a][1] += 100 + WiFi.RSSI(a);
    }

    // adding adjacent channels to channel weight

    for (byte a = 1; a < 14; a++) {
      if (a > 1) {
        channels[a][1] += channels[a - 1][1] / 2;
      }
      if (a > 2) {
        channels[a][1] += channels[a - 2][1] / 4;
      }
      if (a < 13) {
        channels[a][1] += channels[a + 1][1] / 2;
      }
      if (a < 12) {
        channels[a][1] += channels[a + 2][1] / 4;
      }
    }

    // bubble sort the list by weight

    bool swapped = 1;
    while (swapped) {
      for (byte a = 1; a < 13; a++) {
        if (a == 1) {
          swapped = 0;
        }
        if (channels[a][1] > channels[a + 1][1]) {
          byte buf = channels[a][0];
          channels[a][0] = channels[a + 1][0];
          channels[a + 1][0] = buf;

          buf = channels[a][1];
          channels[a][1] = channels[a + 1][1];
          channels[a + 1][1] = buf;
          swapped = 1;
        }
      }
    }

    // print the sorted results

    for (byte a = 1; a < 14; a++) {
      Serial.print(channels[a][0]);
      Serial.print(" - ");
      Serial.println(channels[a][1]);
    }

    WiFi.scanDelete();

    if (channels[1][0] == 12 || channels[1][0] == 13) {
      if (channels[2][0] == 12 || channels[2][0] == 13) {

        return channels[3][0];
      }
      else {
        return channels[2][0];
      }
    }
    return channels[1][0];
  }
}
