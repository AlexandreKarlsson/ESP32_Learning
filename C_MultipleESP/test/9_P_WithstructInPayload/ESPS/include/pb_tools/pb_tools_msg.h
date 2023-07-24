#ifndef PB_TOOLS_MSG_H
#define PB_TOOLS_MSG_H

#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
struct PayloadMessage {
  uint8_t command;
  int value;
};
bool encodeMessage_Command(pb_ostream_t *stream, uint8_t command,int value);
int decodeMessage_Command(const uint8_t* buffer, size_t bytesRead, int* payload);
bool get_ACK(pb_ostream_t *stream, bool isACK_OK);
bool is_ACK_ok(const Message *message);

#endif
