#include <Arduino.h>
#include <WiFi.h>
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"
#define SERVER_IP "192.168.137.232" // Replace by the ESP2 IP
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

void sendCommand(const char* payload, size_t payloadSize) {
  if (!client.connect(SERVER_IP, SERVER_PORT)) {
    Serial.println("Failed to connect to server");
    return;
  }
  blinkLED(3);
  delay(1000);
  Serial.println("Command: "+*payload);
  delay(1000);
  // Create a Message instance
  mypackage_Message message = mypackage_Message_init_default;
  message.cmdType = mypackage_Message_CommandType_COMMAND;
  message.msgSize = payloadSize;
  
  // Set the payload using the pb_bytes_array_t struct
  pb_bytes_array_t payloadBytes;
  payloadBytes.size = payloadSize;
  memcpy(payloadBytes.bytes, payload, payloadSize);
  message.payload.arg = &payloadBytes;
  
  // Create a buffer for the serialized message
  uint8_t buffer[256];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  
  // Serialize the message
  bool status = pb_encode(&stream, mypackage_Message_fields, &message);
  
  if (status) {
    // Send the serialized message via WiFi
    client.write(buffer, stream.bytes_written);
    client.flush();
    Serial.println("Command sent");
    blinkLED(5);

    // Receive and process ACK messages
    while (client.available()) {
      uint8_t ackBuffer[256];
      int bytesRead = client.readBytes(ackBuffer, sizeof(ackBuffer));
      
      // Decode the ACK message
      mypackage_Message ackMessage = mypackage_Message_init_default;
      pb_istream_t ackStream = pb_istream_from_buffer(ackBuffer, bytesRead);
      status = pb_decode(&ackStream, mypackage_Message_fields, &ackMessage);
      
      if (status && ackMessage.cmdType == mypackage_Message_CommandType_ACK) {
        Serial.println("ACK OK");
      } else {
        Serial.println("Failed to decode the ACK message");
      }
    }

  } else {
    Serial.println("Failed to encode the message");
  }
  
  client.stop();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
    Serial.println("Connected");
    lightOn=true;
    isConnected = true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
    lightOn=false;
    return; // Skip the rest of the loop if not connected
  }

  if (isConnected && Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Convert the command string to a character array
    const char* payload = command.c_str();
    
    // Send the command as a serialized message
    sendCommand(payload, command.length());
    
    lightOn = true;
  }

  if(lightOn) {
    digitalWrite(LED_BUILTIN, HIGH);
    lightOn = false;
  }
}
