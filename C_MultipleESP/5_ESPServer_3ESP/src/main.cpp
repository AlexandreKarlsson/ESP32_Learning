#include <Arduino.h>
#include <WiFi.h>
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"
#define SERVER_PORT 9999

WiFiServer server(SERVER_PORT);
WiFiClient client;
bool isConnected = false;
bool lightOn = false;

void blinkLED(int numberOfBlink) {
  for (int i = 0; i < numberOfBlink; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  delay(2000);
}
void blinkLEDERROR() {
  for (int i = 0; i < 20; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
  delay(1000);
}


void processCommand(const Message& message) {
  if (message.cmdType == Message_CommandType_COMMAND) {
    blinkLED(1);
    
    int numberOfBlinks = message.payload;
    
    for (int i = 0; i < numberOfBlinks; i++) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
    }
    
    Message response;
    response.cmdType = Message_CommandType_ACK;
    response.msgSize = 0;
    response.seqNum = message.seqNum;
    response.payload = 0;
    // Create a buffer for the serialized message
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    // Serialize the message
    bool status = pb_encode(&stream, Message_fields, &response);
    
    if (status) {
      // Send the serialized message via WiFi
      client.write(buffer, stream.bytes_written);
      client.flush();
      blinkLED(2);
    } else {
      blinkLEDERROR();
    }
  } else {
    blinkLEDERROR();
  }
  
  blinkLED(3);
  delay(10000);
  blinkLED(2);
  client.stop();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLED(10);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
    Serial.println("Connected");
    delay(2000);
    server.begin();
    Serial.println("Starting");
    isConnected = true;
    lightOn = true;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
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

    if (client.available()) {
      // Read the incoming message
      uint8_t buffer[256];
      size_t bytesRead = client.readBytes(buffer, sizeof(buffer));

      // Parse the received message
      Message message = Message_init_zero;
      pb_istream_t stream = pb_istream_from_buffer(buffer, bytesRead);
      bool status = pb_decode(&stream, Message_fields, &message);

      if (status) {
        processCommand(message);
        lightOn = true;
      } else {
        blinkLEDERROR();
      }
    }
  }
}
