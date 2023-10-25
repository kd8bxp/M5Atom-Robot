//MQTTBot Joystick controller - by LeRoy F. Miller, KD8BXP Oct 20, 2023

#include "src/M5StickC/M5StickC.h"
#include "src/PubSubClient/PubSubClient.h"
#include "Wire.h"
#include "WiFi.h"

#define JOY_ADDR 0x38
#define adcPin 33

const char* ssid = "SSID";
const char* password = "Password";
const char* mqtt_server = "broker.mqtt-dashboard.com";

#define SUBSCRIBETOPIC "somethingDifferent/OUT"
#define PUBLISHTOPIC "somethingDifferent/IN"

int8_t x_data, y_data, button_data;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String messageIn;
int adcIn;


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
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    uint32_t chipid;
    for(int i=0; i<17; i=i+8) {
    chipid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
    char clientid[25];
    snprintf(clientid,25,"MQTTBot-%08X",chipid);
    if (client.connect(clientid)) {
            client.subscribe(SUBSCRIBETOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
    // initialize the display
    M5.begin();
    Serial.begin(115200);
    Wire.begin(0, 26, 100000UL);
    // clear the background
    M5.Lcd.setRotation(3);
    M5.Lcd.fillRect(0, 0, 80, 160, BLACK);
    M5.Lcd.setCursor(20,0);
    M5.Lcd.print("MQTTBot Controler");
    setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
   xTaskCreatePinnedToCore( TaskJoyStick, "TaskJoyStick", 4096, NULL, 2, NULL, 0);
}

void TaskJoyStick (void *pvParameters) {
 while (1) {
  Wire.beginTransmission(JOY_ADDR);
    Wire.write(0x02);
    Wire.endTransmission();
    Wire.requestFrom(JOY_ADDR, 3);
    if (Wire.available()) {
        x_data      = Wire.read();
        y_data      = Wire.read();
        button_data = Wire.read();
        
    }
    vTaskDelay( 100 / portTICK_RATE_MS );
 }  
}

void loop() {
  //client.loop();
   /* Wire.beginTransmission(JOY_ADDR);
    Wire.write(0x02);
    Wire.endTransmission();
    Wire.requestFrom(JOY_ADDR, 3);
    if (Wire.available()) {
        x_data      = Wire.read();
        y_data      = Wire.read();
        button_data = Wire.read();
        
    }*/

       M5.update();
      if (!client.connected()) {
            reconnect();
  }
  client.loop();

  adcIn = map(analogRead(adcPin), 0, 4095, 50, 127);
 
  M5.Lcd.setCursor(10,20);
  M5.Lcd.printf("X: %d   ", -x_data); //negitive because of orintation of device
  M5.Lcd.setCursor(60,20);
  M5.Lcd.printf("Y: %d   ", y_data);
  M5.Lcd.setCursor(0,35); 
  M5.Lcd.printf("But: %d", button_data);
  M5.Lcd.setCursor(40, 35);
  M5.Lcd.printf("SPD: %d   ", adcIn);
  //client.loop();
  M5.Lcd.setCursor(0, 47);
  M5.Lcd.print(messageIn);
  char cstr[20];
  sprintf(cstr, "%d,%d,%d", -x_data, y_data, adcIn);
  client.publish(PUBLISHTOPIC, cstr);
  //Serial.print("Atan2: "); Serial.print(atan2(y_data,-x_data)* 180 / 3.14);
  //Serial.print(" Tan: "); Serial.println(tan(atan2(y_data,-x_data))*10);
 // client.loop();
}
