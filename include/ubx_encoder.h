#ifndef UBX_ENCODER_H
#define UBX_ENCODER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 2 sync bytes + 1 class byte + 1 message-id byte + 2 payload-length bytes + 2 checksum bytes
#define UBX_ENCODED_FRAME_OVERHEAD 8U

typedef enum {
	UBX_ENCODE_OK = 0,
	UBX_ENCODE_NULL_ARGUMENT,
	UBX_ENCODE_PAYLOAD_REQUIRED,
	UBX_ENCODE_PAYLOAD_TOO_LARGE,
	UBX_ENCODE_BUFFER_TOO_SMALL
} ubx_encode_result_t;

// Encode one full UBX frame
ubx_encode_result_t ubx_encode_frame(
	uint8_t message_class,
	uint8_t message_id,
	const uint8_t *payload,
	size_t payload_length,
	uint8_t *output,
	size_t output_capacity,
	size_t *output_length);

#ifdef __cplusplus
}
#endif

#endif