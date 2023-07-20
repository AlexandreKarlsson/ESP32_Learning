#include <Arduino.h>
#include <ESPNowW.h>
#include <WiFi.h>

const unsigned long WIFI_RETRY_DELAY = 20000; // 20 secondes
const unsigned long WIFI_CHECK_INTERVAL = 5000; // Vérifier l'état du WiFi toutes les 5 secondes
// const uint8_t SERVER_MAC_ADDRESS[] = {0xe0, 0x5a, 0x1b, 0xd3, 0x0a, 0x14}; // e0:5a:1b:d3:0a:14
uint8_t SERVER_MAC_ADDRESS[] = {0xe0, 0x5a, 0x1b, 0xd3, 0x0a, 0x14};

struct ServerMessage {
  char ssid[32];
  char password[32];
};

struct ClientMessage {
  uint8_t mac[6];
  bool connected;
};

void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (data_len == sizeof(ServerMessage)) {
    ServerMessage messageReceived;
    memcpy(&messageReceived, data, sizeof(messageReceived));

    // Connecter au WiFi avec les informations reçues du serveur
    Serial.println("Informations de connexion reçues du serveur !");
    Serial.print("SSID : ");
    Serial.println(messageReceived.ssid);
    Serial.print("Password : ");
    Serial.println(messageReceived.password);
    Serial.println();

    // Connecter au WiFi avec les informations du serveur
    WiFi.begin(messageReceived.ssid, messageReceived.password);
  }
}

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

void checkWifi() {
  delay(1000);
  blinkLED(1);
  if (WiFi.status() == WL_CONNECTED) {
    // WiFi connecté avec succès
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("WiFi connecté !");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());
  } else {
    // WiFi déconnecté
    // send_register_ESPNOW();
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Tentative de reconnexion au WiFi...");
    WiFi.reconnect();

    
    ClientMessage messageToSend;
    WiFi.macAddress(messageToSend.mac);
    messageToSend.connected = false;
    uint8_t serializedData[sizeof(messageToSend)];
    memcpy(serializedData, &messageToSend, sizeof(messageToSend));
    ESPNow.send_message(SERVER_MAC_ADDRESS, serializedData, sizeof(serializedData));
    
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  // Initialiser ESP-NOW
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.add_peer(SERVER_MAC_ADDRESS);
  ESPNow.reg_recv_cb(onReceive);
  blinkLED(3);
}

void loop() {
  checkWifi();
}
