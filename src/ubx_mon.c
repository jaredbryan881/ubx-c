#include "ubx_mon.h"
#include "ubx_internal.h"

ubx_mon_decode_result_t ubx_mon_rxr_decode(const ubx_frame_t *frame, ubx_mon_rxr_t *output){
	if ((frame == NULL) || (output == NULL)){
		return UBX_MON_DECODE_NULL_ARGUMENT;
	}

	if ((frame->message_class != UBX_MON_CLASS) || (frame->message_id != UBX_MON_RXR_ID)){
		return UBX_MON_DECODE_WRONG_MESSAGE;
	}

	if (frame->payload_length != UBX_MON_RXR_PAYLOAD_LENGTH){
		return UBX_MON_DECODE_WRONG_LENGTH;
	}

	if (frame->payload == NULL){
		return UBX_MON_DECODE_NULL_ARGUMENT;
	}

	output->flags = frame->payload[0];

	return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t ubx_mon_sys_decode(const ubx_frame_t *frame, ubx_mon_sys_t *output){
	const uint8_t *payload;

	if ((frame == NULL) || (output == NULL)){
		return UBX_MON_DECODE_NULL_ARGUMENT;
	}

	if ((frame->message_class != UBX_MON_CLASS) || (frame->message_id != UBX_MON_SYS_ID)){
		return UBX_MON_DECODE_WRONG_MESSAGE;
	}

	if (frame->payload_length != UBX_MON_SYS_PAYLOAD_LENGTH){
		return UBX_MON_DECODE_WRONG_LENGTH;
	}

	if (frame->payload == NULL){
		return UBX_MON_DECODE_NULL_ARGUMENT;
	}

	payload = frame->payload;

	if (payload[0] != UBX_MON_SYS_VERSION_1){
		return UBX_MON_DECODE_UNSUPPORTED_VERSION;
	}

	output->version = payload[0];
	output->boot_type = payload[1];

	output->cpu_load = payload[2];
	output->cpu_load_max = payload[3];

	output->memory_usage = payload[4];
	output->memory_usage_max = payload[5];

	output->io_usage = payload[6];
	output->io_usage_max = payload[7];

	output->run_time = ubx_read_u32_le(&payload[8]);
	output->notice_count = ubx_read_u16_le(&payload[12]);
	output->warning_count = ubx_read_u16_le(&payload[14]);
	output->error_count = ubx_read_u16_le(&payload[16]);
	output->temperature = ubx_read_i8(&payload[18]);

	return UBX_MON_DECODE_OK;
}