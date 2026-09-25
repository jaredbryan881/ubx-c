#ifndef UBX_CFG_H
#define UBX_CFG_H

#include <stddef.h>
#include <stdint.h>

#define UBX_CFG_CLASS     0x06U
#define UBX_CFG_VALSET_ID 0x8AU

#define UBX_CFG_VALSET_VERSION_0     0x00U
#define UBX_CFG_VALSET_HEADER_LENGTH 4U

#define UBX_CFG_LAYER_MASK  0x07U
#define UBX_CFG_LAYER_RAM   0x01U
#define UBX_CFG_LAYER_BBR   0x02U
#define UBX_CFG_LAYER_FLASH 0x04U

// Output rate of UBX-NAV-PVT on the GNSS receiver's UART1 interface
// Type U1, key 0x20910007 = 1 output message per nav solution
#define UBX_CFG_KEY_MSGOUT_NAV_PVT_UART1 UINT32_C(0x20910007)

typedef enum {
	UBX_CFG_BUILD_OK = 0,
	UBX_CFG_BUILD_NULL_ARGUMENT,
	UBX_CFG_BUILD_INVALID_LAYER,
	UBX_CFG_BUILD_INVALID_STATE,
	UBX_CFG_BUILD_KEY_TYPE_MISMATCH,
	UBX_CFG_BUILD_BUFFER_TOO_SMALL
} ubx_cfg_build_result_t;

typedef struct {
	uint8_t *buffer;
	size_t capacity;
	size_t length;
} ubx_cfg_valset_builder_t;

// Start a UBX-CFG-VALSET payload
ubx_cfg_build_result_t ubx_cfg_valset_begin(ubx_cfg_valset_builder_t *builder, uint8_t *buffer, size_t capacity, uint8_t layers);

// Append one config key
ubx_cfg_build_result_t ubx_cfg_valset_add_u1(ubx_cfg_valset_builder_t *builder, uint32_t key, uint8_t value);

#endif