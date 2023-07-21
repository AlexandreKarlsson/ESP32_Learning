#include <Arduino.h>
#include <ESPNowW.h>
#include <WiFi.h>
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "pb_tools/pb_tools_msg.h"
#include "pb_tools/pb_tools_msg.c"

const unsigned long WIFI_RETRY_DELAY = 20000; // 20 secondes
const unsigned long WIFI_CHECK_INTERVAL = 5000; // Vérifier l'état du WiFi toutes les 5 secondes
// const uint8_t SERVER_MAC_ADDRESS[] = {0xe0, 0x5a, 0x1b, 0xd3, 0x0a, 0x14}; // e0:5a:1b:d3:0a:14
uint8_t SERVER_MAC_ADDRESS[] = {0xe0, 0x5a, 0x1b, 0xd3, 0x0a, 0x14};
IPAddress serverAdress;
int serverPort;
bool received_ACK;
const int sensorPin = 34;
WiFiClient client;

struct ServerMessage {
  char ssid[32];
  char password[32];
  IPAddress serveradress;
  int serverport;
};

struct ClientMessage {
  uint8_t mac[6];
  bool connected;
};

void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  //if (data_len == sizeof(ServerMessage)) {
    ServerMessage messageReceived;
    memcpy(&messageReceived, data, sizeof(messageReceived));

    // Connecter au WiFi avec les informations reçues du serveur
    Serial.println("Informations de connexion reçues du serveur !");
    Serial.print("SSID : ");
    Serial.println(messageReceived.ssid);
    Serial.print("Password : ");
    Serial.println(messageReceived.password);
    Serial.println();
    serverAdress = messageReceived.serveradress;
    serverPort = messageReceived.serverport;
    // Connecter au WiFi avec les informations du serveur
    WiFi.begin(messageReceived.ssid, messageReceived.password);
  //}
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
    /*
    Serial.println("WiFi connecté !");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());
    */

  } else {
    // WiFi déconnecté
    // send_register_ESPNOW();
    digitalWrite(LED_BUILTIN, LOW);
    //Serial.println("Tentative de reconnexion au WiFi...");
    WiFi.reconnect();

    
    ClientMessage messageToSend;
    WiFi.macAddress(messageToSend.mac);
    messageToSend.connected = false;
    uint8_t serializedData[sizeof(messageToSend)];
    memcpy(serializedData, &messageToSend, sizeof(messageToSend));
    ESPNow.send_message(SERVER_MAC_ADDRESS, serializedData, sizeof(serializedData));
    
  }
}
void sendCommand(int value) {
  if (!client.connect(serverAdress, serverPort)) {
    //Serial.println("Failed to connect to server");
    blinkLEDERROR();
    return;
  }
  
  // Create a buffer for the serialized message
  uint8_t buffer[256];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  
  // Serialize the message
  if (!encodeMessage_Command(&stream, value)) {
    //Serial.println("Failed to encode the Command message");
    blinkLEDERROR();
    client.stop();
    return;
  }
  
  // Send the serialized message via WiFi
  client.write(buffer, stream.bytes_written);
  client.flush();
  //Serial.println("Command sent");
  
  // Wait for an ACK
  //Serial.println("Wait for an ACK");
  uint8_t ackBuffer[256];
  received_ACK = false;
  
  while (client.connected() && !received_ACK) {
    if (client.available()) {
      int bytesRead = client.readBytes(ackBuffer, sizeof(ackBuffer));
      
      // Decode the ACK message
      Message ackMessage = Message_init_default;
      pb_istream_t ackStream = pb_istream_from_buffer(ackBuffer, bytesRead);
      if (pb_decode(&ackStream, Message_fields, &ackMessage)) {
        if (is_ACK_ok(&ackMessage)) {
          received_ACK = true;
          //Serial.println("ACK OK");
          //Serial.println("Command executed successfully");
        } else {
          blinkLEDERROR();
          //Serial.println("Invalid ACK received");
          //Serial.println("Command execution failed");
        }
      } else {
        //Serial.println("Failed to decode the ACK message");
      }
    }
  }
}
void initESPNOW(){
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.add_peer(SERVER_MAC_ADDRESS);
  ESPNow.reg_recv_cb(onReceive);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(sensorPin, INPUT);
  Serial.begin(115200);
  // Initialiser ESP-NOW
  initESPNOW();
  blinkLED(3);
}

void loop() {
  checkWifi();
  delay(1000);
  int val = analogRead(sensorPin);
  sendCommand(val);
}
