#ifndef UBX_INTERNAL_H
#define UBX_INTERNAL_H

#include <stdint.h>

// Add one byte to the UBX 8-bit Fletcher checksum
// Checksum does not include the two sync characters or the checksum bytes themselves
static inline void ubx_checksum_update(uint8_t *checksum_a, uint8_t *checksum_b, uint8_t byte){
	*checksum_a = (uint8_t)(*checksum_a + byte);
	*checksum_b = (uint8_t)(*checksum_b + *checksum_a);
}

// Read unsigned little-endian integers from an unaligned byte buffer
static inline uint16_t ubx_read_u16_le(const uint8_t *data){
	return (uint16_t)(((uint16_t)data[0]) | 
					  ((uint16_t)data[1] << 8U));
}

static inline uint32_t ubx_read_u32_le(const uint8_t *data){
	return ((uint32_t)data[0]) |
		   ((uint32_t)data[1] << 8U) |
		   ((uint32_t)data[2] << 16U) |
		   ((uint32_t)data[3] << 24U);
}

// Convert the little-endian bit pattern to a signed value
static inline int16_t ubx_read_i16_le(const uint8_t *data){
	uint16_t value = ubx_read_u16_le(data);

	if (value <= INT16_MAX){
		return (int16_t)value;
	}

	return (int16_t)((int32_t)value - 65536L);
}

static inline int32_t ubx_read_i32_le(const uint8_t *data){
	uint32_t value = ubx_read_u32_le(data);

	if (value <= INT32_MAX){
		return (int32_t)value;
	}

	return (int32_t)((int64_t)value - 4294967296LL);
}

// Write unsigned integers into an unaligned little-endian byte buffer
static inline void ubx_write_u16_le(uint8_t *data, uint32_t value){
	data[0] = (uint8_t)(value);
	data[1] = (uint8_t)(value >> 8U);
}

static inline void ubx_write_u32_le(uint8_t *data, uint32_t value){
	data[0] = (uint8_t)(value);
	data[1] = (uint8_t)(value >> 8U);
	data[2] = (uint8_t)(value >> 16U);
	data[3] = (uint8_t)(value >> 24U);
}

#endif