#include "ubx_rxm.h"
#include "ubx_internal.h"

ubx_rxm_pmreq_build_result_t ubx_rxm_pmreq_build_backup(const ubx_rxm_pmreq_backup_t *request,
														uint8_t *payload,
														size_t capacity,
														size_t *payload_length){
	uint32_t flags;

	if (payload_length == NULL){
		return UBX_RXM_PMREQ_BUILD_NULL_ARGUMENT;
	}

	*payload_length = 0U;

	if ((request == NULL) || (payload == NULL)){
		return UBX_RXM_PMREQ_BUILD_NULL_ARGUMENT;
	}

	if (capacity < UBX_RXM_PMREQ_PAYLOAD_LENGTH){
		return UBX_RXM_PMREQ_BUILD_BUFFER_TOO_SMALL;
	}

	if (request->duration > UBX_RXM_PMREQ_MAX_DURATION){
		return UBX_RXM_PMREQ_BUILD_INVALID_DURATION;
	}

	if (request->force > 1U){
		return UBX_RXM_PMREQ_BUILD_INVALID_FORCE;
	}

	if ((request->wakeup_sources & (uint32_t)~UBX_RXM_PMREQ_WAKEUP_MASK) != 0U){
		return UBX_RXM_PMREQ_BUILD_INVALID_WAKEUP_SOURCES;
	}

	flags = UBX_RXM_PMREQ_FLAG_BACKUP;

	if (request->force != 0U){
		flags |= UBX_RXM_PMREQ_FLAG_FORCE;
	}

	payload[0] = UBX_RXM_PMREQ_VERSION_0;
	payload[1] = 0U; // reserved
	payload[2] = 0U; // reserved
	payload[3] = 0U; // reserved

	// Multi-byte PMREQ
	ubx_write_u32_le(&payload[4], request->duration);
	ubx_write_u32_le(&payload[8], flags);
	ubx_write_u32_le(&payload[12], request->wakeup_sources);

	*payload_length = UBX_RXM_PMREQ_PAYLOAD_LENGTH;

	return UBX_RXM_PMREQ_BUILD_OK;
}

// RAWX validation
static ubx_rxm_decode_result_t ubx_rxm_rawx_validate(const ubx_frame_t *frame){
	size_t expected;

	if ((frame == NULL) || (frame->payload == NULL)){
		return UBX_RXM_DECODE_NULL_ARGUMENT;
	}

	if ((frame->message_class != UBX_RXM_CLASS) || (frame->message_id != UBX_RXM_RAWX_ID)){
		return UBX_RXM_DECODE_WRONG_MESSAGE;
	}

	if (frame->payload_length < UBX_RXM_RAWX_HEADER_LENGTH){
		return UBX_RXM_DECODE_WRONG_LENGTH;
	}

	if (frame->payload[13] != UBX_RXM_RAWX_VERSION_1){
		return UBX_RXM_DECODE_UNSUPPORTED_VERSION;
	}

	expected = (size_t)UBX_RXM_RAWX_HEADER_LENGTH + (size_t)frame->payload[11] * UBX_RXM_RAWX_MEASUREMENT_LENGTH;

	if ((size_t)frame->payload_length != expected){
		return UBX_RXM_DECODE_WRONG_LENGTH;
	}

	return UBX_RXM_DECODE_OK;
}

ubx_rxm_decode_result_t ubx_rxm_rawx_decode(const ubx_frame_t *frame, ubx_rxm_rawx_t *output){
	ubx_rxm_decode_result_t result;
	const uint8_t *p;
	
	if (output == NULL){
		return UBX_RXM_DECODE_NULL_ARGUMENT;
	}
	
	result = ubx_rxm_rawx_validate(frame);
	
	if (result != UBX_RXM_DECODE_OK){
		return result;
	}
	
	p = frame->payload;

	output->receiver_tow = ubx_read_r8_le(p);
	output->week = ubx_read_u16_le(p + 8U);
	output->leap_seconds = ubx_read_i8(p + 10U);
	output->measurement_count = p[11];
	output->receiver_status = p[12];
	output->version = p[13];

	return UBX_RXM_DECODE_OK;
}
ubx_rxm_decode_result_t ubx_rxm_rawx_measurement_decode(const ubx_frame_t *frame, uint16_t measurement_index, ubx_rxm_rawx_measurement_t *output) {
	ubx_rxm_decode_result_t result;
	const uint8_t *p;
	
	if (output == NULL){
		return UBX_RXM_DECODE_NULL_ARGUMENT;
	}
	
	result = ubx_rxm_rawx_validate(frame);
	
	if (result != UBX_RXM_DECODE_OK){
		return result;
	}
	
	if (measurement_index >= frame->payload[11]){
		return UBX_RXM_DECODE_INDEX_OUT_OF_RANGE;
	}
	
	p = frame->payload + UBX_RXM_RAWX_HEADER_LENGTH + (size_t)measurement_index * UBX_RXM_RAWX_MEASUREMENT_LENGTH;
	
	output->pseudorange = ubx_read_r8_le(p);
	output->carrier_phase = ubx_read_r8_le(p + 8U);
	output->doppler = ubx_read_r4_le(p + 16U);
	output->gnss_id = p[20];
	output->satellite_id = p[21];
	output->signal_id = p[22];
	output->frequency_id = p[23];
	output->lock_time = ubx_read_u16_le(p + 24U);
	output->cno = p[26];
	output->pseudorange_stdev = p[27];
	output->carrier_phase_stdev = p[28];
	output->doppler_stdev = p[29];
	output->tracking_status = p[30];

	return UBX_RXM_DECODE_OK;
}

double ubx_rxm_rawx_pseudorange_stdev_m(uint8_t raw){
	return 0.01 * (double)(UINT32_C(1) << (raw & UBX_RXM_RAWX_STDEV_INDEX_MASK));
}

double ubx_rxm_rawx_doppler_stdev_hz(uint8_t raw){
	return 0.002 * (double)(UINT32_C(1) << (raw & UBX_RXM_RAWX_STDEV_INDEX_MASK));
}

ubx_rxm_decode_result_t ubx_rxm_rawx_carrier_phase_stdev_cycles(uint8_t raw, double *cycles){
	uint8_t index = raw & UBX_RXM_RAWX_STDEV_INDEX_MASK;

	if (cycles == NULL){
		return UBX_RXM_DECODE_NULL_ARGUMENT;
	}

	if (index == UBX_RXM_RAWX_CP_STDEV_INVALID){
		return UBX_RXM_DECODE_INVALID_VALUE;
	}
	
	*cycles = 0.004 * (double)index;
	
	return UBX_RXM_DECODE_OK;
}

// SFRBX validation
static ubx_rxm_decode_result_t ubx_rxm_sfrbx_validate(const ubx_frame_t *frame){
	size_t expected;

	if ((frame == NULL) || (frame->payload == NULL)){
		return UBX_RXM_DECODE_NULL_ARGUMENT;
	}

	if ((frame->message_class != UBX_RXM_CLASS) || (frame->message_id != UBX_RXM_SFRBX_ID)){
		return UBX_RXM_DECODE_WRONG_MESSAGE;
	}

	if (frame->payload_length < UBX_RXM_SFRBX_HEADER_LENGTH){
		return UBX_RXM_DECODE_WRONG_LENGTH;
	}

	if (frame->payload[6] != UBX_RXM_SFRBX_VERSION_2){
		return UBX_RXM_DECODE_UNSUPPORTED_VERSION;
	}

	expected = (size_t)UBX_RXM_SFRBX_HEADER_LENGTH + (size_t)frame->payload[4] * UBX_RXM_SFRBX_WORD_LENGTH;
	
	if ((size_t)frame->payload_length != expected){
		return UBX_RXM_DECODE_WRONG_LENGTH;
	}

	return UBX_RXM_DECODE_OK;
}

// Read the satellite, signal, and word-count
ubx_rxm_decode_result_t ubx_rxm_sfrbx_decode(const ubx_frame_t *frame, ubx_rxm_sfrbx_t *output){
	ubx_rxm_decode_result_t result;
	const uint8_t *data;

	if (output == NULL){
		return UBX_RXM_DECODE_NULL_ARGUMENT;
	}

	result = ubx_rxm_sfrbx_validate(frame);
	
	if (result != UBX_RXM_DECODE_OK){
		return result;
	}

	data = frame->payload;
	output->gnss_id = data[0];
	output->satellite_id = data[1];
	output->signal_id = data[2];
	output->frequency_id = data[3];
	output->word_count = data[4];
	output->channel = data[5];
	output->version = data[6];

	return UBX_RXM_DECODE_OK;
}

// Read one navigation word
ubx_rxm_decode_result_t ubx_rxm_sfrbx_word_decode(const ubx_frame_t *frame,
												  uint16_t word_index,
												  uint32_t *output){
	ubx_rxm_decode_result_t result;
	const uint8_t *data;

	if (output == NULL){
		return UBX_RXM_DECODE_NULL_ARGUMENT;
	}

	result = ubx_rxm_sfrbx_validate(frame);

	if (result != UBX_RXM_DECODE_OK){
		return result;
	}

	if (word_index >= frame->payload[4]){
		return UBX_RXM_DECODE_INDEX_OUT_OF_RANGE;
	}

	data = frame->payload + UBX_RXM_SFRBX_HEADER_LENGTH + (size_t)word_index * UBX_RXM_SFRBX_WORD_LENGTH;
	*output = ubx_read_u32_le(data);

	return UBX_RXM_DECODE_OK;
}
