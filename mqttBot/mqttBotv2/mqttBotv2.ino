//MQTTBot Robot Code - by LeRoy F. Miller, KD8BXP Oct 20, 2023
// This code works best when using the ESP32 board core v1.0.6 and the M5Atom

#include "src/M5Atom/M5Atom.h"
#include "AtomMotion.h"
#include "src/PubSubClient/PubSubClient.h"
#include "src/ArduinoJson/ArduinoJson.h"
#include "WiFi.h"

AtomMotion Atom;
int direction = 0; // 0 = stop, 1 = forward, 2 = reverse, 3 = left, 4 = right

const char* ssid = "ssid";
const char* password = "password";
const char* mqtt_server = "broker.mqtt-dashboard.com";

#define SUBSCRIBETOPIC "somethingDifferent/IN"
#define PUBLISHTOPIC "somethingDifferent/OUT"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String messageIn;

//servo angle range 0 ~ 180
//DC motor speed range -127~127
String x, y;
int x_data = 0;
int y_data = 0;
int speed = 0;
String sentMsg;

void TaskMotion(void *pvParameters){
  while (1)
  {
    
    
        
    vTaskDelay( 100 / portTICK_RATE_MS );
 
    if(direction == 0){
      //stop
      Atom.SetMotorSpeed(1, 0);
      Atom.SetMotorSpeed(2, 0);
      M5.dis.drawpix(0, 0x00FF00); //red
      //
    }
     else if (direction == 1) {
      //forward
      Atom.SetMotorSpeed(1,speed); //100);
      Atom.SetMotorSpeed(2,speed); //100);
      M5.dis.drawpix(0, 0xff0000); //green
      //client.publish(PUBLISHTOPIC, "Moving");
    }
    else if (direction == 2) { 
      //reverse
      Atom.SetMotorSpeed(1,-speed); //-65);
      Atom.SetMotorSpeed(2,-speed); //-65);
      M5.dis.drawpix(0, 0x0000ff); //blue
      //client.publish(PUBLISHTOPIC, "Moving");
    }
    else if (direction == 3) {
      //left
      Atom.SetMotorSpeed(1, -speed); ///-60);
      Atom.SetMotorSpeed(2, speed); //50);
      M5.dis.drawpix(0, 0x00eeee);
      //client.publish(PUBLISHTOPIC, "Moving");
    }
    else if (direction == 4) {
      //right
      Atom.SetMotorSpeed(1, speed); //50);
      Atom.SetMotorSpeed(2, -speed); //-50);
      M5.dis.drawpix(0, 0x00eeee);
      //client.publish(PUBLISHTOPIC, "Moving");
    }
    
  }
  
  //vTaskDelay(1000/ portTICK_RATE_MS);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  messageIn = "";
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    messageIn+=(char)payload[i];
  }
 
  //Serial.println();
  //Serial.println("Color: " + color);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    uint32_t chipid;
    for(int i=0; i<17; i=i+8) {
    chipid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
    char clientid[25];
    snprintf(clientid,25,"MQTTBot-%08X",chipid);
    if (client.connect(clientid,"","", PUBLISHTOPIC, 1, true, "Disconnected")) {
    //if (client.connect(clientid)) {
            client.subscribe(SUBSCRIBETOPIC);
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup(){ 
Serial.begin(115200);
  
  M5.begin(true, true, true);
  Atom.Init();     //sda  25     scl  21 
  M5.dis.drawpix(0, 0x000000);
  xTaskCreatePinnedToCore(
    TaskMotion
    ,  "TaskMotion"   // A name just for humans
    ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  0);
setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 direction = 0;
 x_data = 0;
 y_data = 0;
 delay(3000);
 client.publish(PUBLISHTOPIC, "Ready");
}

void loop() {
    M5.update();
      if (!client.connected()) {
        direction = 0; //estop if MQTT connection is lost
    reconnect();
  }
  client.loop();

   char msgIn[25];
   //Serial.print("Message IN: "); Serial.println(messageIn);
  messageIn.toCharArray(msgIn, messageIn.length()+1);
  //Serial.print("msgIn: "); Serial.println(msgIn);
  sscanf(msgIn, "%d,%d,%d", &x_data, &y_data, &speed);

//Serial.print("X, Y "); Serial.print(x_data); Serial.print(","); Serial.println(y_data);

  float calculatedDirection = atan2(y_data, x_data) * 180.0 / 3.14;
  //Serial.print("calculatedDirection: "); Serial.println(calculatedDirection);
  // 0 = stop, 1 = forward, 2 = reverse, 3 = left, 4 = right

  if (calculatedDirection == 0.00 ) {direction = 0; PubMsg("Stopped      ");} else
  if (calculatedDirection >= -40.00 && calculatedDirection <= 40.00) {direction = 1; PubMsg("Forward      ");} else
  if (calculatedDirection <= -99.99 && calculatedDirection <= 130.00 ) {direction = 2; PubMsg("Reverse     ");} else
  if (calculatedDirection <= -39.99 && calculatedDirection >= -100.00) {direction = 3; PubMsg("Left         ");} else
  if (calculatedDirection >= 40.99 && calculatedDirection <= 131.00) {direction = 4; PubMsg("Right            ");}
  //Serial.print("direction: "); Serial.println(direction);
   
}

void PubMsg(String sendMsg) {
  if (sendMsg == sentMsg) {return;} 
  char msgOut[25];
  sendMsg.toCharArray(msgOut, sendMsg.length()+1);
  client.publish(PUBLISHTOPIC, msgOut);
  sentMsg = sendMsg;
}
