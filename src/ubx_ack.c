#include "ubx_ack.h"

ubx_ack_decode_result_t ubx_ack_decode(const ubx_frame_t *frame, ubx_ack_t *ack){
	if ((frame == NULL) || (ack == NULL)){
		return UBX_ACK_DECODE_NULL_ARGUMENT;
	}

	if (frame->message_class != UBX_ACK_CLASS){
		return UBX_ACK_DECODE_WRONG_CLASS;
	}

	if (frame->payload_length != UBX_ACK_PAYLOAD_LENGTH){
		return UBX_ACK_DECODE_WRONG_LENGTH;
	}

	switch (frame->message_id){
		case UBX_ACK_ACK_ID:
			ack->type = UBX_ACK_TYPE_ACK;
			break;

		case UBX_ACK_NAK_ID:
			ack->type = UBX_ACK_TYPE_NAK;
			break;

		default:
			return UBX_ACK_DECODE_WRONG_ID;
	}

	ack->acknowledged_class = frame->payload[0];
	ack->acknowledged_id    = frame->payload[1];

	return UBX_ACK_DECODE_OK;
}
