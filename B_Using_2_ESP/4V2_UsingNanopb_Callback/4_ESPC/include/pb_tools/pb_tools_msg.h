#ifndef PB_TOOLS_MSG_H
#define PB_TOOLS_MSG_H

#include "message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

bool encodeMessage_Command(pb_ostream_t *stream, uint32_t espNumber, const char* commandName, int32_t value);
int decodeMessage(const uint8_t* buffer, size_t bytesRead, PayloadData* payloadData, int* ackStatus);
bool encode_command_name(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    const char *commandName = (const char *)*arg;
    return pb_encode_string(stream, (const pb_byte_t *)commandName, strlen(commandName));
}

bool encodePayloadData(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
  PayloadData* payloadData = (PayloadData*)*arg;
  return pb_encode_submessage(stream, PayloadData_fields, payloadData);
}

#endif
