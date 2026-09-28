#ifndef UBX_CFG_H
#define UBX_CFG_H

#include "ubx_parser.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UBX_CFG_CLASS     0x06U

#define UBX_CFG_VALSET_ID            0x8AU
#define UBX_CFG_VALSET_VERSION_0     0x00U
#define UBX_CFG_VALSET_HEADER_LENGTH 4U
#define UBX_CFG_VALUE_MAX_LENGTH     8U

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

typedef enum
{
	UBX_CFG_KEY_STORAGE_L           = 0x01U,
	UBX_CFG_KEY_STORAGE_ONE_BYTE    = 0x02U,
	UBX_CFG_KEY_STORAGE_TWO_BYTES   = 0x03U,
	UBX_CFG_KEY_STORAGE_FOUR_BYTES  = 0x04U,
	UBX_CFG_KEY_STORAGE_EIGHT_BYTES = 0x05U
} ubx_cfg_key_storage_t;

typedef enum {
	UBX_CFG_BUILD_OK = 0,
	UBX_CFG_BUILD_NULL_ARGUMENT,
	UBX_CFG_BUILD_INVALID_LAYER,
	UBX_CFG_BUILD_INVALID_STATE,
	UBX_CFG_BUILD_INVALID_VALUE,
	UBX_CFG_BUILD_KEY_TYPE_MISMATCH,
	UBX_CFG_BUILD_BUFFER_TOO_SMALL,
	UBX_CFG_BUILD_TOO_MANY_ITEMS
} ubx_cfg_build_result_t;

typedef enum {
	UBX_CFG_DECODE_OK = 0,
	UBX_CFG_DECODE_NULL_ARGUMENT,
	UBX_CFG_DECODE_WRONG_MESSAGE,
	UBX_CFG_DECODE_WRONG_LENGTH,
	UBX_CFG_DECODE_WRONG_VERSION,
	UBX_CFG_DECODE_INVALID_LAYER,
	UBX_CFG_DECODE_INVALID_VALUE,
	UBX_CFG_DECODE_KEY_TYPE_MISMATCH
} ubx_cfg_decode_result_t;

typedef enum {
	UBX_CFG_ITERATE_ITEM = 0,
	UBX_CFG_ITERATE_DONE,
	UBX_CFG_ITERATE_NULL_ARGUMENT,
	UBX_CFG_ITERATE_INVALID_STATE,
	UBX_CFG_ITERATE_MALFORMED_ITEM,
	UBX_CFG_ITERATE_UNSUPPORTED_STORAGE_SIZE
} ubx_cfg_iterate_result_t;

typedef struct {
	uint8_t *buffer;
	size_t capacity;
	size_t length;
	uint8_t item_count;
} ubx_cfg_valset_builder_t;

typedef struct {
	uint8_t *buffer;
	size_t capacity;
	size_t length;
	uint8_t key_count;
} ubx_cfg_valget_builder_t;

// Iterator state for one CFG-VALGET response
typedef struct {
	const uint8_t *payload;
	size_t payload_length;
	size_t offset;
	uint8_t layer;
	uint16_t position;
} ubx_cfg_valget_iterator_t;

// One key/value item returned by the iterator
typedef struct {
	uint32_t key;
	ubx_cfg_key_storage_t storage;
	uint8_t value_length;
	uint8_t value[UBX_CFG_VALUE_MAX_LENGTH];
} ubx_cfg_valget_item_t;

// Start a UBX-CFG-VALSET payload
ubx_cfg_build_result_t ubx_cfg_valset_begin(ubx_cfg_valset_builder_t *builder, 
											uint8_t *buffer, 
											size_t capacity, 
											uint8_t layers);

// Append typed config key-value pairs
ubx_cfg_build_result_t ubx_cfg_valset_add_l(ubx_cfg_valset_builder_t *builder, 
											uint32_t key, 
											uint8_t value);

ubx_cfg_build_result_t ubx_cfg_valset_add_u1(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 uint8_t value);

ubx_cfg_build_result_t ubx_cfg_valset_add_u2(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 uint16_t value);

ubx_cfg_build_result_t ubx_cfg_valset_add_u4(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 uint32_t value);

ubx_cfg_build_result_t ubx_cfg_valset_add_i1(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 int8_t value);

ubx_cfg_build_result_t ubx_cfg_valset_add_i2(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 int16_t value);

ubx_cfg_build_result_t ubx_cfg_valset_add_i4(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 int32_t value);

ubx_cfg_build_result_t ubx_cfg_valset_add_e1(ubx_cfg_valset_builder_t *builder, 
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

// Initialize and advance an iterator over the entries in a UBX-CFG-VALGET response
ubx_cfg_decode_result_t ubx_cfg_valget_iterator_init(const ubx_frame_t *frame, 
													 ubx_cfg_valget_iterator_t *iterator);

ubx_cfg_iterate_result_t ubx_cfg_valget_iterator_next(ubx_cfg_valget_iterator_t *iterator,
													  ubx_cfg_valget_item_t *item);

// Interpret an iterator item using the type of its key
ubx_cfg_decode_result_t ubx_cfg_valget_item_get_l(const ubx_cfg_valget_item_t *item,
												  uint8_t *value);

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_u1(const ubx_cfg_valget_item_t *item,
												   uint8_t *value);

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_u2(const ubx_cfg_valget_item_t *item,
												   uint16_t *value);

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_u4(const ubx_cfg_valget_item_t *item,
												   uint32_t *value);

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_i1(const ubx_cfg_valget_item_t *item,
												   int8_t *value);

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_i2(const ubx_cfg_valget_item_t *item,
												   int16_t *value);

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_i4(const ubx_cfg_valget_item_t *item,
												   int32_t *value);

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_e1(const ubx_cfg_valget_item_t *item,
												   uint8_t *value);

#ifdef __cplusplus
}
#endif

#endif