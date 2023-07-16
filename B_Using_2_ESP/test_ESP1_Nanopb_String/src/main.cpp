#include <Arduino.h>
#include <WiFi.h>
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "My_Tools/pb_tools.h"

#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"
#define SERVER_IP "192.168.137.225" // Replace by the ESPServer IP
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

void sendCommand(const std::string& payload) {
  if (!client.connect(SERVER_IP, SERVER_PORT)) {
    Serial.println("Failed to connect to server");
    return;
  }
  blinkLED(3);
  delay(1000);
  Serial.print("Command: ");
  Serial.println(payload.c_str());
  delay(1000);

  // Create a Message instance
  mypackage_Message message = mypackage_Message_init_default;
  message.cmdType = mypackage_Message_CommandType_COMMAND;
  message.msgSize = payload.size();

  
  // Set the payload using the encode_string callback
  pb_callback_t payloadCallback;
  payloadCallback.funcs.encode = &encode_string;
  payloadCallback.arg = reinterpret_cast<void*>(const_cast<char*>(payload.c_str()));
  message.payload = payloadCallback;


  message.msgEnd=0;
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

    size_t msgSize = ackMessage.msgSize;
    std::string payloadStr;
    pb_callback_t stringCallback = { decode_string_callback, &payloadStr };
    ackMessage.payload.funcs.decode = &decode_string_callback;
    ackMessage.payload.arg = &stringCallback;


    // Check if the payload is "OK"
    if (payloadStr == "OK") {
      Serial.println("ACK OK");
    } else {
      Serial.println("ACK received, but payload is not OK");
    }
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

    if (command.startsWith("blink ")) {
      // Get the blink count from the command
      int blinkCount = command.substring(6).toInt();

      // Check if the blink count is valid
      if (blinkCount > 0) {
        // Construct the command with the format "b:x"
        std::string formattedCommand = "b:" + std::to_string(blinkCount);
        // Send the command
        sendCommand(formattedCommand);
      } else {
        Serial.println("Invalid blink count");
      }
    } else {
      Serial.println("Invalid command");
    }

    lightOn = true;
  }

  if (lightOn) {
    digitalWrite(LED_BUILTIN, HIGH);
    lightOn = false;
  }
}
