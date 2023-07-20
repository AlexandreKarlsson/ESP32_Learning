#include <ESP_WiFiManager.h>

bool isConnected = false;
bool lightOn = false;
unsigned long disconnectedTime = 0;
unsigned long connectionAttempts = 0;
const unsigned long CONNECTION_RETRY_DELAY = 2000; 
const unsigned long MAX_CONNECTION_ATTEMPTS = 15;  

void setup() {
  Serial.begin(115200);
  
  ESP_WiFiManager wifiManager;
  //wifiManager.resetSettings();

  wifiManager.autoConnect("ESPserver");

  Serial.println("Connecté au WiFi!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (!isConnected) {
    if (WiFi.status() != WL_CONNECTED) {
      unsigned long currentTime = millis();
      if (currentTime - disconnectedTime >= (connectionAttempts * CONNECTION_RETRY_DELAY)) 
      {

        Serial.print("Tentative de reconnexion au WiFi :");
        delay(100);
        Serial.print(connectionAttempts);
        Serial.print("/");
        Serial.println(MAX_CONNECTION_ATTEMPTS);
        WiFi.reconnect();
        disconnectedTime = currentTime;
        connectionAttempts++;

        if (connectionAttempts >= MAX_CONNECTION_ATTEMPTS) {
          Serial.println("No connection. Launching a portal...");
          WiFi.disconnect();
          ESP_WiFiManager wifiManager;
          wifiManager.resetSettings();
          wifiManager.startConfigPortal("ESPserver");
        }
      }
    } 
    else {
      // Connexion établie
      isConnected = true;
      Serial.println("Connected WiFi!");
      Serial.print("Adress IP: ");
      Serial.println(WiFi.localIP());
    }
  } 
  else {
    if (WiFi.status() != WL_CONNECTED) {
      // La connexion WiFi a été perdue
      Serial.println("Wifi Lost");
      isConnected = false;
      connectionAttempts = 0;  // Réinitialisation du compteur de tentatives de reconnexion
    } 
    else {
      Serial.println("Connected");
      delay(1000);
      lightOn = true;
    }

    if (lightOn) {
      digitalWrite(LED_BUILTIN, HIGH);
      lightOn = false;
    }
  }
}
