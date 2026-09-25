#include "ubx_cfg.h"
#include "ubx_internal.h"

#define UBX_CFG_KEY_STORAGE_SIZE_SHIFT 28U
#define UBX_CFG_KEY_STORAGE_SIZE_U1    0x02U
#define UBX_CFG_U1_ITEM_LENGTH         5U

static uint8_t ubx_cfg_valget_layer_is_valid(uint8_t layer){
	return (uint8_t)((layer == UBX_CFG_VALGET_LAYER_RAM) ||
					 (layer == UBX_CFG_VALGET_LAYER_BBR) ||
					 (layer == UBX_CFG_VALGET_LAYER_FLASH) ||
					 (layer == UBX_CFG_VALGET_LAYER_DEFAULT));
}

ubx_cfg_build_result_t ubx_cfg_valset_begin(ubx_cfg_valset_builder_t *builder, uint8_t *buffer, size_t capacity, uint8_t layers){
	if ((builder == NULL) || (buffer == NULL)){
		return UBX_CFG_BUILD_NULL_ARGUMENT;
	}

	if ((layers == 0U) || ((layers & (uint8_t)~UBX_CFG_VALSET_LAYER_MASK) != 0U)){
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

	ubx_write_u32_le(&builder->buffer[builder->length], key);

	builder->buffer[builder->length + 4U] = value;
	builder->length += UBX_CFG_U1_ITEM_LENGTH;

	return UBX_CFG_BUILD_OK;
}

ubx_cfg_build_result_t ubx_cfg_valget_begin(ubx_cfg_valget_builder_t *builder,
											uint8_t *buffer,
											size_t capacity,
											uint8_t layer,
											uint16_t position){
	if ((builder == NULL) || (buffer == NULL)){
		return UBX_CFG_BUILD_NULL_ARGUMENT;
	}

	if (ubx_cfg_valget_layer_is_valid(layer) == 0U){
		return UBX_CFG_BUILD_INVALID_LAYER;
	}

	if (capacity < UBX_CFG_VALGET_HEADER_LENGTH){
		return UBX_CFG_BUILD_BUFFER_TOO_SMALL;
	}

	// UBX-CFG-VALGET request header is 4 bytes: version, selector, two position bytes
	buffer[0] = UBX_CFG_VALGET_REQUEST_VERSION;
	buffer[1] = layer;

	ubx_write_u16_le(&buffer[2], position);

	builder->buffer = buffer;
	builder->capacity = capacity;
	builder->length = UBX_CFG_VALGET_HEADER_LENGTH;
	builder->key_count = 0U;

	return UBX_CFG_BUILD_OK;
}

ubx_cfg_build_result_t ubx_cfg_valget_add_key(ubx_cfg_valget_builder_t *builder, uint32_t key){
	if (builder == NULL){
		return UBX_CFG_BUILD_NULL_ARGUMENT;
	}

	if (builder->buffer == NULL){
		return UBX_CFG_BUILD_INVALID_STATE;
	}

	if (builder->length > builder->capacity){
		return UBX_CFG_BUILD_INVALID_STATE;
	}

	if (builder->key_count >= UBX_CFG_VALGET_MAX_KEYS){
		return UBX_CFG_BUILD_TOO_MANY_ITEMS;
	}

	if ((builder->capacity - builder->length) < UBX_CFG_VALGET_KEY_LENGTH){
		return UBX_CFG_BUILD_BUFFER_TOO_SMALL;
	}

	ubx_write_u32_le(&builder->buffer[builder->length], key);

	builder->length += UBX_CFG_VALGET_KEY_LENGTH;
	builder->key_count++;

	return UBX_CFG_BUILD_OK;
}

ubx_cfg_decode_result_t ubx_cfg_valget_decode_u1(const ubx_frame_t *frame, ubx_cfg_valget_u1_t *output){
	const uint8_t *payload;
	uint32_t key;
	uint32_t storage_size;

	if ((frame == NULL) || (output == NULL)){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	if ((frame->message_class != UBX_CFG_CLASS) || (frame->message_id != UBX_CFG_VALGET_ID)){
		return UBX_CFG_DECODE_WRONG_MESSAGE;
	}

	if (frame->payload == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	if (frame->payload_length != 9U){
		return UBX_CFG_DECODE_WRONG_LENGTH;
	}

	payload = frame->payload;

	if (payload[0] != UBX_CFG_VALGET_RESPONSE_VERSION){
		return UBX_CFG_DECODE_WRONG_VERSION;
	}

	if (ubx_cfg_valget_layer_is_valid(payload[1]) == 0U){
		return UBX_CFG_DECODE_INVALID_LAYER;
	}

	key = ubx_read_u32_le(&payload[4]);
	storage_size = key >> UBX_CFG_KEY_STORAGE_SIZE_SHIFT;

	if (storage_size != UBX_CFG_KEY_STORAGE_SIZE_U1){
		return UBX_CFG_DECODE_KEY_TYPE_MISMATCH;
	}

	output->layer = payload[1];
	output->position = ubx_read_u16_le(&payload[2]);
	output->key = key;
	output->value = payload[8];

	return UBX_CFG_DECODE_OK;
}