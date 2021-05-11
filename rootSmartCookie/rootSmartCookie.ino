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
#include <ESP8266httpUpdate.h>

#define TRUC_VERSION "0_0_3"
#define SPIFFS_VERSION "0_5_0"
#define THIS_DEVICE "root"
#define REMOTE_IP "34.208.195.177"

const char * updateUrl = "http://"REMOTE_IP":1880/update/"THIS_DEVICE;

// WiFi Credentials
#define   MESH_PREFIX     "mesh0"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

char serialBuf[100];
int i1 = 1;

String readString;

bool actualUpdate(bool sketch = false) {
  Serial.println("CHECKING FOR UPDATE...");
  String msg;
  t_httpUpdate_return ret;

  ESPhttpUpdate.rebootOnUpdate(false);
  if (sketch) {
    ret = ESPhttpUpdate.update(updateUrl, TRUC_VERSION);  // **************** This is the line that "does the business"
  }
  else {
    //    ret = ESPhttpUpdate.updateSPIFFS(updateUrl, SPIFFS_VERSION);
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
  }
  else if (doc["req"] == "?") {
    String nodeName = doc["name"];
    Serial.printf("?,%u,", from);
    Serial.println(nodeName);
  }
}

void newConnectionCallback(uint32_t nodeId) {
  sendMeshTopology();
}
void changedConnectionCallback() {
  sendMeshTopology();
}
void nodeTimeAdjustedCallback(int32_t offset) {
}
void showNodeList() {
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin("ATTeEPEtxi", "2s?h7j7sw8j=");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (actualUpdate(false)) ESP.restart();
  delay(100);
  if (actualUpdate(true)) ESP.restart();

  WiFi.disconnect();

  Serial.println("[ROOT] SMART COOKIE - SI7021");
  Serial.print("FIRMWARE VERSION: ");
  Serial.println(TRUC_VERSION);
  Serial.print("WEB APP VERSION: ");
  Serial.println(SPIFFS_VERSION );

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );

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

  Serial.println(mesh.getNodeId());
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
        DynamicJsonDocument doc(1024);
        readString.toCharArray(serialBuf, 100); //was 40

        Serial.println(serialBuf); //prints string to serial port out
        DeserializationError error = deserializeJson(doc, serialBuf);

        // Test if parsing succeeds.
        if (error) {
          Serial.println("deserializeJson() failed: ");
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
