/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.7 */

#ifndef PB_MYPACKAGE_MESSAGE_PB_H_INCLUDED
#define PB_MYPACKAGE_MESSAGE_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _mypackage_Message_CommandType {
    mypackage_Message_CommandType_COMMAND = 0,
    mypackage_Message_CommandType_ACK = 1,
    mypackage_Message_CommandType_REQ = 2
} mypackage_Message_CommandType;

/* Struct definitions */
typedef struct _mypackage_Message {
    uint32_t cmdType;
    uint32_t msgSize;
    uint32_t seqNum;
    pb_callback_t payload;
    uint32_t msgEnd;
} mypackage_Message;


#ifdef __cplusplus
extern "C" {
#endif

/* Helper constants for enums */
#define _mypackage_Message_CommandType_MIN mypackage_Message_CommandType_COMMAND
#define _mypackage_Message_CommandType_MAX mypackage_Message_CommandType_REQ
#define _mypackage_Message_CommandType_ARRAYSIZE ((mypackage_Message_CommandType)(mypackage_Message_CommandType_REQ+1))



/* Initializer values for message structs */
#define mypackage_Message_init_default           {0, 0, 0, {{NULL}, NULL}, 0}
#define mypackage_Message_init_zero              {0, 0, 0, {{NULL}, NULL}, 0}

/* Field tags (for use in manual encoding/decoding) */
#define mypackage_Message_cmdType_tag            1
#define mypackage_Message_msgSize_tag            2
#define mypackage_Message_seqNum_tag             3
#define mypackage_Message_payload_tag            4
#define mypackage_Message_msgEnd_tag             5

/* Struct field encoding specification for nanopb */
#define mypackage_Message_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   cmdType,           1) \
X(a, STATIC,   SINGULAR, UINT32,   msgSize,           2) \
X(a, STATIC,   SINGULAR, UINT32,   seqNum,            3) \
X(a, CALLBACK, SINGULAR, STRING,   payload,           4) \
X(a, STATIC,   SINGULAR, UINT32,   msgEnd,            5)
#define mypackage_Message_CALLBACK pb_default_field_callback
#define mypackage_Message_DEFAULT NULL

extern const pb_msgdesc_t mypackage_Message_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define mypackage_Message_fields &mypackage_Message_msg

/* Maximum encoded size of messages (where known) */
/* mypackage_Message_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
