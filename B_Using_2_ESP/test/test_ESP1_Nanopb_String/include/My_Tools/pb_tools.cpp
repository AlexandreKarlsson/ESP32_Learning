#include <pb_encode.h>
#include <pb_decode.h>
#include "pb_tools.h"
#include <string>

bool encode_string(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    /*
    Cette ligne convertit le pointeur arg en un pointeur constant de type std::string. Ainsi, nous pouvons accéder à la chaîne de caractères à encoder.
    */
    const std::string* str = static_cast<const std::string*>(*arg);

    /*
    Cette ligne encode le tag du champ dans le flux. Le tag identifie le champ dans le message protobuf.
    */
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    /*
    Cette ligne encode la chaîne de caractères dans le flux. Nous utilisons pb_encode_string pour effectuer l'encodage, en fournissant le flux, un pointeur vers les données de la chaîne de caractères et la taille de la chaîne de caractères.
    */
    return pb_encode_string(stream, reinterpret_cast<const uint8_t*>(str->c_str()), str->length());
}

bool decode_string(pb_istream_t* stream, const pb_field_t* field, void** arg)
{
    std::string* str = static_cast<std::string*>(*arg); // Conversion du pointeur arg en pointeur de std::string

    pb_byte_t* buffer = new pb_byte_t[field->data_size];
    size_t count = stream->bytes_left;

    if (!pb_read(stream, buffer, count)) {
        delete[] buffer;
        return false;
    }

    str->assign(reinterpret_cast<const char*>(buffer), count);
    delete[] buffer;

    return true;
}

bool decode_string_callback(pb_istream_t* stream, const pb_field_t* field, void** arg)
{
    std::string* str = reinterpret_cast<std::string*>(*arg);
    pb_byte_t buffer[256];
    size_t count = sizeof(buffer);

    if (!pb_read(stream, buffer, count)) {
        return false;
    }

    str->assign(reinterpret_cast<const char*>(buffer), count);

    return true;
}








