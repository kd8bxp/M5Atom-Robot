/*
  Modified M5Rover code to work with the M5Atom Motion
  LeRoy Miller, KD8BXP April 2021
*/

#include "src/M5Atom/M5Atom.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "AtomMotion.h"

const char *ssid = "M5AP";
const char *password = "77777777";

//TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
WiFiServer server(80);

WiFiUDP Udp1;

AtomMotion Atom;
bool direction = true;
uint8_t SendBuff[9] = {0xAA, 0x55,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0xee};



int16_t speed_buff[4] = {0};
int8_t speed_sendbuff[4] = {0};
uint32_t count = 0;
uint8_t IIC_ReState = I2C_ERROR_NO_BEGIN;


xSemaphoreHandle CtlSemaphore;


//DC motor speed range -127~127

void TaskMotion(void *pvParameters){
  while (1)
  {
  
    Atom.SetMotorSpeed(1, speed_buff[0]);
    Atom.SetMotorSpeed(2, speed_buff[1]);
    vTaskDelay( 100 / portTICK_RATE_MS );
    M5.dis.drawpix(0, 0xff0000);
  
  }
}

void setup()
{
  M5.begin(true, false, true);
  Atom.Init();     //sda  25     scl  21 
  vSemaphoreCreateBinary( CtlSemaphore );
  xTaskCreatePinnedToCore(
    TaskMotion
    ,  "TaskMotion"   // A name just for humans
    ,  4096  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  0);

    uint64_t chipid = ESP.getEfuseMac();
    String str = ssid + String((uint32_t)(chipid >> 32), HEX);

    Serial.begin(115200);
    //Set device in STA mode to begin with
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    WiFi.softAP(str.c_str(), password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.begin();

    Udp1.begin(1003);
}


uint8_t Setspeed(int16_t Vtx, int16_t Vty, int16_t Wt)
{
    Wt = (Wt > 100) ? 100 : Wt;
    Wt = (Wt < -100) ? -100 : Wt;

    Vtx = (Vtx > 100) ? 100 : Vtx;
    Vtx = (Vtx < -100) ? -100 : Vtx;
    Vty = (Vty > 100) ? 100 : Vty;
    Vty = (Vty < -100) ? -100 : Vty;

    Vtx = (Wt != 0) ? Vtx * (100 - abs(Wt)) / 100 : Vtx;
    Vty = (Wt != 0) ? Vty * (100 - abs(Wt)) / 100 : Vty;

    speed_buff[0] = Vty - Vtx - Wt;
    speed_buff[1] = Vty + Vtx + Wt;
    speed_buff[3] = Vty - Vtx + Wt;
    speed_buff[2] = Vty + Vtx - Wt;

}  

void loop()
{
    int udplength = Udp1.parsePacket();
    if (udplength)
    {
        char udodata[udplength];
        Udp1.read(udodata, udplength);
        IPAddress udp_client = Udp1.remoteIP();
        if ((udodata[0] == 0xAA) && (udodata[1] == 0x55) && (udodata[7] == 0xee))
        {
            for (int i = 0; i < 8; i++)
            {
                Serial.printf("%02X ", udodata[i]);
            }
            Serial.println();
            if (udodata[6] == 0x01)
            {
                IIC_ReState = Setspeed(udodata[3] - 100, udodata[4] - 100, udodata[5] - 100);
            }
            else
            {
                IIC_ReState = Setspeed(0, 0, 0);
            }
        }
        else
        {
            IIC_ReState = Setspeed(0, 0, 0);
        }
    }
    count++;
    if (count > 100)
    {
        count = 0;

    }
}
