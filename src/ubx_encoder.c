#include "ubx_encoder.h"
#include "ubx_internal.h"
#include <stdint.h>

#define UBX_ENCODER_SYNC_CHAR_1 0xB5U
#define UBX_ENCODER_SYNC_CHAR_2 0x62U

#define UBX_ENCODER_HEADER_LENGTH   6U
#define UBX_ENCODER_CHECKSUM_LENGTH 2U

ubx_encode_result_t ubx_encode_frame(uint8_t message_class,
									 uint8_t message_id,
									 const uint8_t *payload,
									 size_t payload_length,
									 uint8_t *output,
									 size_t output_capacity,
									 size_t *output_length){
	size_t required_length;
	size_t index;
	uint8_t checksum_a = 0U;
	uint8_t checksum_b = 0U;

	if (output_length == NULL){
		return UBX_ENCODE_NULL_ARGUMENT;
	}

	*output_length = 0U;

	if (output == NULL){
		return UBX_ENCODE_NULL_ARGUMENT;
	}

	if ((payload_length > 0U) && (payload == NULL)){
		return UBX_ENCODE_PAYLOAD_REQUIRED;
	}

	if (payload_length > (size_t)UINT16_MAX)
	{
		return UBX_ENCODE_PAYLOAD_TOO_LARGE;
	}

	required_length = payload_length + UBX_ENCODED_FRAME_OVERHEAD;

	if (output_capacity < required_length){
		return UBX_ENCODE_BUFFER_TOO_SMALL;
	}

	// Fixed UBX header
	output[0] = UBX_ENCODER_SYNC_CHAR_1;
	output[1] = UBX_ENCODER_SYNC_CHAR_2;
	output[2] = message_class;
	output[3] = message_id;

	// Little-endian format for multi-byte values
	output[4] = (uint8_t)(payload_length & 0xFFU);
	output[5] = (uint8_t)((payload_length >> 8) & 0xFFU);

	// Copy payload
	for (index = 0U; index < payload_length; index++){
		output[UBX_ENCODER_HEADER_LENGTH + index] = payload[index];
	}

	// Calculate checksum
	for (index = 2U; index < (UBX_ENCODER_HEADER_LENGTH + payload_length); index++){
		ubx_checksum_update(&checksum_a, &checksum_b, output[index]);
	}

	output[UBX_ENCODER_HEADER_LENGTH + payload_length] = checksum_a;
	output[UBX_ENCODER_HEADER_LENGTH + payload_length + 1U] = checksum_b;

	*output_length = required_length;

	return UBX_ENCODE_OK;
}