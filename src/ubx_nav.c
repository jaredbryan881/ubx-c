#include "ubx_nav.h"
#include <stddef.h>
#include <stdint.h>

// Read unsigned little-endian integers from an unaligned byte buffer
static uint16_t ubx_read_u16_le(const uint8_t *data){
	return (uint16_t)(((uint16_t)data[0]) | 
					  ((uint16_t)data[1] << 8));
}

static uint32_t ubx_read_u32_le(const uint8_t *data){
	return ((uint32_t)data[0]) |
		   ((uint32_t)data[1] << 8) |
		   ((uint32_t)data[2] << 16) |
		   ((uint32_t)data[3] << 24);
}

// Convert the little-endian bit pattern to a signed value
static int16_t ubx_read_i16_le(const uint8_t *data){
	uint16_t value = ubx_read_u16_le(data);

	if (value <= INT16_MAX){
		return (int16_t)value;
	}

	return (int16_t)((int32_t)value - 65536L);
}

static int32_t ubx_read_i32_le(const uint8_t *data){
	uint32_t value = ubx_read_u32_le(data);

	if (value <= INT32_MAX){
		return (int32_t)value;
	}

	return (int32_t)((int64_t)value - 4294967296LL);
}

ubx_nav_pvt_decode_result_t ubx_nav_pvt_decode(const ubx_frame_t *frame, ubx_nav_pvt_t *output){
	const uint8_t *payload;

	if ((frame == NULL) || (output == NULL)){
		return UBX_NAV_PVT_DECODE_NULL_ARGUMENT;
	}

	if ((frame->message_class != UBX_NAV_PVT_CLASS) || (frame->message_id != UBX_NAV_PVT_ID)){
		return UBX_NAV_PVT_DECODE_WRONG_MESSAGE;
	}

	if (frame->payload_length != UBX_NAV_PVT_PAYLOAD_LENGTH){
		return UBX_NAV_PVT_DECODE_WRONG_LENGTH;
	}

	if (frame->payload == NULL){
		return UBX_NAV_PVT_DECODE_NULL_ARGUMENT;
	}

	payload = frame->payload;

	output->i_tow                = ubx_read_u32_le(&payload[0]);

	output->year                 = ubx_read_u16_le(&payload[4]);
	output->month                = payload[6];
	output->day                  = payload[7];
	output->hour                 = payload[8];
	output->minute               = payload[9];
	output->second               = payload[10];
	output->valid                = payload[11];

	output->time_accuracy        = ubx_read_u32_le(&payload[12]);
	output->nanosecond           = ubx_read_i32_le(&payload[16]);

	output->fix_type             = payload[20];
	output->flags                = payload[21];
	output->flags2               = payload[22];
	output->satellites_used      = payload[23];

	output->longitude            = ubx_read_i32_le(&payload[24]);
	output->latitude             = ubx_read_i32_le(&payload[28]);
	output->height_ellipsoid     = ubx_read_i32_le(&payload[32]);
	output->height_msl           = ubx_read_i32_le(&payload[36]);
	output->horizontal_accuracy  = ubx_read_u32_le(&payload[40]);
	output->vertical_accuracy    = ubx_read_u32_le(&payload[44]);
	output->velocity_north       = ubx_read_i32_le(&payload[48]);
	output->velocity_east        = ubx_read_i32_le(&payload[52]);
	output->velocity_down        = ubx_read_i32_le(&payload[56]);
	output->ground_speed         = ubx_read_i32_le(&payload[60]);
	output->heading_motion       = ubx_read_i32_le(&payload[64]);
	output->speed_accuracy       = ubx_read_u32_le(&payload[68]);
	output->heading_accuracy     = ubx_read_u32_le(&payload[72]);
	output->p_dop                = ubx_read_u16_le(&payload[76]);
	output->flags3               = ubx_read_u16_le(&payload[78]);
	// Payload bytes 80 through 83 are reserved.
	output->heading_vehicle      = ubx_read_i32_le(&payload[84]);
	output->magnetic_declination = ubx_read_i16_le(&payload[88]);
	output->magnetic_accuracy    = ubx_read_u16_le(&payload[90]);

	return UBX_NAV_PVT_DECODE_OK;
}