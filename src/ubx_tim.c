#include "ubx_tim.h"
#include "ubx_internal.h"

ubx_tim_tp_decode_result_t ubx_tim_tp_decode(const ubx_frame_t *frame, ubx_tim_tp_t *output){
	const uint8_t *payload;

	if ((frame == NULL) || (output == NULL)){
		return UBX_TIM_TP_DECODE_NULL_ARGUMENT;
	}

	if ((frame->message_class != UBX_TIM_CLASS) || (frame->message_id != UBX_TIM_TP_ID)){
		return UBX_TIM_TP_DECODE_WRONG_MESSAGE;
	}

	if (frame->payload_length != UBX_TIM_TP_PAYLOAD_LENGTH){
		return UBX_TIM_TP_DECODE_WRONG_LENGTH;
	}

	if (frame->payload == NULL){
		return UBX_TIM_TP_DECODE_NULL_ARGUMENT;
	}

	payload = frame->payload;

	output->tow                = ubx_read_u32_le(&payload[0]);
	output->tow_sub            = ubx_read_u32_le(&payload[4]);
	output->quantization_error = ubx_read_i32_le(&payload[8]);
	output->week               = ubx_read_u16_le(&payload[12]);
	output->flags              = payload[14];
	output->reference_info     = payload[15];

	return UBX_TIM_TP_DECODE_OK;
}
