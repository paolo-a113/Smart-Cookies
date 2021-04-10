
#include "painlessMesh.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_Si7021.h"
#include <LittleFS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>


//#include <DHT.h>
// WiFi Credentials
#define   MESH_PREFIX     "mesh0"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
//Pin Declaration
//#define Relay1 D6
//#define DHTPIN D5
//#define DHTTYPE DHT11     // DHT 11
//DHT dht(DHTPIN, DHTTYPE);
Adafruit_Si7021 sensor = Adafruit_Si7021();

char receiveBuf[40];

//Variables
//bool relay1_status = 0;
Scheduler userScheduler;
painlessMesh  mesh;
void sendMessage() ;
Task taskSendMessage( TASK_SECOND * 5 , TASK_FOREVER, &sendMessage );

// JSON CONFIG FILE STUFF

//bool getName(uint32_t id) {
//  File configFile = LittleFS.open("/config.json", "r");
//  if (!configFile) {
//    saveConfig("My Sensor Node");
//    Serial.println("Failed to open config file");
//    loadConfig();
//    return false;
//  }
//
//  size_t size = configFile.size();
//  if (size > 1024) {
//    Serial.println("Config file size is too large");
//    return false;
//  }
//
//  // Allocate a buffer to store contents of the file.
//  std::unique_ptr<char[]> buf(new char[size]);
//
//  // We don't use String here because ArduinoJson library requires the input
//  // buffer to be mutable. If you don't use ArduinoJson, you may as well
//  // use configFile.readString instead.
//  configFile.readBytes(buf.get(), size);
//
//  StaticJsonDocument<200> doc;
//  auto error = deserializeJson(doc, buf.get());
//  if (error) {
//    Serial.println("Failed to parse config file");
//    return false;
//  }
//
//  const char* nodeName = doc["nodeName"];
//  const char* accessToken = doc["accessToken"];
//
//  // Real world application would store these values in some variables for
//  // later use.
//
//  mesh.sendSingle(id, nodeName);
//
//  Serial.print("Loaded nodeName: ");
//  Serial.println(nodeName);
//  Serial.print("Loaded accessToken: ");
//  Serial.println(accessToken);
//  return true;
//}
//
String getName() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    setName("My Sensor Node");
//    setMesh("mesh0");

    Serial.println("Failed to open config file");
    return "";
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return "";
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return "";
  }

  const char* nodeName = doc["nodeName"];

  // Real world application would store these values in some variables for
  // later use.

  Serial.print("Loaded nodeName: ");
  Serial.println(nodeName);

  return nodeName;
}

//
bool setName(String nodeName) {
  StaticJsonDocument<200> doc;
  doc["nodeName"] = nodeName;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}

void sendMessage()
{
  DynamicJsonDocument doc(1024);
  float h = sensor.readHumidity();
  float t = sensor.readTemperature();
  doc["req"] = "$";
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
  StaticJsonDocument<200> received;
  DeserializationError error = deserializeJson(received, msg.c_str());

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    //            Serial.println(error.f_str());
    return;
  }

  if (received["req"] == "!" || received["req"] == "?") {
    if (received["req"] == "!") {
      setName(received["ren"]);
    }

    DynamicJsonDocument doc(1024);
    doc["req"] = "?";
    doc["name"] = getName();
    String msg ;
    serializeJson(doc, msg);
    mesh.sendSingle(from, msg);

  }


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

    delay(1000);
  Serial.println("Mounting FS...");

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  getName();
  
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
