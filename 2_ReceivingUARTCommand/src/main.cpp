#include <Arduino.h>
#include <WiFi.h>
#define blinkingTime 500
#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"

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
      blinkLED(1);
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
        blinkLED(2);
        delay(2000);
        // Extract the integer of the command
        int delayTime;
        String delayString = command.substring(6);
        Serial.print(delayString);
        // Security if the variable is an integer
        if (delayString.toInt() != 0) 
        {
          delayTime = delayString.toInt();
          blinkLED(3);
          delay(2000);
          Serial.println("\n Delay received ");
          digitalWrite(LED_BUILTIN, HIGH);
          delay(delayTime);
          digitalWrite(LED_BUILTIN, LOW);
          delay(delayTime);
          digitalWrite(LED_BUILTIN, HIGH);
          delay(delayTime);
          digitalWrite(LED_BUILTIN, LOW);
          delay(delayTime);
        } 
        else 
        {
          blinkLED(10);
        }
      }
    }
  }
}