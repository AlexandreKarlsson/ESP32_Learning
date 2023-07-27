#include <Arduino.h>
#include <ESPNowW.h>
#include <WiFi.h>
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "pb_tools/pb_tools_msg.h"
#include "pb_tools/pb_tools_msg.c"
#include "hardware/indicator.h"
#include "hardware/indicator.cpp"
#include "espnow_tools/espnow_tool.h"
#include "espnow_tools/espnow_tool.cpp"

#define ESP_NUMBER 2

uint8_t SERVER_MAC_ADDRESS[] = {0xe0, 0x5a, 0x1b, 0xd3, 0x0a, 0x14};
IPAddress serverAdress;
unsigned long previousTime = 0;
const unsigned long interval = 3000;
unsigned long previousTime2 = 0;
const unsigned long interval2 = 2000;
bool serverisavailable=false;
bool serverwasavailable=true;
int serverPort;
bool received_ACK;
bool isconnected = false;
const int sensorPin = 34;
WiFiClient client;


void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  Serial.println("ESPNOW message received");
  //if (data_len == sizeof(ServerMessage)) {
    ServerMessage messageReceived;
    memcpy(&messageReceived, data, sizeof(messageReceived));
    // Connecter au WiFi avec les informations reçues du serveur
    //Serial.print("SSID : ");
    //Serial.println(messageReceived.ssid);
    //Serial.print("Password : ");
    //Serial.println(messageReceived.password);
    //Serial.print("Server adress : ");
    //Serial.println(messageReceived.serveradress);
    //Serial.println();
    deserializeIPAddress(messageReceived.serveradress, serverAdress);
    serverPort = messageReceived.serverport;
    // Connecter au WiFi avec les informations du serveur
    WiFi.begin(messageReceived.ssid, messageReceived.password);
    serverisavailable=true;
    serverwasavailable=true;
  //}
}




void checkWifi() {
  if (WiFi.status() == WL_CONNECTED && serverisavailable) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    WiFi.reconnect();
  }
  if(!serverisavailable)
  {
    unsigned long currentTime = millis();
    if(serverwasavailable){
      previousTime=currentTime;
      serverwasavailable=false;
    }
    
    if (currentTime - previousTime >= interval)  
    {
      Serial.print("ESPNOW message :");
      ClientMessage messageToSend;
      WiFi.macAddress(messageToSend.mac);
      messageToSend.connected = false;
      uint8_t serializedData[sizeof(messageToSend)];
      memcpy(serializedData, &messageToSend, sizeof(messageToSend));
      ESPNow.send_message(SERVER_MAC_ADDRESS, serializedData, sizeof(serializedData));
      previousTime=currentTime;
      Serial.println("send");
    }
  }  
}

void sendCommand(int value) {
  if (!client.connect(serverAdress, serverPort)) 
  {
    serverisavailable=false;
    blinkLEDERROR();
    Serial.println("Server is not available");
    return;
  }
  // Create a buffer for the serialized message
  uint8_t buffer[256];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  // Serialize the message
  if (!encodeMessage_Command(&stream, value,ESP_NUMBER)) {
    blinkLEDERROR();
    client.stop();
    Serial.println("ERROR encoding command");
    return;
  }
  // Send the serialized message via WiFi
  client.write(buffer, stream.bytes_written);
  client.flush();
  // Wait for an ACK
  uint8_t ackBuffer[256];
  received_ACK = false;
  while (client.connected() && !received_ACK) {
    if (client.available()) {
      int bytesRead = client.readBytes(ackBuffer, sizeof(ackBuffer));
      // Decode the ACK message
      Message ackMessage = Message_init_default;
      pb_istream_t ackStream = pb_istream_from_buffer(ackBuffer, bytesRead);
      if (pb_decode(&ackStream, Message_fields, &ackMessage)) {
        if (is_ACK_ok(&ackMessage)) {
          received_ACK = true;
          serverisavailable=true;
          serverwasavailable=true;
          Serial.println("Command executed successfully");
          blinkLED(1);
        } else {
          blinkLEDERROR();
          received_ACK =true;
          Serial.println("Invalid ACK received");
        }
      } else {
        blinkLEDERROR();
        Serial.println("Failed to decode the ACK message");
      }
    }
  }
}
void initESPNOW(){
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.add_peer(SERVER_MAC_ADDRESS);
  ESPNow.reg_recv_cb(onReceive);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(sensorPin, INPUT);
  Serial.begin(115200);
  initESPNOW();
}

void loop() {
  checkWifi();
  //Serial.println(serverAdress);
  //Serial.println(serverPort);
  unsigned long currentTime2 = millis();
  if (currentTime2 - previousTime2 >= interval2)  
  {
    int val = analogRead(sensorPin);
    sendCommand(val);
    currentTime2=previousTime2;
  }
}
