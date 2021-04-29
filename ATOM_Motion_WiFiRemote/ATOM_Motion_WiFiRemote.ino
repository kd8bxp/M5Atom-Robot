/*
    Description: 
    Use ATOM Motion to control 4 channel servo and 2 channel DC motor
    press btn to change the DC motor direction
*/

/*
 * Web controlled robot - LeRoy Milller, KD8BXP April 13, 2021
 * 
 */

#include "src/M5Atom/M5Atom.h"
#include "AtomMotion.h"
#include <DNSServer.h>
#include "src/ESPUI/ESPUI.h"
#include <WiFi.h>

AtomMotion Atom;
int direction = 0; // 0 = stop, 1 = forward, 2 = reverse, 3 = left, 4 = right

const byte DNS_PORT = 53;
//IPAddress apIP(192, 168, 4, 1);
IPAddress apIP(192, 168, 1, 95); //I know this is weird, you will need to get your IP address and then put it here for the DNS Server to work, reupload your sketch. 
DNSServer dnsServer;

const char *ssid = ""; //SSID to connect for website control
const char *password = ""; //no password needed to connect
const char hostname[] = "Robot_1";

xSemaphoreHandle CtlSemaphore;

void padExample(Control sender, int value) {
  switch (value) {
    case P_LEFT_DOWN:
      direction = 3;
      break;
    case P_LEFT_UP:
      direction = 0;
      break;
    case P_RIGHT_DOWN:
      direction = 4;
      break;
    case P_RIGHT_UP:
      direction = 0;
      break;
    case P_FOR_DOWN:
      direction = 1;
      break;
    case P_FOR_UP:
      direction = 0; 
      break;
    case P_BACK_DOWN:
      direction = 2; 
      break;
    case P_BACK_UP:
      direction = 0; 
      break;
    case P_CENTER_DOWN:
      direction = 0; 
      break;
    case P_CENTER_UP:
      direction = 0; 
      break;
  }
  //Serial.print(" ");
  //Serial.println(sender.id);
}


void GetStatus(){
  
  //Serial.printf("Motor Channel %d: %d \n",1,Atom.ReadMotorSpeed(1));
  //Serial.printf("Motor Channel %d: %d \n",2,Atom.ReadMotorSpeed(2));
  char buffer[50];
  sprintf(buffer, "M1: %d, M2: %d",Atom.ReadMotorSpeed(1),Atom.ReadMotorSpeed(2));
  ESPUI.print("Status:", buffer);
}

//servo angle range 0 ~ 180
//DC motor speed range -127~127

void TaskMotion(void *pvParameters){
  while (1)
  {
    
    GetStatus();
        
    vTaskDelay( 100 / portTICK_RATE_MS );
    if(direction == 0){
      //stop
      Atom.SetMotorSpeed(1, 0);
      Atom.SetMotorSpeed(2, 0);
      M5.dis.drawpix(0, 0x00FF00); //red
    }
     else if (direction == 1) {
      //forward
      Atom.SetMotorSpeed(1,100);
      Atom.SetMotorSpeed(2,100);
      M5.dis.drawpix(0, 0xff0000); //green
    }
    else if (direction == 2) { 
      //reverse
      Atom.SetMotorSpeed(1,-100);
      Atom.SetMotorSpeed(2,-100);
      M5.dis.drawpix(0, 0x0000ff); //blue
    }
    else if (direction == 3) {
      //left
      Atom.SetMotorSpeed(1, -60);
      Atom.SetMotorSpeed(2, 50);
      M5.dis.drawpix(0, 0x00eeee);
    }
    else if (direction == 4) {
      //right
      Atom.SetMotorSpeed(1, 50);
      Atom.SetMotorSpeed(2, -50);
      M5.dis.drawpix(0, 0x00eeee);
    }
  }
  //vTaskDelay(1000/ portTICK_RATE_MS);
}

void setup(){ 
Serial.begin(115200);
delay(1000);
  //WiFi.mode(WIFI_AP);
  //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  /*
  #if defined(ESP32)
    WiFi.setHostname(ssid);
  #else
    WiFi.hostname(ssid);
  #endif
  */

  //WiFi.softAP(ssid);

  WiFi.setHostname(hostname);
WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected."); Serial.println(WiFi.localIP());

  delay(1000);
  M5.begin(true, false, true);
  Atom.Init();     //sda  25     scl  21 
  vSemaphoreCreateBinary( CtlSemaphore );
  xTaskCreatePinnedToCore(
    TaskMotion
    ,  "TaskMotion"   // A name just for humans
    ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  0);

     ESPUI.label("Status:", COLOR_TURQUOISE, "Stop");
     ESPUI.pad("Pad without center", false, &padExample, COLOR_CARROT);
     
     dnsServer.start(DNS_PORT, "*", apIP);
     //dnsServer.start(DNS_PORT, "*", WiFi.localIP());
     ESPUI.begin("Robot Control");
}

void loop() {
    M5.update();
    dnsServer.processNextRequest();
    
}
