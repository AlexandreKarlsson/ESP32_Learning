#include <ESP_WiFiManager.h>
#include <WiFi.h>
#include <ESPNowW.h>
#define SERVER_PORT 9999
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "pb_tools/pb_tools_msg.h"
#include "pb_tools/pb_tools_msg.c"
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

bool isConnected = false;
bool lightOn = false;
unsigned long disconnectedTime = 0;
unsigned long connectionAttempts = 0;
const unsigned long CONNECTION_RETRY_DELAY = 2000;
const unsigned long MAX_CONNECTION_ATTEMPTS = 15;
WiFiServer server(SERVER_PORT);
WiFiClient client;
AsyncWebServer webserver(80);
const int sensor_pin = 34;

void blinkLED(int nbrOfBlink) {
  for (int i = 0; i < nbrOfBlink; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
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
  IPAddress serveradress;
  int serverport;
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
  wifiManager.autoConnect("ESPserver");
  Serial.println("Connecté au WiFi!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
}

void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  blinkLED(2);
  Serial.println("Message from client!");
  //if (data_len == sizeof(ServerMessage)) {
    ClientMessage messageReceived;
    memcpy(&messageReceived, data, sizeof(messageReceived));
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
    messageToSend.serveradress = WiFi.localIP();
    messageToSend.serverport= SERVER_PORT;
    ESPNow.send_message(messageReceived.mac, (uint8_t*)&messageToSend, sizeof(messageToSend));
  //}
}

void processCommand(const uint8_t* buffer, size_t bytesRead) {
    int value=0;
    int decodeStatus = decodeMessage_Command(buffer, bytesRead, &value);

    if (decodeStatus == 0) {
    blinkLED(1);
    Serial.println(value);

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
void checkSPIFFS(){
  //----------------------------------------------------SPIFFS
  if(!SPIFFS.begin())
  {
    Serial.println("Error SPIFFS...");
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while(file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }
}
void initESPNOW(){
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.reg_recv_cb(onReceive);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  checkSPIFFS();
  initESPNOW();
  blinkLED(3);
}
void launchWevServer(){
  Serial.print("Launch Web Server");
  //----------------------------------------------------SERVER
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  webserver.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/w3.css", "text/css");
  });

  webserver.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  webserver.on("/lireLuminosite", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    int val = analogRead(sensor_pin);
    String luminosite = String(val);
    request->send(200, "text/plain", luminosite);
  });

  webserver.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    blinkLED(1);
    request->send(200);
  });

  webserver.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    blinkLED(2);
    request->send(200);
  });

  webserver.begin();
  Serial.println("Serveur actif!");
}

void checkWifi(){
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
        Serial.println("Connected");
        delay(500);
        server.begin();
        Serial.println("Starting");
        launchWevServer();

        isConnected = true;
        lightOn = true;
    }
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
      Serial.println("WiFi perdu");
      isConnected = false;
      connectionAttempts = 0;
      blinkLEDERROR;
    } 
    else {
      Serial.println(WiFi.localIP());
      delay(500);
      if (!lightOn)lightOn = true;
    }

    if (lightOn) {
      digitalWrite(LED_BUILTIN, HIGH);
      lightOn = false;
    }
  }
}

void loop() {
  checkWifi();
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
