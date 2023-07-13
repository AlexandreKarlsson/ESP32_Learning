#include <Arduino.h>
#include <WiFi.h>
#include "message.pb.h"
#include "message.pb.cc"
#include <pb_encode.h>
#include <pb_decode.h>
#define blinkingTime 500
#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"
// Define the IP address and port of the server
IPAddress serverIP(192, 168, 1, 100);
const int serverPort = 1234;

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
void setup() 
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("starting");
}

bool isConnected = false;

void loop() {
  if (WiFi.status() == WL_CONNECTED && !isConnected) 
  {
    Serial.println("Connected");
    digitalWrite(LED_BUILTIN, HIGH);
    isConnected = true;
  }

  if (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
  }

  if(isConnected)
  {
    // If UART activate
    if (Serial.available() > 0) 
    {
      blinkLED(3);
      delay(2000);
      // Take all char until \n => Doesn't work well with some of UART terminal (PlatformIO serial terminal is fine)
      String command = Serial.readStringUntil('\n');
      // Suppress blank before and after
      command.trim(); 
      // Suppress all the line
      while (Serial.available() && Serial.read() != '\n');
      // Send back the command to confirm what is read
      Serial.println("\n Command : " + command);
      // If it's a blink command
      if (command.startsWith("blink")) 
      {
        blinkLED(6);
        delay(2000);
        // Extract the integer of the command
        int delayTime;
        String delayString = command.substring(6);
        Serial.print(delayString);
        // Security if the variable is an integer
        if (delayString.toInt() != 0) 
        {
          delayTime = delayString.toInt();
          blinkLED(9);
          delay(2000);
          Serial.println("\n Sending Blinking Command ");  
          // Build the Message
          message.set_cmdtype(Message_CommandType_COMMAND);
          message.set_msgsize(6); // Size of "blink x"
          message.set_seqnum(1); // Example sequence number
          message.set_payload(command); // Command as payload
          message.set_msgend(0); // Example message end

          // Serialize the message into a byte array
          uint8_t messageBuffer[message_size];
          pb_ostream_t stream = pb_ostream_from_buffer(messageBuffer, sizeof(messageBuffer));
          pb_encode(&stream, Message_fields, &message);

          // Establish a TCP connection to the server
          WiFiClient client;
          if (client.connect(serverIP, serverPort)) 
          {
            blinkLED(9);
            delay(2000);
            // Send the message over the TCP connection
            client.write(messageBuffer, stream.bytes_written);
            client.flush();
            client.stop();
          }
        } 
        else 
        {
          blinkLED(2);
        }
      }
    }
  }
}