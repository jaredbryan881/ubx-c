#include "ubx_parser.h"
#include "ubx_internal.h"

// Prepare to receive a UBX frame
static void ubx_start_frame(ubx_parser_t *parser){
	parser->state = UBX_PARSER_CLASS;

	parser->payload_index = 0U;

	parser->message_class = 0U;
	parser->message_id = 0U;
	parser->payload_length = 0U;

	parser->checksum_a = 0U;
	parser->checksum_b = 0U;
	parser->received_checksum_a = 0U;

	parser->discard_remaining = 0U;
}

// Initialize the UBX parser
void ubx_parser_init(ubx_parser_t *parser, uint8_t *payload_buffer, size_t payload_capacity){
	if (parser == NULL){
		return;
	}

	parser->payload_buffer = payload_buffer;
	
	// Regardless of what payload_capacity is claimed to be, a NULL buffer is treated as having zero capacity
	parser->payload_capacity = (payload_buffer != NULL) ? payload_capacity : 0U;
	
	ubx_parser_reset(parser);
}

// Reset everything except payload_buffer and payload_capacity, which we leave to whoever called this.
void ubx_parser_reset(ubx_parser_t *parser){
	if (parser == NULL){
		return;
	}

	parser->state = UBX_PARSER_SYNC_1;

	parser->payload_index = 0U;

	parser->message_class = 0U;
	parser->message_id = 0U;
	parser->payload_length = 0U;

	parser->checksum_a = 0U;
	parser->checksum_b = 0U;
	parser->received_checksum_a = 0U;

	parser->discard_remaining = 0U;
}

ubx_parse_result_t ubx_parser_feed(ubx_parser_t *parser, uint8_t byte, ubx_frame_t *frame){
	if (parser == NULL){
		return UBX_PARSE_IN_PROGRESS;
	}

	switch (parser->state){
		case UBX_PARSER_SYNC_1:
			// First sync character
			if (byte == UBX_SYNC_CHAR_1){
				// Continue to second sync character
				parser->state = UBX_PARSER_SYNC_2;
			}
			break;

		case UBX_PARSER_SYNC_2:
			// Second sync character
			if (byte == UBX_SYNC_CHAR_2){
				// Continue with reading the UBX frame
				ubx_start_frame(parser);
			}
			// Not the expected second sync character
			// Reset parser state to expect the first sync character
			// unless we just received that in which case say expecting the second sync character
			else if (byte != UBX_SYNC_CHAR_1){
				parser->state = UBX_PARSER_SYNC_1;
			}
			break;

		case UBX_PARSER_CLASS:
			parser->message_class = byte;
			ubx_checksum_update(&parser->checksum_a, &parser->checksum_b, byte);
			parser->state = UBX_PARSER_ID;
			break;

		case UBX_PARSER_ID:
			parser->message_id = byte;
			ubx_checksum_update(&parser->checksum_a, &parser->checksum_b, byte);
			parser->state = UBX_PARSER_LENGTH_1;
			break;

		case UBX_PARSER_LENGTH_1:
			// Low byte (UBX uses little-endian integers)
			parser->payload_length = (uint16_t)byte;
			ubx_checksum_update(&parser->checksum_a, &parser->checksum_b, byte);
			parser->state = UBX_PARSER_LENGTH_2;
			break;

		case UBX_PARSER_LENGTH_2:
			// High byte (UBX uses little-endian integers)
			parser->payload_length |= (uint16_t)((uint16_t)byte << 8);
			ubx_checksum_update(&parser->checksum_a, &parser->checksum_b, byte);

			// Discard the payload plus the checksum bytes
			if ((size_t)parser->payload_length > parser->payload_capacity){
				parser->discard_remaining = (size_t)parser->payload_length + 2U;
				parser->state = UBX_PARSER_DISCARD;

				return UBX_PARSE_OVERSIZE;
			}

			// No payload, skip to checksum
			if (parser->payload_length == 0U){
				parser->state = UBX_PARSER_CHECKSUM_A;
			}
			// Continue to payload
			else{
				parser->payload_index = 0U;
				parser->state = UBX_PARSER_PAYLOAD;
			}
			break;

		case UBX_PARSER_PAYLOAD:
			parser->payload_buffer[parser->payload_index] = byte;
			parser->payload_index++;
			ubx_checksum_update(&parser->checksum_a, &parser->checksum_b, byte);

			if (parser->payload_index >= (size_t)parser->payload_length){
				parser->state = UBX_PARSER_CHECKSUM_A;
			}
			break;

		case UBX_PARSER_CHECKSUM_A:
			parser->received_checksum_a = byte;
			parser->state = UBX_PARSER_CHECKSUM_B;
			break;

		case UBX_PARSER_CHECKSUM_B:
			if ((parser->received_checksum_a == parser->checksum_a) && (byte == parser->checksum_b)){
				if (frame != NULL){
					frame->message_class = parser->message_class;
					frame->message_id = parser->message_id;
					frame->payload_length = parser->payload_length;
					frame->payload = parser->payload_buffer;
				}

				ubx_parser_reset(parser);
				return UBX_PARSE_FRAME_COMPLETE;
			}

			ubx_parser_reset(parser);
			return UBX_PARSE_CHECKSUM_ERROR;

		case UBX_PARSER_DISCARD:
			if (parser->discard_remaining > 0U){
				parser->discard_remaining--;
			}

			if (parser->discard_remaining == 0U){
				ubx_parser_reset(parser);
			}
			break;

		default:
			// Invalid state
			ubx_parser_reset(parser);
			break;
	}

	return UBX_PARSE_IN_PROGRESS;
}
