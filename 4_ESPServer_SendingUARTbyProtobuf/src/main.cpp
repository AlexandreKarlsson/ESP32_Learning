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
bool lightON = false;

void blinkLED(int numberOfBlink) {
  for (int i = 0; i < numberOfBlink; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void processCommand(const mypackage_Message& message) {
  if (message.cmdType == mypackage_Message_CommandType_COMMAND) {
    int delayTime = atoi((const char*)message.payload.arg);
    blinkLED(3); // Indicate command received
    delay(2000);

    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(delayTime);
      digitalWrite(LED_BUILTIN, LOW);
      delay(delayTime);
    }
    
    mypackage_Message response;
    response.cmdType = mypackage_Message_CommandType_ACK;
    response.msgSize = 0;
    response.seqNum = message.seqNum;
    response.payload.arg = NULL;
    response.msgEnd = 0;

    // Create a buffer for the serialized message
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    // Serialize the message
    bool status = pb_encode(&stream, mypackage_Message_fields, &response);

    if (status) {
      // Send the serialized message via WiFi
      client.write(buffer, stream.bytes_written);
      client.flush();
      Serial.println("Command executed");
    } else {
      Serial.println("Failed to encode the message");
    }
  } else {
    Serial.println("ERROR : Invalid command");
  }
  
  delay(1000);
  blinkLED(3);
  delay(1000);
  client.stop();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLED(10);
  delay(2000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
    Serial.println("Connected");
    delay(3000);
    server.begin();
    Serial.println("Starting");
    isConnected = true;
    lightON = true;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
    lightON = false;
    return; // Skip the rest of the loop if not connected
  }
  
  if (lightON) {
    digitalWrite(LED_BUILTIN, HIGH);
    lightON = false;
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
      mypackage_Message message = mypackage_Message_init_zero;
      pb_istream_t stream = pb_istream_from_buffer(buffer, bytesRead);
      bool status = pb_decode(&stream, mypackage_Message_fields, &message);

      if (status) {
        processCommand(message);
        lightON = true;
      } else {
        blinkLED(10);
        delay(1000);
        Serial.println("Failed to decode the message");
      }
    }
  }
}
