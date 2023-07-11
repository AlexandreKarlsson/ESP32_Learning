#include <WiFi.h>

#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"
#define SERVER_IP "192.168.137.233" // Replace by the ESP2 IP
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

void sendCommand(const String& command) {
  if (!client.connect(SERVER_IP, SERVER_PORT)) {
    Serial.println("Failed to connect to server");
    return;
  }
  blinkLED(3);
  Serial.println("Command: " + command);

  client.println(command);
  client.flush();
  delay(1000);
  Serial.println("Waiting for ACK");
  while(true)
  {
    String response = client.readStringUntil('\n');
    if (response.indexOf("Command executed") != -1) {
      Serial.println("\n Response from server: " + response);
      break;
    } else {
      Serial.print("."+response);
    }
  }
  client.stop();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
    Serial.println("Connected");
    lightOn=true;
    isConnected = true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
    lightOn=false;
    return; // Skip the rest of the loop if not connected
  }

  if (isConnected && Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    sendCommand(command);
    lightOn = true;
  }

  if(lightOn)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    lightOn = false;
  }
}
