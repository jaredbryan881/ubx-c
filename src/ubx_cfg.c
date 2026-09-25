#include "ubx_cfg.h"
#include "ubx_internal.h"

#define UBX_CFG_KEY_STORAGE_SIZE_SHIFT    28U
#define UBX_CFG_KEY_STORAGE_SIZE_U1       0x02U
#define UBX_CFG_U1_ITEM_LENGTH            5U

ubx_cfg_build_result_t ubx_cfg_valset_begin(ubx_cfg_valset_builder_t *builder, uint8_t *buffer, size_t capacity, uint8_t layers){
	if ((builder == NULL) || (buffer == NULL)){
		return UBX_CFG_BUILD_NULL_ARGUMENT;
	}

	if ((layers == 0U) || ((layers & (uint8_t)~UBX_CFG_LAYER_MASK) != 0U)){
		return UBX_CFG_BUILD_INVALID_LAYER;
	}

	if (capacity < UBX_CFG_VALSET_HEADER_LENGTH){
		return UBX_CFG_BUILD_BUFFER_TOO_SMALL;
	}

	// UBX-CFG-VALSET header is 4 bytes: version, destination layers, and two reserved bytes
	buffer[0] = UBX_CFG_VALSET_VERSION_0;
	buffer[1] = layers;
	buffer[2] = 0U;
	buffer[3] = 0U;

	builder->buffer   = buffer;
	builder->capacity = capacity;
	builder->length   = UBX_CFG_VALSET_HEADER_LENGTH;

	return UBX_CFG_BUILD_OK;
}

ubx_cfg_build_result_t ubx_cfg_valset_add_u1(ubx_cfg_valset_builder_t *builder, uint32_t key, uint8_t value){
	uint32_t storage_size;

	if (builder == NULL){
		return UBX_CFG_BUILD_NULL_ARGUMENT;
	}

	if (builder->buffer == NULL){
		return UBX_CFG_BUILD_INVALID_STATE;
	}

	if (builder->length > builder->capacity){
		return UBX_CFG_BUILD_INVALID_STATE;
	}

	storage_size = key >> UBX_CFG_KEY_STORAGE_SIZE_SHIFT;

	if (storage_size != UBX_CFG_KEY_STORAGE_SIZE_U1){
		return UBX_CFG_BUILD_KEY_TYPE_MISMATCH;
	}

	if ((builder->capacity - builder->length) < UBX_CFG_U1_ITEM_LENGTH){
		return UBX_CFG_BUILD_BUFFER_TOO_SMALL;
	}

	ubx_cfg_write_u32_le(&builder->buffer[builder->length], key);

	builder->buffer[builder->length + 4U] = value;
	builder->length += UBX_CFG_U1_ITEM_LENGTH;

	return UBX_CFG_BUILD_OK;
}
