//Voice Controlled Bot Robot Code - by LeRoy F. Miller, KD8BXP Nov 2, 2023
//for best results use ESP32 board core version 1.0.6 (LEDs work with this version)

//Also works with the Gesture Controller sketch without any modifications Nov 25, 2023

#include "src/M5Atom/M5Atom.h"
#include "AtomMotion.h"
#include "WiFi.h"
#include <esp_now.h>

#define CHANNEL 1

AtomMotion Atom;
int direction = 0; // 0 = stop, 1 = forward, 2 = reverse, 3 = left, 4 = right
int speed = 50;
uint8_t controllData;
uint8_t previousData;

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
  }
}


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
 WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
 direction = 0;
 controllData = 0;
 delay(3000);
 
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  Serial.println("");
  controllData = *data;
}


void loop() {
    M5.update();
  
  // 0 = stop, 1 = forward, 2 = reverse, 3 = left, 4 = right
  if (previousData != controllData) {
if (controllData > 2 && controllData <= 10 ||controllData == 22) {
  
  if (controllData == 5) {direction = 0;} else
  if (controllData == 22) {direction = 1;} else
  if (controllData == 8) {direction = 2;} else
  if (controllData == 6) {direction = 3;} else
  if (controllData == 7) {direction = 4;} else
  if (controllData == 9) {speed = speed + 5; 
      if (speed > 80) {speed = 80;}
  } else
  if (controllData == 10) {speed = speed - 5;
      if (speed < 50) {speed = 50;}
  } else
  { direction = 0; } //fail to stop }
}
  Serial.print("controllData: "); Serial.println(controllData);
  Serial.print("direction: "); Serial.println(direction);
  Serial.print("Speed: "); Serial.println(speed);
  previousData = controllData; 
}
}

/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Slave >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/
