#include <ESP_WiFiManager.h>
#include <WiFi.h>
#include <ESPNowW.h>
//#include "indicator.h"

bool isConnected = false;
bool lightOn = false;
unsigned long disconnectedTime = 0;
unsigned long connectionAttempts = 0;
const unsigned long CONNECTION_RETRY_DELAY = 2000;
const unsigned long MAX_CONNECTION_ATTEMPTS = 15;

void blinkLED(int nbrOfBlink) {
  for (int i = 0; i < nbrOfBlink; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}
void blinkLEDERROR() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

struct ServerMessage {
  char ssid[32];
  char password[32];
};

struct ClientMessage {
  uint8_t mac[6];
  bool connected;
};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Informations de connexion envoyées avec succès à l'ESP client !");
  } else {
    Serial.println("Erreur lors de l'envoi des informations de connexion à l'ESP client !");
  }
  blinkLED(2);
}

void initWifiManager(){
  ESP_WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.autoConnect("ESPserver");
  Serial.println("Connecté au WiFi!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
}

void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  blinkLEDERROR();
  Serial.println("Message from client!");
  //if (data_len == sizeof(ServerMessage)) {
    ClientMessage messageReceived;
    memcpy(&messageReceived, data, sizeof(messageReceived));

    // Connecter au WiFi avec les informations reçues du serveur
    
    Serial.print("connected : ");
    Serial.println(messageReceived.connected);
    Serial.print("MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.print(messageReceived.mac[i], HEX);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
    ESPNow.add_peer(messageReceived.mac);

    ServerMessage messageToSend;
    strncpy(messageToSend.password, WiFi.psk().c_str(), sizeof(messageToSend.password));
    strncpy(messageToSend.ssid, WiFi.SSID().c_str(), sizeof(messageToSend.ssid));

    // Envoie les informations de connexion au client
    ESPNow.send_message(messageReceived.mac, (uint8_t*)&messageToSend, sizeof(messageToSend));
  //}
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialiser ESP-NOW
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.reg_recv_cb(onReceive);
  blinkLED(3);
}

void checkWifi(){
  if (!isConnected) {
    if (WiFi.status() != WL_CONNECTED) {
      unsigned long currentTime = millis();
      if (currentTime - disconnectedTime >= (connectionAttempts * CONNECTION_RETRY_DELAY)) {

        Serial.print("Tentative de reconnexion au WiFi :");
        delay(100);
        Serial.print(connectionAttempts);
        Serial.print("/");
        Serial.println(MAX_CONNECTION_ATTEMPTS);
        WiFi.reconnect();
        disconnectedTime = currentTime;
        connectionAttempts++;

        if (connectionAttempts >= MAX_CONNECTION_ATTEMPTS) {
          Serial.println("Pas de connexion. Lancement d'un portail...");
          WiFi.disconnect();
          ESP_WiFiManager wifiManager;
          wifiManager.resetSettings();
          wifiManager.startConfigPortal("ESPserver");
          blinkLEDERROR;
        }
      }
    } 
    else {
      // Connexion établie
      isConnected = true;
      Serial.println("WiFi connecté !");
      Serial.print("Adresse IP: ");
      Serial.println(WiFi.localIP());
    }
  } 
  else {
    if (WiFi.status() != WL_CONNECTED) {
      // La connexion WiFi a été perdue
      Serial.println("WiFi perdu");
      isConnected = false;
      connectionAttempts = 0;  // Réinitialiser le compteur de tentatives de reconnexion
      blinkLEDERROR;
    } 
    else {
      Serial.println("Connecté");
      delay(1000);
      lightOn = true;
    }

    if (lightOn) {
      digitalWrite(LED_BUILTIN, HIGH);
      lightOn = false;
    }
  }
}

void loop() {
  checkWifi();
}
