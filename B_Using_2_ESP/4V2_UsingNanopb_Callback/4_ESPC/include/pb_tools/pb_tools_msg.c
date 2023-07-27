#include <Arduino.h>
#include "pb_tools/pb_tools_msg.h"
#include "message.pb.h" // Assuming you have the generated message_pb header

bool encodeMessage_Command(pb_ostream_t *stream, uint32_t espNumber, const char* commandName, int32_t value) {
    Message message = Message_init_default;
    message.cmdType = Message_CommandType_COMMAND;

    // Encode the payload data
    PayloadData payloadData = PayloadData_init_default;
    payloadData.deviceNumber = espNumber;
    payloadData.commandName.funcs.encode = &encode_command_name;
    payloadData.commandName.arg = (void *)commandName;
    payloadData.value = value;

    message.payload.arg = &payloadData;
    message.payload.funcs.encode = &encodePayloadData;

    return pb_encode(stream, Message_fields, &message);
}



int decodeMessage(const uint8_t* buffer, size_t bytesRead, PayloadData* payloadData, int* ackStatus) {
    // Parse the received message
    Message message = Message_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(buffer, bytesRead);
    bool status = pb_decode(&stream, Message_fields, &message);

    if (!status) {
        return 1; // Failed to decode the message
    }

    switch (message.cmdType) {
        case Message_CommandType_COMMAND:
            // Validate the payload
            if (!message.payload.arg) {
                return 1; // Invalid payload data
            }

            // Copy the payload data to the provided structure
            *payloadData = *(PayloadData *)message.payload.arg;
            return 0; // Command decoding successful

        case Message_CommandType_ACK:
            if (!message.payload.arg) {
                return 11; // Invalid ACK payload data
            }
            PayloadData ackPayload;
            ackPayload = *(PayloadData *)message.payload.arg;
            // Copy the ACK payload data to the provided structure
            //*ackStatus = message.payload.ok;
            return 10; // ACK decoding successful

        case Message_CommandType_REQ:
            // Handle request message here (if required)
            return 20; // Not implemented

        default:
            return 50; // Invalid command type
    }
}





