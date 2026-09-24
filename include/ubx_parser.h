#ifndef UBX_PARSER_H
#define UBX_PARSER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UBX_SYNC_CHAR_1 0xB5U
#define UBX_SYNC_CHAR_2 0x62U

// Results of passing one byte to ubx_parser_feed
typedef enum {
	UBX_PARSE_IN_PROGRESS = 0, // byte was consumed, but the frame is not finished
	UBX_PARSE_FRAME_COMPLETE, // complete frame is available, valid checksum
	UBX_PARSE_CHECKSUM_ERROR, // frame ended, but the checksum was invalid
	UBX_PARSE_OVERSIZE // payload is larger than the available buffer
} ubx_parse_result_t;

// Internal state of the byte-by-byte parser
typedef enum {
	UBX_PARSER_SYNC_1 = 0,
	UBX_PARSER_SYNC_2,
	UBX_PARSER_CLASS,
	UBX_PARSER_ID,
	UBX_PARSER_LENGTH_1,
	UBX_PARSER_LENGTH_2,
	UBX_PARSER_PAYLOAD,
	UBX_PARSER_CHECKSUM_A,
	UBX_PARSER_CHECKSUM_B,
	UBX_PARSER_DISCARD
} ubx_parser_state_t;

// Details of a successful frame per the UBX structure
typedef struct {
	uint8_t message_class;
	uint8_t message_id;
	uint16_t payload_length;
	const uint8_t *payload; // payload pointer refers to the buffer given to ubx_parser_init
} ubx_frame_t;

// State of the UBX byte stream
typedef struct {
	ubx_parser_state_t state;

	uint8_t *payload_buffer;
	size_t payload_capacity;
	size_t payload_index;

	uint8_t message_class;
	uint8_t message_id;
	uint16_t payload_length;

	uint8_t checksum_a;
	uint8_t checksum_b;
	uint8_t received_checksum_a;

	size_t discard_remaining;
} ubx_parser_t;

// Initialize the UBX parser. It's up to whoever calls this to maintain a valid buffer.
void ubx_parser_init(ubx_parser_t *parser,
					 uint8_t *payload_buffer,
					 size_t payload_capacity);

// Reset the parser to its initialization state.
void ubx_parser_reset(ubx_parser_t *parser);

// Pass one byte from a UBX byte stream to the parser.
ubx_parse_result_t ubx_parser_feed(ubx_parser_t *parser,
									uint8_t byte,
									ubx_frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif
