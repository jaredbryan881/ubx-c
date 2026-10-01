#ifndef UBX_INTERNAL_H
#define UBX_INTERNAL_H

#include <stdint.h>
#include <string.h>

// Add one byte to the UBX 8-bit Fletcher checksum.
static inline void ubx_checksum_update(uint8_t *checksum_a, uint8_t *checksum_b,
                                       uint8_t byte) {
  *checksum_a = (uint8_t)(*checksum_a + byte);
  *checksum_b = (uint8_t)(*checksum_b + *checksum_a);
}

static inline uint16_t ubx_read_u16_le(const uint8_t *data) {
  return (uint16_t)(((uint16_t)data[0]) | ((uint16_t)data[1] << 8U));
}

static inline uint32_t ubx_read_u32_le(const uint8_t *data) {
  return ((uint32_t)data[0]) | ((uint32_t)data[1] << 8U) |
         ((uint32_t)data[2] << 16U) | ((uint32_t)data[3] << 24U);
}

static inline uint64_t ubx_read_u64_le(const uint8_t *data) {
  return ((uint64_t)data[0]) | ((uint64_t)data[1] << 8U) |
         ((uint64_t)data[2] << 16U) | ((uint64_t)data[3] << 24U) |
         ((uint64_t)data[4] << 32U) | ((uint64_t)data[5] << 40U) |
         ((uint64_t)data[6] << 48U) | ((uint64_t)data[7] << 56U);
}

static inline float ubx_read_r4_le(const uint8_t *data) {
  uint32_t bits = ubx_read_u32_le(data);
  float value;
  (void)memcpy(&value, &bits, sizeof(value));
  return value;
}

static inline double ubx_read_r8_le(const uint8_t *data) {
  uint64_t bits = ubx_read_u64_le(data);
  double value;
  (void)memcpy(&value, &bits, sizeof(value));
  return value;
}

static inline int16_t ubx_read_i16_le(const uint8_t *data) {
  uint16_t value = ubx_read_u16_le(data);

  if (value <= (uint16_t)INT16_MAX) {
    return (int16_t)value;
  }

  return (int16_t)((int32_t)value - 65536L);
}

static inline int32_t ubx_read_i32_le(const uint8_t *data) {
  uint32_t value = ubx_read_u32_le(data);

  if (value <= (uint32_t)INT32_MAX) {
    return (int32_t)value;
  }

  return (int32_t)((int64_t)value - 4294967296LL);
}

static inline void ubx_write_u16_le(uint8_t *data, uint16_t value) {
  data[0] = (uint8_t)value;
  data[1] = (uint8_t)(value >> 8U);
}

static inline void ubx_write_u32_le(uint8_t *data, uint32_t value) {
  data[0] = (uint8_t)value;
  data[1] = (uint8_t)(value >> 8U);
  data[2] = (uint8_t)(value >> 16U);
  data[3] = (uint8_t)(value >> 24U);
}

static inline int8_t ubx_read_i8(const uint8_t *data) {
  uint8_t value = data[0];

  if (value <= (uint8_t)INT8_MAX) {
    return (int8_t)value;
  }

  return (int8_t)((int16_t)value - 256);
}

#endif
