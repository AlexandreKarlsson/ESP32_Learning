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

#define ESPNUMBER 1



WiFiClient client;
bool isConnected = false;
bool lightOn = false;
int serverPort;
IPAddress serverAdress;
uint8_t SERVER_MAC_ADDRESS[] = {0xe0, 0x5a, 0x1b, 0xc8, 0x3f, 0x14}; //e0:5a:1b:c8:3f:14
unsigned long previousTime = 0;
const unsigned long interval = 5000;

void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  Serial.println("ESPNOW message received");
  ServerMessage messageReceived;
  memcpy(&messageReceived, data, sizeof(messageReceived));
  deserializeIPAddress(messageReceived.serveradress, serverAdress);
  serverPort = messageReceived.serverport;
  // Connecter au WiFi avec les informations du serveur
  WiFi.begin(messageReceived.ssid, messageReceived.password);
}
void initESPNOW(){
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.add_peer(SERVER_MAC_ADDRESS);
  ESPNow.reg_recv_cb(onReceive);
}

void sendCommand(uint32_t espNumber, const char* commandName, int32_t value) {
  if (!client.connect(serverAdress, serverPort)) {
    Serial.println("Failed to connect to server");
    return;
  }

  // Create a buffer for the serialized message
  uint8_t buffer[256];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  // Serialize the message
  if (!encodeMessage_Command(&stream, espNumber, commandName, value)) {
    Serial.println("Failed to encode the Command message");
    client.stop();
    return;
  }

  // Send the serialized message via WiFi
  client.write(buffer, stream.bytes_written);
  client.flush();
}
void sendESPNOWconfig(){
  Serial.print("ESPNOW message :");
  ClientMessage messageToSend;
  WiFi.macAddress(messageToSend.mac);
  messageToSend.connected = false;
  uint8_t serializedData[sizeof(messageToSend)];
  memcpy(serializedData, &messageToSend, sizeof(messageToSend));
  ESPNow.send_message(SERVER_MAC_ADDRESS, serializedData, sizeof(serializedData));
  Serial.println("send");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  initESPNOW();
  Serial.println("Starting");
}
void loop() {
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
    Serial.println("Connected");
    lightOn = true;
    isConnected = true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    isConnected = false;
    lightOn = false;
    unsigned long currentTime = millis();
    if (currentTime - previousTime >= interval) 
    {
    sendESPNOWconfig();
    previousTime = currentTime;
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    }
  
    return; // Skip the rest of the loop if not connected
  }

  if (isConnected && Serial.available() > 0) {
    /*
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.startsWith("blink")) {
      int numberOfBlinks = command.substring(6).toInt();
      sendCommand(ESPNUMBER,"blink",numberOfBlinks);
      Serial.println("Command send");
    }
    */
   delay(5000);
   sendCommand(ESPNUMBER,"blink",3);
   Serial.println("send");
    
    lightOn = true;
  }

  if (lightOn) {
    digitalWrite(LED_BUILTIN, HIGH);
    lightOn = false;
  }
}
