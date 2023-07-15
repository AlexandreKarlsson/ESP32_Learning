#include <WiFi.h>

#define WIFI_SSID "reseauESP"
#define WIFI_PASSWORD "mdp123456789"
#define SERVER_PORT 9999

WiFiServer server(SERVER_PORT);
WiFiClient client;

bool isConnected = false;
bool lightON = false;

void blinkLED(int numberOfBlink) {
  for (int i = 0; i < numberOfBlink; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLED(10);
  delay(2000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void processCommand(const String& command) {
  if (command.startsWith("blink")) {
    int delayTime = command.substring(6).toInt();
    blinkLED(3); // Indicate command received
    delay(2000);

    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(delayTime);
      digitalWrite(LED_BUILTIN, LOW);
      delay(delayTime);
    }
    client.println("Command executed");
  }
  else{
    client.println("ERROR : Command executed");
  }
  delay(1000);
  blinkLED(3);
  delay(1000);
  client.stop();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
    Serial.println("Connected");
    delay(3000);
    server.begin();
    Serial.println("Starting");
    isConnected = true;
    lightON = true;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
    lightON = false;
    return; // Skip the rest of the loop if not connected
  }
  if(lightON)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    lightON = false;
  }
  if (isConnected) {
    if (!client || !client.connected()) {
      client = server.available();
      return;
    }

    if (client.available()) {
      String command = client.readStringUntil('\n');
      command.trim();
      processCommand(command);
      lightON=true;
    }
  }
}
