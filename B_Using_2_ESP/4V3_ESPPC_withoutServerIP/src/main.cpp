#include <Arduino.h>
#include <WiFi.h>
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "pb_tools/pb_tools_msg.h"
#include "pb_tools/pb_tools_msg.c"

#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"
#define SERVER_MAC { 0xE0, 0x5A, 0x1B, 0xC8, 0x03, 0x24 }
#define SERVER_HOSTNAME "esp32-C80324"
#define SERVER_PORT 9999

WiFiClient client;
bool isConnected = false;
bool lightOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Starting");
}

void blinkLED(int numberOfBlink) {
  for (int i = 0; i < numberOfBlink; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void sendCommand(int numberOfBlinks) {
  //IPAddress serverIP;
  IPAddress serverIP;
  
  if (!WiFi.macAddress(SERVER_MAC, serverIP)) {
    Serial.println("Failed to convert MAC to IP");
    return;
  }
  /*
  // Résolution de l'adresse MAC en adresse IP
  if (!WiFi.hostByName()) {
    Serial.println("Failed to resolve server IP 1");
    return;
  }
  */
  
 // Résolution du nom d'hôte en adresse IP
  if (!WiFi.hostByName(SERVER_HOSTNAME, serverIP)) {
    Serial.println("Failed to resolve server IP 2");
    return;
  }
  if (!client.connect(serverIP, SERVER_PORT)) {
    Serial.println("Failed to connect to server");
    return;
  }
  
  // Create a buffer for the serialized message
  uint8_t buffer[256];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  
  // Serialize the message
  if (!encodeMessage_Command(&stream, numberOfBlinks)) {
    Serial.println("Failed to encode the Command message");
    client.stop();
    return;
  }
  
  // Send the serialized message via WiFi
  client.write(buffer, stream.bytes_written);
  client.flush();
  Serial.println("Command sent");
  
  // Wait for an ACK
  Serial.println("Wait for an ACK");
  uint8_t ackBuffer[256];
  bool received_ACK = false;
  
  while (client.connected() && !received_ACK) {
    if (client.available()) {
      int bytesRead = client.readBytes(ackBuffer, sizeof(ackBuffer));
      
      // Decode the ACK message
      Message ackMessage = Message_init_default;
      pb_istream_t ackStream = pb_istream_from_buffer(ackBuffer, bytesRead);
      if (pb_decode(&ackStream, Message_fields, &ackMessage)) {
        if (is_ACK_ok(&ackMessage)) {
          received_ACK = true;
          Serial.println("ACK OK");
          Serial.println("Command executed successfully");
        } else {
          Serial.println("Invalid ACK received");
          Serial.println("Command execution failed");
        }
      } else {
        Serial.println("Failed to decode the ACK message");
      }
    }
  }
  
  client.stop();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
    Serial.println("Connected");
    lightOn = true;
    isConnected = true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
    lightOn = false;
    return; // Skip the rest of the loop if not connected
  }

  if (isConnected && Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.startsWith("blink")) {
      int numberOfBlinks = command.substring(6).toInt();
      sendCommand(numberOfBlinks);
    }
    
    lightOn = true;
  }

  if (lightOn) {
    digitalWrite(LED_BUILTIN, HIGH);
    lightOn = false;
  }
}
