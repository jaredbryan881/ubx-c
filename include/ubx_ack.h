#ifndef UBX_ACK_H
#define UBX_ACK_H

#include <stdint.h>
#include "ubx_parser.h"

#define UBX_ACK_CLASS          0x05U
#define UBX_ACK_NAK_ID         0x00U
#define UBX_ACK_ACK_ID         0x01U
#define UBX_ACK_PAYLOAD_LENGTH 2U

typedef enum {
	UBX_ACK_TYPE_NAK = 0,
	UBX_ACK_TYPE_ACK
} ubx_ack_type_t;

typedef struct {
	ubx_ack_type_t type; // ACK and NAK have the same payload layout, so we need the type
	
	uint8_t acknowledged_class;
	uint8_t acknowledged_id;
} ubx_ack_t;

typedef enum {
	UBX_ACK_DECODE_OK = 0,
	UBX_ACK_DECODE_NULL_ARGUMENT,
	UBX_ACK_DECODE_WRONG_CLASS,
	UBX_ACK_DECODE_WRONG_ID,
	UBX_ACK_DECODE_WRONG_LENGTH
} ubx_ack_decode_result_t;

ubx_ack_decode_result_t ubx_ack_decode(const ubx_frame_t *frame, ubx_ack_t *ack);

#endif
