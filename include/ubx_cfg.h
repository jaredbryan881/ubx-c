#ifndef UBX_CFG_H
#define UBX_CFG_H

#include "ubx_parser.h"
#include <stddef.h>
#include <stdint.h>

#define UBX_CFG_CLASS     0x06U
#define UBX_CFG_VALSET_ID 0x8AU

#define UBX_CFG_VALSET_VERSION_0     0x00U
#define UBX_CFG_VALSET_HEADER_LENGTH 4U

#define UBX_CFG_VALSET_LAYER_MASK  0x07U
#define UBX_CFG_VALSET_LAYER_RAM   0x01U
#define UBX_CFG_VALSET_LAYER_BBR   0x02U
#define UBX_CFG_VALSET_LAYER_FLASH 0x04U

#define UBX_CFG_VALGET_ID               0x8BU
#define UBX_CFG_VALGET_REQUEST_VERSION  0x00U
#define UBX_CFG_VALGET_RESPONSE_VERSION 0x01U
#define UBX_CFG_VALGET_HEADER_LENGTH    4U
#define UBX_CFG_VALGET_KEY_LENGTH       4U
#define UBX_CFG_VALGET_MAX_KEYS         64U

// VALGET uses a layer selector, not a layer bitmask like VALSET
#define UBX_CFG_VALGET_LAYER_DEFAULT 7U
#define UBX_CFG_VALGET_LAYER_RAM     0U
#define UBX_CFG_VALGET_LAYER_BBR     1U
#define UBX_CFG_VALGET_LAYER_FLASH   2U

// Output rate of UBX-NAV-PVT on the GNSS receiver's UART1 interface
// Type U1, key 0x20910007 = 1 output message per nav solution
#define UBX_CFG_KEY_MSGOUT_NAV_PVT_UART1 UINT32_C(0x20910007)

typedef enum {
	UBX_CFG_BUILD_OK = 0,
	UBX_CFG_BUILD_NULL_ARGUMENT,
	UBX_CFG_BUILD_INVALID_LAYER,
	UBX_CFG_BUILD_INVALID_STATE,
	UBX_CFG_BUILD_KEY_TYPE_MISMATCH,
	UBX_CFG_BUILD_BUFFER_TOO_SMALL,
	UBX_CFG_BUILD_TOO_MANY_ITEMS
} ubx_cfg_build_result_t;

typedef enum
{
	UBX_CFG_DECODE_OK = 0,
	UBX_CFG_DECODE_NULL_ARGUMENT,
	UBX_CFG_DECODE_WRONG_MESSAGE,
	UBX_CFG_DECODE_WRONG_LENGTH,
	UBX_CFG_DECODE_WRONG_VERSION,
	UBX_CFG_DECODE_INVALID_LAYER,
	UBX_CFG_DECODE_KEY_TYPE_MISMATCH
} ubx_cfg_decode_result_t;

typedef struct {
	uint8_t *buffer;
	size_t capacity;
	size_t length;
} ubx_cfg_valset_builder_t;

typedef struct {
	uint8_t *buffer;
	size_t capacity;
	size_t length;
	uint8_t key_count;
} ubx_cfg_valget_builder_t;

typedef struct {
	uint8_t layer;
	uint16_t position;
	uint32_t key;
	uint8_t value;
} ubx_cfg_valget_u1_t;

// Start a UBX-CFG-VALSET payload
ubx_cfg_build_result_t ubx_cfg_valset_begin(ubx_cfg_valset_builder_t *builder, 
											uint8_t *buffer, 
											size_t capacity, 
											uint8_t layers);

// Append one config key-value pair
ubx_cfg_build_result_t ubx_cfg_valset_add_u1(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 uint8_t value);

// Start a UBX-CFG-VALGET
ubx_cfg_build_result_t ubx_cfg_valget_begin(ubx_cfg_valget_builder_t *builder,
											uint8_t *buffer,
											size_t capacity,
											uint8_t layer,
											uint16_t position);

ubx_cfg_build_result_t ubx_cfg_valget_add_key(ubx_cfg_valget_builder_t *builder,
											  uint32_t key);

ubx_cfg_decode_result_t ubx_cfg_valget_decode_u1(const ubx_frame_t *frame,
												 ubx_cfg_valget_u1_t *output);

#endif