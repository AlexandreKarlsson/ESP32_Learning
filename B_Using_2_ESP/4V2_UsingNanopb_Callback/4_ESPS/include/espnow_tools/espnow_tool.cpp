#include "espnow_tool.h"
#include <WiFi.h>
#include <Arduino.h>
#include <ESPNowW.h>
#include <pb_encode.h>
#include <pb_decode.h>

void serializeIPAddress(const IPAddress& ip, uint8_t* buffer) {
  for (int i = 0; i < 4; i++) {
    buffer[i] = ip[i];
  }
}

void deserializeIPAddress(const uint8_t* buffer, IPAddress& ip) {
  for (int i = 0; i < 4; i++) {
    ip[i] = buffer[i];
  }
}

