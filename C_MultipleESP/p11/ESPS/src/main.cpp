#include <ESP_WiFiManager.h>
#include <WiFi.h>
#include <ESPNowW.h>
#define SERVER_PORT 9999
#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "pb_tools/pb_tools_msg.h"
#include "pb_tools/pb_tools_msg.c"
#include "hardware/indicator.h"
#include "hardware/indicator.cpp"
#include "espnow_tools/espnow_tool.h"
#include "espnow_tools/espnow_tool.cpp"
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

bool isConnected = false;
bool lightOn = false;
bool espnowActive =false;
uint8_t mac_Client[6];
unsigned long disconnectedTime = 0;
unsigned long previousTime = 0;
const unsigned long interval = 10000;
unsigned long connectionAttempts = 0;
const unsigned long CONNECTION_RETRY_DELAY = 5000;
const unsigned long MAX_CONNECTION_ATTEMPTS = 6;
int value_esp1=0;
int value_esp2=0;
WiFiServer server(SERVER_PORT);
WiFiClient client;
AsyncWebServer webserver(80);
const int sensor_pin = 34;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("ESPNOW send success");
  } else {
    Serial.println("ERROR sending ESPNOW");
  }
}

void initWifiManager(){
  ESP_WiFiManager wifiManager;
  WiFi.mode(WIFI_MODE_STA);
  wifiManager.autoConnect("ESPserver");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
/*
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  blinkLED(2);
  Serial.print("Message from client");
  //if (data_len == sizeof(ServerMessage)) {
    ClientMessage messageReceived;
    memcpy(&messageReceived, data, sizeof(messageReceived));
    Serial.print("  connected : ");
    Serial.print(messageReceived.connected);
    Serial.print("  MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.print(messageReceived.mac[i], HEX);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
    ESPNow.init();
    ESPNow.add_peer(messageReceived.mac);
    Serial.println("Peer added");
    ServerMessage messageToSend;
    strncpy(messageToSend.password, WiFi.psk().c_str(), sizeof(messageToSend.password));
    strncpy(messageToSend.ssid, WiFi.SSID().c_str(), sizeof(messageToSend.ssid));
    IPAddress localIP = WiFi.localIP();
    serializeIPAddress(localIP, messageToSend.serveradress);
    messageToSend.serverport= SERVER_PORT;
    Serial.println("Message configured");
    ESPNow.send_message(messageReceived.mac, (uint8_t*)&messageToSend, sizeof(messageToSend));
    Serial.println("Message send");
  //}
}
*/
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  Serial.println("Message from client");
  if(isConnected) {
    espnowActive=true;
    // Assuming data_len is equal to sizeof(ClientMessage)
    ClientMessage messageReceived;
    memcpy(&messageReceived, data, sizeof(messageReceived));
    memcpy(&mac_Client, messageReceived.mac, sizeof(mac_Client));
    Serial.print("  connected : ");
    Serial.print(messageReceived.connected);
    Serial.print("  MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.print(mac_Client[i], HEX);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }
}
void sendespnowConfig(){
  ESPNow.init();
  ESPNow.add_peer(mac_Client);
  Serial.println("Peer added");
  ServerMessage messageToSend;
  strncpy(messageToSend.password, WiFi.psk().c_str(), sizeof(messageToSend.password));
  strncpy(messageToSend.ssid, WiFi.SSID().c_str(), sizeof(messageToSend.ssid));
  IPAddress localIP = WiFi.localIP();
  serializeIPAddress(localIP, messageToSend.serveradress);
  messageToSend.serverport = SERVER_PORT;
  
  // Afficher les valeurs des variables du message
  Serial.print("Message to send - SSID: ");
  Serial.println(messageToSend.ssid);
  Serial.print("Message to send - Password: ");
  Serial.println(messageToSend.password);
  Serial.print("Message to send - Server IP Address: ");
  Serial.println(localIP);
  Serial.print("Message to send - Server Port: ");
  Serial.println(messageToSend.serverport);
  Serial.println("Message configured");
  ESPNow.send_message(mac_Client, (uint8_t*)&messageToSend, sizeof(messageToSend));
  Serial.println("Message sent");
  espnowActive=false;
}


void processCommand(const uint8_t* buffer, size_t bytesRead) {
    PayloadData payloadData;
    int ackStatus = 0; // You may need to set a proper initial value based on your logic

    int decodeResult = decodeMessage(buffer, bytesRead, &payloadData, &ackStatus);
    if (decodeResult == 0) {
        // Decoding successful
        int espNumber = payloadData.deviceNumber;
        const char* commandName = (const char*)payloadData.commandName.arg;
        int32_t value = payloadData.value;

        // Process the command based on espNumber, commandName, and value
        Serial.print("Received Command from ESP: ");
        Serial.println(espNumber);
        Serial.print("Command Name: ");
        Serial.println(commandName);
        Serial.print("Command Value: ");
        Serial.println(value);

       // Create a buffer for the serialized message
        uint8_t ackBuffer[256];
        pb_ostream_t ackStream = pb_ostream_from_buffer(ackBuffer, sizeof(ackBuffer));

        // Assuming you want to send an ACK indicating success
        int success = 0;
        bool ackEncodeStatus = encodeMessage_ACK(&ackStream, success);

        if (ackEncodeStatus) {
            // Send the serialized ACK message via WiFi
            // client.write(ackBuffer, ackStream.bytes_written);
            // client.flush();
            // blinkLED(2);
        } else {
            Serial.println("ERROR encoding ACK");
            blinkLEDERROR();
        }
        
    } else if (decodeResult == 1) {
        // Handle decoding failure
        // ...
        value_esp1 = ackStatus; // ??
    } else if (decodeResult == 11) {
        // Handle invalid ACK payload data
        // ...
        value_esp2 = ackStatus; // ??
    } else if (decodeResult == 10) {
        // Handle ACK decoding successful
        // ...
    } else if (decodeResult == 20) {
        // Handle REQ command type (if needed)
        // ...
    } else {
        // Handle other cases
        blinkLEDERROR();
    }
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
  
  //WiFi.mode(WIFI_MODE_AP);
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.reg_recv_cb(onReceive);
  ESPNow.reg_send_cb(OnDataSent);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  initESPNOW();
  checkSPIFFS();
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

   webserver.on("/esp1", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String value = String(value_esp1);
    request->send(200, "text/plain", value);
  });

   webserver.on("/esp2", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String value = String(value_esp2);
    request->send(200, "text/plain", String(value));
  });

  webserver.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    //blinkLED(1);
    request->send(200);
  });

  webserver.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    //blinkLED(2);
    request->send(200);
  });

  webserver.begin();
  Serial.println("Serveur actif!");
}

void checkWifi(){
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
        Serial.println("Connected");
        server.begin();
        launchWevServer();
        isConnected = true;
        lightOn = true;
    }
  if (!isConnected) {
    if (WiFi.status() != WL_CONNECTED) {
      unsigned long currentTime = millis();
      if (currentTime - disconnectedTime >= CONNECTION_RETRY_DELAY) {
        Serial.print("connection try :");
        Serial.print(connectionAttempts);
        Serial.print("/");
        Serial.println(MAX_CONNECTION_ATTEMPTS);
        WiFi.reconnect();
        
        disconnectedTime = currentTime;
        connectionAttempts++;
        if (WiFi.SSID()==0)
        {
          Serial.println("No SSID register");
        }
        if (connectionAttempts >= MAX_CONNECTION_ATTEMPTS) {
          Serial.println("No connection. WiFiManager launch");
          WiFi.disconnect();
          ESP_WiFiManager wifiManager;
          wifiManager.resetSettings();
          wifiManager.startConfigPortal("ESPserver");
          blinkLEDERROR;
        }
      }
    } 
    else 
    {
      // Connection acquired
      isConnected = true;
      Serial.println("WiFi connected !");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    }
  } 
  else 
  {
    if (WiFi.status() != WL_CONNECTED) 
    {
      // Connection just lost
      Serial.println("WiFi lost");
      isConnected = false;
      connectionAttempts = 0;
      blinkLEDERROR;
    } 
    else 
    {
      // ALL OK : server connected
      unsigned long currentTime = millis();
      if (currentTime - previousTime >= interval) 
      {
      Serial.println(WiFi.localIP());
      if (!lightOn)lightOn = true;
      previousTime = currentTime;
      }
    }
    if (lightOn) {
      digitalWrite(LED_BUILTIN, HIGH);
      lightOn = false;
    }
  }
}

void loop() {
  if(!espnowActive){
    checkWifi();
    delay(200);
    if (isConnected) {
          if (!client || !client.connected()) {
            //Serial.print("x");
            client = server.available();
          }

          if (client.available()) 
          {
            Serial.print(".");
            /*
            // Read the incoming message
            uint8_t buffer[256];
            size_t bytesRead = client.readBytes(buffer, sizeof(buffer));
            //blinkLED(1);
            processCommand(buffer, bytesRead);
            lightOn = true;
            */
          }
      }
  }else{
    sendespnowConfig();
  }
  
}
