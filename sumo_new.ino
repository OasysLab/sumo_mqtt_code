String ID = "2";
#define Accept_Toppic "sumo/callback/output/2"
#define Reject_Toppic "sumo/callback/reject/2"
#define Output_Toppic "sumo/output/2"
#include <SevenSegmentTM1637.h>
const byte PIN_CLK = D0;   // define CLK pin (any digital pin)
const byte PIN_DIO = D1;   // define DIO pin (any digital pin)
SevenSegmentTM1637    display(PIN_CLK, PIN_DIO);
const int buttonPinAccept = D2;  
const int buttonPinReject = D7;
const long interval = 1000;
unsigned long previousMillis_Accept = 0;
unsigned long previousMillis_Reject = 0;
unsigned long currentMillis = 0;
void Accept();
void Reject();
///MQTT///////////////////////////////////////
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define WLAN_SSID       "sumowifi"
#define WLAN_PASS       "123456789"
#define MQTT_SERVER      "202.28.24.69"
#define MQTT_SERVERPORT  1883
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT);
Adafruit_MQTT_Subscribe Accept_feed = Adafruit_MQTT_Subscribe(&mqtt, Accept_Toppic, MQTT_QOS_1);
Adafruit_MQTT_Subscribe Reject_feed = Adafruit_MQTT_Subscribe(&mqtt, Reject_Toppic);
Adafruit_MQTT_Publish OutputPub = Adafruit_MQTT_Publish(&mqtt,Output_Toppic);
void AcceptCallback(char *data, uint16_t len) {
  Serial.print("Callback accept");
  display.clear();
  display.print(data);
}
void RejectCallback(char *data, uint16_t len) {
  Serial.print("Callback reject");
  display.clear();
  display.print(data);
}
void setup() {
  currentMillis = millis();
  Serial.begin(9600);
  delay(10);
  display.begin();
  display.print("WIFI");
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  display.print("MQTT");
  Serial.println();  
  Accept_feed.setCallback(AcceptCallback);
  Reject_feed.setCallback(RejectCallback);
  mqtt.subscribe(&Accept_feed);
  mqtt.subscribe(&Reject_feed);
  attachInterrupt(digitalPinToInterrupt(buttonPinAccept), Accept, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPinReject), Reject, FALLING);
  
}
void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}
void Accept(){
  currentMillis = millis();
  if (currentMillis - previousMillis_Accept >= interval){
    OutputPub.publish(1);
    previousMillis_Accept = currentMillis;
  }
}
void Reject(){
  currentMillis = millis();
  if (currentMillis - previousMillis_Accept >= interval){
    OutputPub.publish(0);
    previousMillis_Accept = currentMillis;
  }
}
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  display.clear();
  display.print("MQTT");
  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(3000 );  // wait 10 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
  display.clear();
  display.print("RDY");
}

