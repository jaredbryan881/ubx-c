#include "ubx_cfg.h"
#include "ubx_internal.h"

#define UBX_CFG_KEY_STORAGE_SHIFT 28U
#define UBX_CFG_KEY_STORAGE_MASK  0x07U
#define UBX_CFG_KEY_FIELD_LENGTH  4U
#define UBX_CFG_VALSET_MAX_ITEMS  64U

static uint8_t ubx_cfg_valget_layer_is_valid(uint8_t layer){
	return (uint8_t)((layer == UBX_CFG_VALGET_LAYER_RAM) ||
					 (layer == UBX_CFG_VALGET_LAYER_BBR) ||
					 (layer == UBX_CFG_VALGET_LAYER_FLASH) ||
					 (layer == UBX_CFG_VALGET_LAYER_DEFAULT));
}

static ubx_cfg_key_storage_t ubx_cfg_key_storage(uint32_t key){
	return (ubx_cfg_key_storage_t)((key >> UBX_CFG_KEY_STORAGE_SHIFT) & (UBX_CFG_KEY_STORAGE_MASK));
}

static uint8_t ubx_cfg_storage_value_length(ubx_cfg_key_storage_t storage){
	switch (storage){
	case UBX_CFG_KEY_STORAGE_L:
		return 1U;

	case UBX_CFG_KEY_STORAGE_ONE_BYTE:
		return 1U;

	case UBX_CFG_KEY_STORAGE_TWO_BYTES:
		return 2U;

	case UBX_CFG_KEY_STORAGE_FOUR_BYTES:
		return 4U;

	case UBX_CFG_KEY_STORAGE_EIGHT_BYTES:
		return 8U;

	default:
		return 0U;
	}
}

static ubx_cfg_build_result_t ubx_cfg_valset_add_raw(ubx_cfg_valset_builder_t *builder,
													 uint32_t key,
													 ubx_cfg_key_storage_t expected_storage,
													 const uint8_t *value,
													 uint8_t value_length){
	ubx_cfg_key_storage_t actual_storage;
	size_t item_length;
	uint8_t index;

	if ((builder == NULL) || (value == NULL)){
		return UBX_CFG_BUILD_NULL_ARGUMENT;
	}

	if (builder->buffer == NULL){
		return UBX_CFG_BUILD_INVALID_STATE;
	}

	if (builder->length > builder->capacity){
		return UBX_CFG_BUILD_INVALID_STATE;
	}

	if (builder->item_count >= UBX_CFG_VALSET_MAX_ITEMS){
		return UBX_CFG_BUILD_TOO_MANY_ITEMS;
	}

	actual_storage = ubx_cfg_key_storage(key);

	if (actual_storage != expected_storage){
		return UBX_CFG_BUILD_KEY_TYPE_MISMATCH;
	}

	if (ubx_cfg_storage_value_length(actual_storage) != value_length){
		return UBX_CFG_BUILD_KEY_TYPE_MISMATCH;
	}

	item_length = UBX_CFG_KEY_FIELD_LENGTH + (size_t)value_length;

	if ((builder->capacity - builder->length) < item_length){
		return UBX_CFG_BUILD_BUFFER_TOO_SMALL;
	}

	ubx_write_u32_le(&builder->buffer[builder->length], key);

	for (index = 0U; index < value_length; index++){
		builder->buffer[builder->length + UBX_CFG_KEY_FIELD_LENGTH + index] = value[index];
	}

	builder->length += item_length;
	builder->item_count++;


	return UBX_CFG_BUILD_OK;
}

static ubx_cfg_decode_result_t ubx_cfg_valget_item_check(const ubx_cfg_valget_item_t *item,
														 ubx_cfg_key_storage_t expected_storage,
														 uint8_t expected_length){
	if (item == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	if ((item->storage != expected_storage) || (item->value_length != expected_length)){
		return UBX_CFG_DECODE_KEY_TYPE_MISMATCH;
	}

	return UBX_CFG_DECODE_OK;
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
	builder->iterm_count = 0U;

	return UBX_CFG_BUILD_OK;
}

ubx_cfg_build_result_t ubx_cfg_valset_add_l(ubx_cfg_valset_builder_t *builder,
											uint32_t key,
											uint8_t value){
	if (value > 1U){
		return UBX_CFG_BUILD_INVALID_VALUE;
	}

	return ubx_cfg_valset_add_raw(builder,
								  key,
								  UBX_CFG_KEY_STORAGE_L,
								  &value,
								  1U);
}

ubx_cfg_build_result_t ubx_cfg_valset_add_u1(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 uint8_t value){
	return ubx_cfg_valset_add_raw(builder,
								  key,
								  UBX_CFG_KEY_STORAGE_ONE_BYTE,
								  &value,
								  1U);
}

ubx_cfg_build_result_t ubx_cfg_valset_add_u2(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 uint16_t value){
	uint8_t encoded_value[2];
	ubx_write_u16_le(encoded_value, value);

	return ubx_cfg_valset_add_raw(builder,
								  key,
								  UBX_CFG_KEY_STORAGE_TWO_BYTES,
								  encoded_value,
								  2U);
}

ubx_cfg_build_result_t ubx_cfg_valset_add_u4(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 uint32_t value){
	uint8_t encoded_value[4];
	ubx_write_u32_le(encoded_value, value);

	return ubx_cfg_valset_add_raw(builder,
								  key,
								  UBX_CFG_KEY_STORAGE_FOUR_BYTES,
								  encoded_value,
								  4U);
}

ubx_cfg_build_result_t ubx_cfg_valset_add_i1(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 int8_t value){
	uint8_t encoded_value = (uint8_t)value;

	return ubx_cfg_valset_add_raw(builder,
								  key,
								  UBX_CFG_KEY_STORAGE_ONE_BYTE,
								  &encoded_value,
								  1U);
}

ubx_cfg_build_result_t ubx_cfg_valset_add_i2(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 int16_t value){
	uint8_t encoded_value[2];
	ubx_write_u16_le(encoded_value, (uint16_t)value);

	return ubx_cfg_valset_add_raw(builder,
								  key,
								  UBX_CFG_KEY_STORAGE_TWO_BYTES,
								  encoded_value,
								  2U);
}

ubx_cfg_build_result_t ubx_cfg_valset_add_i4(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 int32_t value){
	uint8_t encoded_value[4];
	ubx_write_u32_le(encoded_value, (uint32_t)value);

	return ubx_cfg_valset_add_raw(builder,
								  key,
								  UBX_CFG_KEY_STORAGE_FOUR_BYTES,
								  encoded_value,
								  4U);
}

ubx_cfg_build_result_t ubx_cfg_valset_add_e1(ubx_cfg_valset_builder_t *builder, 
											 uint32_t key, 
											 uint8_t value){

	return ubx_cfg_valset_add_raw(builder,
								  key,
								  UBX_CFG_KEY_STORAGE_ONE_BYTE,
								  &value,
								  1U);
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

ubx_cfg_build_result_t ubx_cfg_valget_add_key(ubx_cfg_valget_builder_t *builder, 
											  uint32_t key){
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

ubx_cfg_decode_result_t ubx_cfg_valget_iterator_init(const ubx_frame_t *frame,
													 ubx_cfg_valget_iterator_t *iterator){
	const uint8_t *payload;

	if ((frame == NULL) || (iterator == NULL)){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	if ((frame->message_class != UBX_CFG_CLASS) || (frame->message_id != UBX_CFG_VALGET_ID)){
		return UBX_CFG_DECODE_WRONG_MESSAGE;
	}

	if (frame->payload == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	if (frame->payload_length < UBX_CFG_VALGET_HEADER_LENGTH){
		return UBX_CFG_DECODE_WRONG_LENGTH;
	}

	payload = frame->payload;

	if (payload[0] != UBX_CFG_VALGET_RESPONSE_VERSION){
		return UBX_CFG_DECODE_WRONG_VERSION;
	}

	if (ubx_cfg_valget_layer_is_valid(payload[1]) == 0U){
		return UBX_CFG_DECODE_INVALID_LAYER;
	}

	iterator->payload = payload;
	iterator->payload_length = frame->payload_length;
	iterator->offset = UBX_CFG_VALGET_HEADER_LENGTH;
	iterator->layer = payload[1];
	iterator->position = ubx_read_u16_le(&payload[2]);

	return UBX_CFG_DECODE_OK;
}

ubx_cfg_iterate_result_t ubx_cfg_valget_iterator_next(ubx_cfg_valget_iterator_t *iterator,
													  ubx_cfg_valget_item_t *item){
	uint32_t key;
	ubx_cfg_key_storage_t storage;
	uint8_t value_length;
	size_t item_length;
	uint8_t index;

	if ((iterator == NULL) || (item == NULL)){
		return UBX_CFG_ITERATE_NULL_ARGUMENT;
	}

	if ((iterator->payload == NULL) || (iterator->offset > iterator->payload_length)){
		return UBX_CFG_ITERATE_INVALID_STATE;
	}

	if (iterator->offset == iterator->payload_length){
		return UBX_CFG_ITERATE_DONE;
	}

	if ((iterator->payload_length - iterator->offset) < UBX_CFG_KEY_FIELD_LENGTH){
		return UBX_CFG_ITERATE_MALFORMED_ITEM;
	}

	key = ubx_read_u32_le(&iterator->payload[iterator->offset]);

	storage = ubx_cfg_key_storage(key);
	value_length = ubx_cfg_storage_value_length(storage);

	if (value_length == 0U){
		return UBX_CFG_ITERATE_UNSUPPORTED_STORAGE_SIZE;
	}

	item_length = UBX_CFG_KEY_FIELD_LENGTH + (size_t)value_length;

	if ((iterator->payload_length - iterator->offset) < item_length){
		return UBX_CFG_ITERATE_MALFORMED_ITEM;
	}

	item->key = key;
	item->storage = storage;
	item->value_length = value_length;

	for (index = 0U; index < UBX_CFG_VALUE_MAX_LENGTH; index++){
		item->value[index] = 0U;
	}

	for (index = 0U; index < value_length; index++){
		item->value[index] = iterator->payload[iterator->offset + UBX_CFG_KEY_FIELD_LENGTH + index];
	}

	iterator->offset += item_length;

	return UBX_CFG_ITERATE_ITEM;
}

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_l(const ubx_cfg_valget_item_t *item,
												  uint8_t *value){
	ubx_cfg_decode_result_t result;

	if (value == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	result = ubx_cfg_valget_item_check(item, UBX_CFG_KEY_STORAGE_L, 1U);

	if (result != UBX_CFG_DECODE_OK){
		return result;
	}

	if (item->value[0] > 1U){
		return UBX_CFG_DECODE_INVALID_VALUE;
	}

	*value = item->value[0];

	return UBX_CFG_DECODE_OK;
}

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_u1(const ubx_cfg_valget_item_t *item,
												   uint8_t *value){
	ubx_cfg_decode_result_t result;

	if (value == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	result = ubx_cfg_valget_item_check(item, UBX_CFG_KEY_STORAGE_ONE_BYTE, 1U);

	if (result != UBX_CFG_DECODE_OK){
		return result;
	}

	*value = item->value[0];

	return UBX_CFG_DECODE_OK;
}

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_u2(const ubx_cfg_valget_item_t *item,
												   uint16_t *value){
	ubx_cfg_decode_result_t result;

	if (value == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	result = ubx_cfg_valget_item_check(item, UBX_CFG_KEY_STORAGE_TWO_BYTES, 2U);

	if (result != UBX_CFG_DECODE_OK){
		return result;
	}

	*value = ubx_read_u16_le(item->value);

	return UBX_CFG_DECODE_OK;
}

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_u4(const ubx_cfg_valget_item_t *item,
												   uint32_t *value){
	ubx_cfg_decode_result_t result;

	if (value == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	result = ubx_cfg_valget_item_check(item, UBX_CFG_KEY_STORAGE_FOUR_BYTES, 4U);

	if (result != UBX_CFG_DECODE_OK){
		return result;
	}

	*value = ubx_read_u32_le(item->value);

	return UBX_CFG_DECODE_OK;
}

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_i1(const ubx_cfg_valget_item_t *item,
												   int8_t *value){
	ubx_cfg_decode_result_t result;
	uint8_t raw_value;

	if (value == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	result = ubx_cfg_valget_item_check(item, UBX_CFG_KEY_STORAGE_ONE_BYTE, 1U);

	if (result != UBX_CFG_DECODE_OK){
		return result;
	}

	raw_value = item->value[0];

	if (raw_value <= (uint8_t)INT8_MAX){
		*value = (int8_t)raw_value;
	}
	else{
		*value =(int8_t)(-1 - (int16_t)(UINT8_MAX - raw_value));
	}

	return UBX_CFG_DECODE_OK;
}

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_i2(const ubx_cfg_valget_item_t *item,
												   int16_t *value){
	ubx_cfg_decode_result_t result;

	if (value == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	result = ubx_cfg_valget_item_check(item, UBX_CFG_KEY_STORAGE_TWO_BYTES, 2U);

	if (result != UBX_CFG_DECODE_OK){
		return result;
	}

	*value = ubx_read_i16_le(item->value);

	return UBX_CFG_DECODE_OK;
}

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_i4(const ubx_cfg_valget_item_t *item,
												   int32_t *value){
	ubx_cfg_decode_result_t result;

	if (value == NULL){
		return UBX_CFG_DECODE_NULL_ARGUMENT;
	}

	result = ubx_cfg_valget_item_check(item, UBX_CFG_KEY_STORAGE_FOUR_BYTES, 4U);

	if (result != UBX_CFG_DECODE_OK){
		return result;
	}

	*value = ubx_read_i32_le(item->value);

	return UBX_CFG_DECODE_OK;
}

ubx_cfg_decode_result_t ubx_cfg_valget_item_get_e1(const ubx_cfg_valget_item_t *item,
												   uint8_t *value){
	return ubx_cfg_valget_item_get_u1(item, value);
}
