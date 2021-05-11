//#include <WiFi.h>
//#include <Hash.h>
//#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <FS.h>
#include "SPIFFS.h" // SPIFFS is declared

#include <Adafruit_Sensor.h>
#include "Adafruit_Si7021.h"
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <AsyncElegantOTA.h>

#include "painlessMesh.h"
//#include <ArduinoJson.h>

// WiFi Credentials
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555



//Adafruit_Si7021 sensor = Adafruit_Si7021();
//Receiver code
char cString[40];
byte chPos = 0;
byte ch = 0;

//char serialBuf[40];
int i1 = 1;

String readString;

SimpleList<uint32_t> nodes;


/* Put your SSID & Password */
const char* ssid = "Smart Cookies";  // Enter SSID here
const char* password = "123";  //Enter Password here

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer webSocket1 = WebSocketsServer(81);

//unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
//const long interval = 2000;           // interval at which to blink (milliseconds)

Scheduler userScheduler;
painlessMesh  mesh;
void sendMessage();
void sendMeshTopology();
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskSendMeshTopology( TASK_SECOND * 5 , TASK_FOREVER, &sendMeshTopology );

void sendMessage() {}
//
void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length)
{
Serial.println(xPortGetCoreID());

  if (type == WStype_TEXT)
  {
    //    for (int i = 0; i < length; i++) {
    //      Serial.print((char) payload[i]);
    //    }
    //    Serial.println();

    DynamicJsonDocument doc(1024);
    //    readString.toCharArray(serialBuf, 100); //was 40
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
//      Serial.println("deserializeJson() failed: ");
      //            Serial.println(error.f_str());
      return;
    }

    Serial.println("probe1");

    const char* dest_node = doc["node"];
    const char* req = doc["req"];
    const char* ren = doc["ren"];

    Serial.println("probe2");

    //        Serial.println(dest_node); //prints string to serial port out
    //        Serial.println(sensor); //prints string to serial port out
    Serial.println("probe3");
    uint32_t destID = strtoul(dest_node, NULL, 10);
    Serial.println("probe4");

    StaticJsonDocument<200> reqJson;
    Serial.println("probe5");

    reqJson["req"] = req;
    reqJson["ren"] = ren;
    String msg;
    serializeJson(reqJson, msg);
    mesh.sendSingle(destID, msg);


    //
    //     char bin[16];
    //strcpy(bin, (char *)(payload)); // skip first char
    //Serial.print(bin);
  }


}

void sendMeshTopology() {
  Serial.println(xPortGetCoreID());

  nodes = mesh.getNodeList();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  Serial.print('*');
  while (node != nodes.end()) {
    Serial.printf(",%u", *node);
    node++;
  }
  Serial.println();

  //  Serial.println('*'+mesh.subConnectionJson());
  taskSendMessage.setInterval((TASK_SECOND * 5));

}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.println(xPortGetCoreID());

  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  char id[12];
  utoa( from,
        id,
        10 );

  if (doc["req"] == "$") {
    String Temp = doc["T"];
    String Hum = doc["H"];
    String Pres = doc["P"];

    Serial.printf("$,%u,", from);
    Serial.print(Temp);
    Serial.print(",");
    Serial.print(Hum);
    Serial.print(",");
    Serial.println(Pres);

    strcpy (cString, "$,");
    strcat(cString, id);
    strcat(cString, ",");
    strcat(cString, Temp.c_str());
    strcat(cString, ",");
    strcat(cString, Hum.c_str());
    strcat(cString, ",");
    strcat(cString, Pres.c_str());
  }
  else if (doc["req"] == "?") {
    String nodeName = doc["name"];

    strcpy (cString, "?,");
    strcat(cString, id);
    strcat(cString, ",");
    strcat(cString, nodeName.c_str());
  }
  webSocket1.broadcastTXT(cString);

}

void newConnectionCallback(uint32_t nodeId) {
  
  //  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  sendMeshTopology();
}
void changedConnectionCallback() {
  //  Serial.printf("Changed connections\n");
  sendMeshTopology();
}

void setup() {
  Serial.println(xPortGetCoreID());

  Serial.begin(115200);
  Serial.println("SMART COOKIE - SI7021");
  Serial.println("VERSION 1.1");
  //  pinMode(LED_BUILTIN, OUTPUT);


  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  //    mesh.initOTAReceive("bridge");
  //  mesh.stationManual("ATTeEPEtxi", "2s?h7j7sw8j=");



  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  userScheduler.addTask( taskSendMeshTopology );
  taskSendMeshTopology.enable();

  mesh.setRoot(true);
  mesh.setContainsRoot(true);

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
  //  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  Serial.println(WiFi.softAPIP());
  //  Serial.println(WiFi.localIP());

  //
  if (!MDNS.begin("smartcookies")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/bootstrap.min.js", "text/js");
  });
  server.on("/jquery.easing.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/jquery.easing.min.js", "text/js");
  });
  server.on("/jquery-3.6.0.slim.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/jquery-3.6.0.slim.min.js", "text/js");
  });

  //    AsyncElegantOTA.begin(&server);    // Start ElegantOTA



  server.begin();
  //    MDNS.addService("http", "tcp", 80);

  webSocket1.begin();
  webSocket1.onEvent(webSocketEvent);

}

void loop() {
  //    AsyncElegantOTA.loop();
  
  mesh.update();

  webSocket1.loop();

  //  MDNS.update();

}
