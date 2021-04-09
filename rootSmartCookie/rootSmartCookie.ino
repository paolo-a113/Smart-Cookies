#include "painlessMesh.h"
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
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>


// WiFi Credentials
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

char serialBuf[40];
int i1 = 1;

////  char fromBuf[10];
//  byte chPos = 0;
//  byte ch = 0;
String readString;

/* Put your SSID & Password */
//const char* ssid = "Smart Cookies";  // Enter SSID here
//const char* password = "";  //Enter Password here

// Create AsyncWebServer object on port 80
//AsyncWebServer server(80);
//WebSocketsServer webSocket1 = WebSocketsServer(81);


SimpleList<uint32_t> nodes;

Scheduler userScheduler;
painlessMesh  mesh;
void sendMessage();
void sendMeshTopology();
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskSendMeshTopology( TASK_SECOND * 5 , TASK_FOREVER, &sendMeshTopology );

void sendMessage()
{

  //  sendSingle(destID, "!" + newName)
  ////// Reading Status of Pushbutton
  ////  if (digitalRead(Button1) == HIGH)
  ////    button1_status = !button1_status;
  ////  if (digitalRead(Button2) == HIGH)
  ////    button2_status = !button2_status;
  //// Serializing in JSON Format
  //  DynamicJsonDocument doc(1024);
  ////  doc["Relay1"] = button1_status;
  ////  doc["Relay2"] = button2_status;
  //  String msg ;
  //  serializeJson(doc, msg);
  //    mesh.sendBroadcast( msg );
  //  Serial.println(msg);
  //  taskSendMessage.setInterval((TASK_SECOND * 1));
}

void sendMeshTopology() {
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
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  String Temp = doc["T"];
  String Hum = doc["H"];
  String Pres = doc["P"];

  Serial.print("$,");
  Serial.printf("%u,", from);
  Serial.print(Temp);
  Serial.print(",");
  Serial.print(Hum);
  Serial.print(",");
  Serial.println(Pres);

  //   strcpy(cString, "$,123456789,");
  ////   sprintf(fromBuf, "%u,", from);
  ////   strcpy(fromBuf, "12345678,");
  ////   strcat(cString, fromBuf);
  ////   memset(fromBuf, 0, sizeof(fromBuf));
  //   strcat(cString, Temp.c_str());
  //   strcat(cString, ",");
  //   strcat(cString, Hum.c_str());
  //   strcat(cString, ",");
  //   strcat(cString, Pres.c_str());
  //   webSocket1.broadcastTXT(cString);
  //   memset(cString, 0, sizeof(cString));

}

void newConnectionCallback(uint32_t nodeId) {
  //  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  sendMeshTopology();
}
void changedConnectionCallback() {
  //  Serial.printf("Changed connections\n");
  sendMeshTopology();
}
void nodeTimeAdjustedCallback(int32_t offset) {
  //  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}
void showNodeList() {
}

void setup() {
  Serial.begin(115200);
  //  pinMode(Button1, INPUT);
  //  pinMode(Button2, INPUT);

  //  // Initialize SPIFFS
  //  if(!SPIFFS.begin()){
  //    Serial.println("An Error has occurred while mounting SPIFFS");
  //    return;
  //  }

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  //    mesh.initOTAReceive("bridge");
  //  mesh.stationManual("ATTeEPEtxi", "2s?h7j7sw8j=");



  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask( taskSendMeshTopology );
  taskSendMeshTopology.enable();

  mesh.setRoot(true);
  mesh.setContainsRoot(true);

  //  WiFi.softAP(ssid);
  //  Serial.println(WiFi.softAPIP());

  //  if (!MDNS.begin("smartcookies")) {
  //    Serial.println("Error setting up MDNS responder!");
  //    while (1) {
  //      delay(1000);
  //    }
  //  }
  //    Serial.println("mDNS responder started");
  //
  //  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  //    request->send(SPIFFS, "/index.html");
  //  });

  //      AsyncElegantOTA.begin(&server);    // Start ElegantOTA


  // server.begin();
  //  MDNS.addService("http", "tcp", 80);
  //  Serial.println("mDNS responder started");

  //    webSocket1.begin();


}
void loop() {
  mesh.update();
  //      AsyncElegantOTA.loop();

  //    webSocket1.loop();

  //      MDNS.update();

  if (Serial.available())  {
    char c = Serial.read();  //gets one byte from serial buffer
    if (c == '\n') {  //looks for end of data packet marker
      Serial.read(); //gets rid of following \r
      if (readString.startsWith("{")) {
        StaticJsonDocument<200> doc;
        readString.toCharArray(serialBuf, 40); //was 40

        Serial.println(serialBuf); //prints string to serial port out


        DeserializationError error = deserializeJson(doc, serialBuf);

        // Test if parsing succeeds.
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          //            Serial.println(error.f_str());
          return;
        }


        const char* dest_node = doc["node"];
        const char* sensor = doc["req"];


        Serial.println(dest_node); //prints string to serial port out
        Serial.println(sensor); //prints string to serial port out

        uint32_t destID = strtoul(dest_node, NULL, 10);
        mesh.sendSingle(destID, "?name");
      }
      //      if (serialBuf[0] == "!"){
      //        //parse info
      //
      //do stuff with captured readString

      readString = ""; //clears variable for new input
    }
    else {
      readString += c; //makes the string readString
    }
  }
}
