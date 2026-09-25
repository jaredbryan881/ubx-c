#ifndef UBX_INTERNAL_H
#define UBX_INTERNAL_H

#include <stdint.h>

// Add one byte to the UBX 8-bit Fletcher checksum
// Checksum does not include the two sync characters or the checksum bytes themselves
static inline void ubx_checksum_update(uint8_t *checksum_a, uint8_t *checksum_b, uint8_t byte){
	*checksum_a = (uint8_t)(*checksum_a + byte);
	*checksum_b = (uint8_t)(*checksum_b + *checksum_a);
}

#endif