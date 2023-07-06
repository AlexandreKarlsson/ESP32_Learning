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
    // Vérifier si des données sont disponibles sur le port série
    if (Serial.available() > 0) 
    {
      blinkLED(1);
      delay(2000);
      // Lire la commande envoyée depuis le terminal UART
      String command = Serial.readStringUntil('\n');
      command.trim(); // Supprimer les espaces blancs au début et à la fin de la commande
      // Vider le caractère de nouvelle ligne du buffer série
      while (Serial.available() && Serial.read() != '\n');
      // Renvoyer les données lues sur l'UART
      Serial.println("\n Command : " + command);
      // Vérifier si la commande est "blink x" (où x est un entier)
      if (command.startsWith("blink")) 
      {
        blinkLED(2);
        delay(2000);
        // Extraire la valeur de x de la commande
        int delayTime;
        String delayString = command.substring(6);
        Serial.print(delayString);
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