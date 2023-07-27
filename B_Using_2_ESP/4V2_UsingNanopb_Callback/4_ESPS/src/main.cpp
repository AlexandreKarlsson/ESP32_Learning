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

#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"
#define SERVER_PORT 9999

WiFiServer server(SERVER_PORT);
WiFiClient client;
bool isConnected = false;
bool lightOn = false;


void processCommand(const uint8_t* buffer, size_t bytesRead) {
    PayloadData payloadData;
    int ackStatus;
    int messageType = decodeMessage(buffer, bytesRead, &payloadData, &ackStatus);

    if (messageType == 0) {
        blinkLED(1);

        for (int i = 0; i < payloadData.value; i++) {
            digitalWrite(LED_BUILTIN, LOW);
            delay(200);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(200);
        }
    }
}
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  blinkLED(1);
  Serial.print("Message from client");
  //if (data_len == sizeof(ServerMessage)) {
  ClientMessage messageReceived;
  memcpy(&messageReceived, data, sizeof(messageReceived));
  Serial.print("  connected : ");
  Serial.print(messageReceived.connected);
  Serial.print("  MAC: ");
  for (int i = 0; i < 6; i++) 
  {
    Serial.print(messageReceived.mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  //////////////////////////////////////////////
  ESPNow.init();
  ESPNow.add_peer(messageReceived.mac);
  Serial.println("Peer added");
  ServerMessage messageToSend;
  strncpy(messageToSend.password, WiFi.psk().c_str(), sizeof(messageToSend.password));
  strncpy(messageToSend.ssid, WiFi.SSID().c_str(), sizeof(messageToSend.ssid));
  IPAddress localIP = WiFi.localIP();
  serializeIPAddress(localIP, messageToSend.serveradress);
  messageToSend.serverport= SERVER_PORT;
  Serial.println("Message configured");
  ESPNow.send_message(messageReceived.mac, (uint8_t*)&messageToSend, sizeof(messageToSend));
  Serial.println("Message send");
  //}
}


void initESPNOW(){
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.reg_recv_cb(onReceive);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLED(10);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  initESPNOW();
}


void loop() {
    if (!isConnected && WiFi.status() == WL_CONNECTED) {
        // WiFi is connected for the first time
        Serial.println("Connected");
        server.begin();
        isConnected = true;
        lightOn = true;
    }
  
    if (WiFi.status() != WL_CONNECTED) {
        // WiFi is disconnected
        blinkLED(1);
        isConnected = false;
        lightOn = false;
        return; // Skip the rest of the loop if not connected
    }
  
    if (lightOn) {
        digitalWrite(LED_BUILTIN, HIGH);
        lightOn = false;
    }
  
    if (isConnected) {
        if (!client || !client.connected()) {
            client = server.available();
            return;
        }

        if (client.available()) 
        {
            // Read the incoming message
            uint8_t buffer[256];
            size_t bytesRead = client.readBytes(buffer, sizeof(buffer));
           
            processCommand(buffer, bytesRead);
            lightOn = true;
        }
    }
}


