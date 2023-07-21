#ifndef PB_TOOLS_H
#define PB_TOOLS_H

#include <pb_encode.h>
#include <pb_decode.h>
#include <string>

bool encode_string(pb_ostream_t* stream, const pb_field_t* field, void* const* arg);
bool decode_string(pb_istream_t* stream, const pb_field_t* field, void** arg);
bool decode_string_callback(pb_istream_t* stream, const pb_field_t* field, void** arg);


#endif  // PB_TOOLS_H