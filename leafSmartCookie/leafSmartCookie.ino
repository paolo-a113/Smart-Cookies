
#include "painlessMesh.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_Si7021.h"
//#include <DHT.h>
// WiFi Credentials
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
//Pin Declaration
//#define Relay1 D6
//#define DHTPIN D5
//#define DHTTYPE DHT11     // DHT 11
//DHT dht(DHTPIN, DHTTYPE);
Adafruit_Si7021 sensor = Adafruit_Si7021();

//Variables
//bool relay1_status = 0;
Scheduler userScheduler; 
painlessMesh  mesh;
void sendMessage() ; 
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
void sendMessage()
{
  DynamicJsonDocument doc(1024);
  float h = sensor.readHumidity();
  float t = sensor.readTemperature();
  doc["T"] = t;
  doc["H"] = h;
  String msg ;
  serializeJson(doc, msg);
  mesh.sendBroadcast( msg );
//  Serial.println("from node2");
  Serial.println(msg);
  taskSendMessage.setInterval((TASK_SECOND * 1));
}
// Needed for painless library
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
//  relay1_status = doc["Relay1"];
//  digitalWrite(Relay1, relay1_status);
}
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}
void setup() {
  Serial.begin(115200);
    bool status;
  status = sensor.begin();
  if (!status) {
      Serial.println("COULD NOT FIND SENSOR");
      while (1);
  }
  else {
    Serial.println("SENSOR WORKING");
  }
//  pinMode(Relay1, OUTPUT);
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
    mesh.setContainsRoot(true);

}
void loop() {
  mesh.update();
}
