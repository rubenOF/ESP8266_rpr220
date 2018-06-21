#include "ESP8266WiFi.h"
#include <PubSubClient.h>

#define IR_INPUT_PIN A0
#define IR_LED_PIN D1

const char* wifi_ssid = "IOT";
const char* wifi_password = "AardvarkBadgerHedgehog";

const char* mqtt_topic = "esp8266_arduino_out";
const char* mqtt_server = "home.hofman.frl";
const int8_t mqtt_idx = 124;

int16_t _state_low = 100;
int16_t _state_high = 600;
int16_t _state_threshold = 50;

int16_t _state = _state_low;

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

  int16_t val = readRPR220();
  checkState(val);
}

void loop() {
  int16_t val = readRPR220();

  Serial.println(val);

  if (checkState(val)) {
    sendBasicInfo(val);
//    sendDomoticz();
  }

  // 1000ms / (40ms + 5ms + 5ms) = 20 times/second.
  delay(40);
}

bool checkState(int16_t val) {
  int16_t newState = (_state == _state_low) ? _state_high : _state_low;

  if (val > (newState - _state_threshold)
    && val < (newState + _state_threshold)) {

    Serial.print("Switching state from ");
    Serial.print(_state);
    Serial.print(" to ");
    Serial.println(newState);

    _state = newState;

    return true;
  }

  return false;
}

int16_t readRPR220() {
  uint16_t val = 0;

  digitalWrite(IR_LED_PIN, LOW);
  delay(5);
  val = analogRead(IR_INPUT_PIN);

  digitalWrite(IR_LED_PIN, HIGH);
  delay(5);
  val = analogRead(IR_INPUT_PIN) - val;
  val = 0 - val;

  return (val < 0) ? 0 : val;
}

void sendBasicInfo(int16_t val) {
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
