// Include the necessary libraries
#include <Arduino.h>
#include <WiFi.h>
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

#define blinkingTime 500

// Define WiFi credentials
#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"

// Define the server port
const int serverPort = 1234;
WiFiServer server(serverPort);

// Create a global instance of the Message
Message message;

void blinkLED(int numberOfBlink)
{
  for (int i = 0; i < numberOfBlink; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(blinkingTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(blinkingTime);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  blinkLED(2);
  delay(2000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  blinkLED(2);
  delay(2000);
  Serial.println("WiFi connected");

  // Start the TCP server
  server.begin();
  blinkLED(2);
  delay(2000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    blinkLED(3);
    delay(2000);
    // Check if a client is available
    WiFiClient client = server.available();
    if (client) {
      // Wait for data to be available
      while (!client.available()) {
        delay(1);
      }
      blinkLED(6);
      delay(2000);

      // Read the incoming data into a buffer
      uint8_t messageBuffer[message.msgsize()];
      int bytesRead = client.read(messageBuffer, sizeof(messageBuffer));

      // Deserialize the message from the buffer
      pb_istream_t stream = pb_istream_from_buffer(messageBuffer, bytesRead);
      pb_decode(&stream, Message_fields, &message);

      // Check if the command type is COMMAND
      if (message.cmdtype() == Message_CommandType_COMMAND) {
        // Extract the payload (command)
        String command = String(message.payload().c_str());

        // Perform the blinking operation if the command is "blink x"
        if (command.startsWith("blink")) {
          blinkLED(9);
          delay(2000);
          // Extract the delay time from the command
          int delayTime = command.substring(6).toInt();

          // Perform the blinking operation with the received delay time
          for (int i = 0; i < 3; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(delayTime);
            digitalWrite(LED_BUILTIN, LOW);
            delay(delayTime);
          }

          // Build the ACK message
          message.set_cmdtype(Message_CommandType_ACK);
          message.set_msgsize(0);
          message.set_seqnum(1); // Example sequence number
          message.set_payload(""); // Empty payload
          message.set_msgend(0); // Example message end

          // Serialize the message into a byte array
          uint8_t ackBuffer[message.msgsize()];
          pb_ostream_t ackStream;
          ackStream = pb_ostream_from_buffer(ackBuffer, sizeof(ackBuffer));
          pb_encode(&ackStream, Message_fields, &message);

          // Send the ACK message over the TCP connection
          client.write(ackBuffer, ackStream.bytes_written);
          client.flush();
        }
      }
    }
  }
}
