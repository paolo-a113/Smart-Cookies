#include "painlessMesh.h"
// WiFi Credentials
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// Create AsyncWebServer object on port 80
SimpleList<uint32_t> nodes;

Scheduler userScheduler; 
painlessMesh  mesh;
void sendMessage(); 
void sendMeshTopology();
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskSendMeshTopology( TASK_SECOND * 5 , TASK_FOREVER, &sendMeshTopology );

void sendMessage() 
{
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

//  Serial.println("From node1");
//   Serial.println("Temperature:");
Serial.print("$,");
Serial.printf("%u,",from);
   Serial.print(Temp);
    Serial.print(",");
   Serial.print(Hum);
     Serial.print(",");
   Serial.println(Pres);
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





//  Serial.println(WiFi.softAPIP());

 
//  if (!MDNS.begin("smartcookies")) {
//    Serial.println("Error setting up MDNS responder!");
//    while (1) {
//      delay(1000);
//    }
//  }
//  Serial.println("mDNS responder started");
  
//  MDNS.addService("http", "tcp", 80);

}
void loop() {
  mesh.update();
//      MDNS.update();
}
