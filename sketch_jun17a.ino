#include "ESP8266WiFi.h"
#include <PubSubClient.h>

#define IR_INPUT_PIN A0
#define IR_LED_PIN D1
#define THRESHOLD 50

#define THRESHOLD_LOW 110
#define THRESHOLD_HIGH 350

const char* wifi_ssid = "IOT";
const char* wifi_password = "AardvarkBadgerHedgehog";

const char* mqtt_topic = "esp8266_arduino_out";
const char* mqtt_server = "home.hofman.frl";
const int8_t mqtt_idx = 124;

int8_t state = THRESHOLD_LOW;

WiFiClient espClient;
PubSubClient client(espClient);

void setup(){
 Serial.begin(9600); 

  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 

  client.setServer(mqtt_server, 1883);
  client.connect("myClientID");
  Serial.println(client.state());

  char* message = "Hello World";
  int8_t length = strlen(message);
  client.publish(mqtt_topic,(byte*)message, length, false);

  pinMode(IR_INPUT_PIN, INPUT);
  pinMode(IR_LED_PIN, OUTPUT);

  int8_t val = readRPR220();
  checkState(val);
}


void loop() {
  int8_t val = readRPR220();

  Serial.println(val);

  if (checkState(val)) {
    sendBasicInfo(val);
//    sendDomoticz();
  }

  delay(80);
}


bool checkState(int8_t val) {
  int8_t newState = (state == THRESHOLD_LOW) ? THRESHOLD_HIGH : THRESHOLD_LOW;

  if (val > (newState - THRESHOLD)
    && val < (newState + THRESHOLD)) {

    Serial.print("Switching state from " + state);
    Serial.println(" to " + newState);
    state = newState;

    return true;
  }

  return false;
}


int8_t readRPR220() {
  uint8_t val = 0;

  digitalWrite(IR_LED_PIN, LOW);
  delay(10);
  val = analogRead(IR_INPUT_PIN);

  digitalWrite(IR_LED_PIN, HIGH);
  delay(10);
  val = analogRead(IR_INPUT_PIN) - val;
  val = 0 - val;

  return (val < 0) ? 0 : val;
}

void sendBasicInfo(int8_t val) {
  String payload = "{\"val\":";
  payload += val;
  payload += "}";

  client.publish(mqtt_topic, payload.c_str());
}

void sendDomoticz() {
  String payload = "{\"idx\":";
  payload += mqtt_idx;
  payload += ", \"nvalue\" : 0, \"svalue\" : \"";
  payload += "0.5";
  payload += "\"}";

  client.publish("domoticz/in", payload.c_str());
}
