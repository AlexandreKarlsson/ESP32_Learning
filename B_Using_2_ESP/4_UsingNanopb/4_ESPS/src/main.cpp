#include <Arduino.h>
#include <WiFi.h>
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "pb_tools/pb_tools_msg.h"
#include "pb_tools/pb_tools_msg.c"

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

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLED(10);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void processCommand(const uint8_t* buffer, size_t bytesRead) {
    int numberOfBlinks=0;
    int decodeStatus = decodeMessage_Command(buffer, bytesRead, &numberOfBlinks);

    if (decodeStatus == 0) {
    blinkLED(1);

    for (int i = 0; i < numberOfBlinks; i++) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
    }

    // Create a buffer for the serialized message
    uint8_t ackBuffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(ackBuffer, sizeof(ackBuffer));
    // Create an ACK message
    bool status = get_ACK(&stream, true);

    if (status) {
        // Send the serialized message via WiFi
        client.write(ackBuffer, stream.bytes_written);
        client.flush();
        blinkLED(2);
    } else {
        blinkLEDERROR();
    }
    } else {
    blinkLEDERROR();
  }

  blinkLED(3);
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

