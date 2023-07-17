#include "pb_tools_msg.h"

bool encodeMessage_Command(pb_ostream_t *stream, int numberOfBlinks) {
  Message message = Message_init_default;
  message.cmdType = Message_CommandType_COMMAND;
  message.msgSize = sizeof(numberOfBlinks);
  message.payload = numberOfBlinks;
  return pb_encode(stream, Message_fields, &message);
}

int decodeMessage_Command(const uint8_t* buffer, size_t bytesRead, int* payload) {
  // Parse the received message
  Message message = Message_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(buffer, bytesRead);
  bool status = pb_decode(&stream, Message_fields, &message);

  if (!status) {
    return 1; // Failed to decode the message
  }

  if (message.cmdType != Message_CommandType_COMMAND) {
    return 2; // Invalid command type
  }

  if (message.msgSize != sizeof(*payload)) {
    return 3; // Invalid message size
  }

  *payload = message.payload;
  return 0; // Decoding successful
}


bool get_ACK(pb_ostream_t *stream, bool isACK_OK) {
  // Create an ACK message
  Message ackMessage = Message_init_default;
  ackMessage.cmdType = Message_CommandType_ACK;
  ackMessage.payload = isACK_OK ? 0 : 1;

  // Encode the ACK message
  bool status = pb_encode(stream, Message_fields, &ackMessage);
  return status;
}


bool is_ACK_ok(const Message *message) {
  return (message->cmdType == Message_CommandType_ACK) && (message->payload == 0);
}
