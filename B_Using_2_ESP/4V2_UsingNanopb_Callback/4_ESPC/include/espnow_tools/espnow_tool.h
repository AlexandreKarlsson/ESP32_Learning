#ifndef ESPNOW_TOO_H
#define ESPNOW_TOO_H

struct ServerMessage {
  char ssid[32];
  char password[32];
  uint8_t serveradress[4];
  int serverport;
};

struct ClientMessage {
  uint8_t mac[6];
  bool connected;
};

void serializeIPAddress(const IPAddress& ip, uint8_t* buffer);
void deserializeIPAddress(const uint8_t* buffer, IPAddress& ip);

#endif // ESPNOW_TOO_H