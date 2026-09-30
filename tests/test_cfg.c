#include "test_util.h"
#include "ubx_cfg.h"

// Key IDs: the top nibble of the key selects the storage size
#define KEY_L 0x10520005UL
#define KEY_U1 0x20520005UL
#define KEY_U2 0x30210001UL
#define KEY_BAD_STORAGE 0x00000001UL
#define KEY_STORAGE_7 0x70000001UL

static void test_valset_layout(void) {
  static const uint8_t expected[] = {
      0x00U, 0x01U, 0x00U, 0x00U,              // header: version, RAM, reserved
      0x05U, 0x00U, 0x52U, 0x10U, 0x01U,       // L = 1
      0x01U, 0x00U, 0x21U, 0x30U, 0x34U, 0x12U // U2 = 0x1234
  };
  uint8_t buffer[64];
  ubx_cfg_valset_builder_t builder;

  CHECK_EQ(ubx_cfg_valset_begin(&builder, buffer, sizeof(buffer),
                                UBX_CFG_VALSET_LAYER_RAM),
           UBX_CFG_BUILD_OK);
  CHECK_EQ(ubx_cfg_valset_add_l(&builder, KEY_L, 1U), UBX_CFG_BUILD_OK);
  CHECK_EQ(ubx_cfg_valset_add_u2(&builder, KEY_U2, 0x1234U), UBX_CFG_BUILD_OK);
  CHECK_EQ(builder.length, sizeof(expected));
  CHECK_EQ(builder.item_count, 2U);
  CHECK_BYTES(buffer, expected, sizeof(expected));
}

static void test_valset_rejects_bad_input(void) {
  uint8_t buffer[16];
  ubx_cfg_valset_builder_t builder;

  CHECK_EQ(ubx_cfg_valset_begin(NULL, buffer, sizeof(buffer), 1U),
           UBX_CFG_BUILD_NULL_ARGUMENT);
  CHECK_EQ(ubx_cfg_valset_begin(&builder, NULL, sizeof(buffer), 1U),
           UBX_CFG_BUILD_NULL_ARGUMENT);
  CHECK_EQ(ubx_cfg_valset_begin(&builder, buffer, sizeof(buffer), 0U),
           UBX_CFG_BUILD_INVALID_LAYER);
  CHECK_EQ(ubx_cfg_valset_begin(&builder, buffer, sizeof(buffer), 0x08U),
           UBX_CFG_BUILD_INVALID_LAYER);
  CHECK_EQ(ubx_cfg_valset_begin(&builder, buffer, 3U, 1U),
           UBX_CFG_BUILD_BUFFER_TOO_SMALL);

  CHECK_EQ(ubx_cfg_valset_begin(&builder, buffer, sizeof(buffer), 1U),
           UBX_CFG_BUILD_OK);
  CHECK_EQ(ubx_cfg_valset_add_l(&builder, KEY_L, 2U),
           UBX_CFG_BUILD_INVALID_VALUE);
  // Wrong storage class for the key
  CHECK_EQ(ubx_cfg_valset_add_u1(&builder, (uint32_t)KEY_U2, 1U),
           UBX_CFG_BUILD_KEY_TYPE_MISMATCH);
  CHECK_EQ(ubx_cfg_valset_add_u4(&builder, (uint32_t)KEY_U2, 1U),
           UBX_CFG_BUILD_KEY_TYPE_MISMATCH);
  CHECK_EQ(ubx_cfg_valset_add_u1(&builder, (uint32_t)KEY_BAD_STORAGE, 1U),
           UBX_CFG_BUILD_KEY_TYPE_MISMATCH);
  CHECK_EQ(ubx_cfg_valset_add_u1(&builder, (uint32_t)KEY_STORAGE_7, 1U),
           UBX_CFG_BUILD_KEY_TYPE_MISMATCH);
  // Nothing above may have been written
  CHECK_EQ(builder.length, UBX_CFG_VALSET_HEADER_LENGTH);
  CHECK_EQ(builder.item_count, 0U);

  // 16 byte buffer: 4 header + 5 + 5 = 14, a third 5-byte item does not fit
  CHECK_EQ(ubx_cfg_valset_add_u1(&builder, (uint32_t)KEY_U1, 1U),
           UBX_CFG_BUILD_OK);
  CHECK_EQ(ubx_cfg_valset_add_u1(&builder, (uint32_t)KEY_U1, 2U),
           UBX_CFG_BUILD_OK);
  CHECK_EQ(ubx_cfg_valset_add_u1(&builder, (uint32_t)KEY_U1, 3U),
           UBX_CFG_BUILD_BUFFER_TOO_SMALL);
  CHECK_EQ(builder.length, 14U);
}

static void test_valset_item_limit(void) {
  uint8_t buffer[1024];
  ubx_cfg_valset_builder_t builder;
  unsigned count;

  CHECK_EQ(ubx_cfg_valset_begin(&builder, buffer, sizeof(buffer), 1U),
           UBX_CFG_BUILD_OK);
  for (count = 0U; count < 64U; count++) {
    CHECK_EQ(ubx_cfg_valset_add_u1(&builder, (uint32_t)KEY_U1, 0U),
             UBX_CFG_BUILD_OK);
  }
  CHECK_EQ(ubx_cfg_valset_add_u1(&builder, (uint32_t)KEY_U1, 0U),
           UBX_CFG_BUILD_TOO_MANY_ITEMS);
}

static void test_valget_request(void) {
  uint8_t buffer[12];
  ubx_cfg_valget_builder_t builder;

  CHECK_EQ(ubx_cfg_valget_begin(&builder, buffer, sizeof(buffer), 3U, 0U),
           UBX_CFG_BUILD_INVALID_LAYER);
  CHECK_EQ(ubx_cfg_valget_begin(&builder, buffer, sizeof(buffer),
                                UBX_CFG_VALGET_LAYER_BBR, 0x0102U),
           UBX_CFG_BUILD_OK);
  CHECK_EQ(buffer[1], UBX_CFG_VALGET_LAYER_BBR);
  CHECK_EQ(buffer[2], 0x02U);
  CHECK_EQ(buffer[3], 0x01U);
  CHECK_EQ(ubx_cfg_valget_add_key(&builder, (uint32_t)KEY_U2),
           UBX_CFG_BUILD_OK);
  CHECK_EQ(builder.length, 8U);
  CHECK_EQ(ubx_cfg_valget_add_key(&builder, (uint32_t)KEY_U2),
           UBX_CFG_BUILD_OK);
  CHECK_EQ(builder.length, 12U);
  // 12 byte buffer is now full: header (4) + 2 keys (4 each)
  CHECK_EQ(ubx_cfg_valget_add_key(&builder, (uint32_t)KEY_U2),
           UBX_CFG_BUILD_BUFFER_TOO_SMALL);
}

static ubx_frame_t make_valget_frame(const uint8_t *payload, uint16_t length) {
  ubx_frame_t frame;

  frame.message_class = UBX_CFG_CLASS;
  frame.message_id = UBX_CFG_VALGET_ID;
  frame.payload_length = length;
  frame.payload = payload;

  return frame;
}

static void test_valget_iterator(void) {
  static const uint8_t payload[] = {
      0x01U, 0x00U, 0x00U, 0x00U,               // response, RAM, position 0
      0x01U, 0x00U, 0x21U, 0x30U, 0x34U, 0x12U, // U2 = 0x1234
      0x03U, 0x00U, 0x91U, 0x40U, 0xFEU, 0xFFU, 0xFFU, 0xFFU, // I4 = -2
      0x01U, 0x00U, 0x91U, 0x20U, 0x80U                       // I1 = -128
  };
  ubx_frame_t frame = make_valget_frame(payload, (uint16_t)sizeof(payload));
  ubx_cfg_valget_iterator_t iterator;
  ubx_cfg_valget_item_t item;
  uint16_t u2 = 0U;
  int32_t i4 = 0;
  int8_t i1 = 0;

  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator), UBX_CFG_DECODE_OK);

  CHECK_EQ(ubx_cfg_valget_iterator_next(&iterator, &item),
           UBX_CFG_ITERATE_ITEM);
  CHECK_EQ(ubx_cfg_valget_item_get_u2(&item, &u2), UBX_CFG_DECODE_OK);
  CHECK_EQ(u2, 0x1234U);
  // Reading with the wrong type must fail rather than reinterpret bytes
  CHECK_EQ(ubx_cfg_valget_item_get_u4(&item, (uint32_t[1]){0}),
           UBX_CFG_DECODE_KEY_TYPE_MISMATCH);

  CHECK_EQ(ubx_cfg_valget_iterator_next(&iterator, &item),
           UBX_CFG_ITERATE_ITEM);
  CHECK_EQ(ubx_cfg_valget_item_get_i4(&item, &i4), UBX_CFG_DECODE_OK);
  CHECK_EQ(i4, -2);

  CHECK_EQ(ubx_cfg_valget_iterator_next(&iterator, &item),
           UBX_CFG_ITERATE_ITEM);
  CHECK_EQ(ubx_cfg_valget_item_get_i1(&item, &i1), UBX_CFG_DECODE_OK);
  CHECK_EQ(i1, -128);

  CHECK_EQ(ubx_cfg_valget_iterator_next(&iterator, &item),
           UBX_CFG_ITERATE_DONE);
}

static void test_i1_conversion_edges(void) {
  ubx_cfg_valget_item_t item = {0};
  int8_t value = 0;
  unsigned raw;

  item.storage = UBX_CFG_KEY_STORAGE_ONE_BYTE;
  item.value_length = 1U;

  for (raw = 0U; raw <= 255U; raw++) {
    item.value[0] = (uint8_t)raw;
    CHECK_EQ(ubx_cfg_valget_item_get_i1(&item, &value), UBX_CFG_DECODE_OK);
    CHECK_EQ(value, (int8_t)(raw < 128U ? (int)raw : (int)raw - 256));
  }
}

static void test_valget_iterator_malformed(void) {
  static const uint8_t truncated_key[] = {0x01U, 0x00U, 0x00U,
                                          0x00U, 0x01U, 0x00U};
  static const uint8_t truncated_value[] = {0x01U, 0x00U, 0x00U, 0x00U, 0x01U,
                                            0x00U, 0x21U, 0x30U, 0x34U};
  static const uint8_t bad_storage[] = {0x01U, 0x00U, 0x00U, 0x00U, 0x01U,
                                        0x00U, 0x00U, 0x00U, 0x00U};
  static const uint8_t bad_version[] = {0x00U, 0x00U, 0x00U, 0x00U};
  static const uint8_t bad_layer[] = {0x01U, 0x05U, 0x00U, 0x00U};
  static const uint8_t short_header[] = {0x01U, 0x00U, 0x00U};
  ubx_cfg_valget_iterator_t iterator;
  ubx_cfg_valget_item_t item;
  ubx_frame_t frame;

  frame = make_valget_frame(truncated_key, (uint16_t)sizeof(truncated_key));
  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator), UBX_CFG_DECODE_OK);
  CHECK_EQ(ubx_cfg_valget_iterator_next(&iterator, &item),
           UBX_CFG_ITERATE_MALFORMED_ITEM);

  frame = make_valget_frame(truncated_value, (uint16_t)sizeof(truncated_value));
  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator), UBX_CFG_DECODE_OK);
  CHECK_EQ(ubx_cfg_valget_iterator_next(&iterator, &item),
           UBX_CFG_ITERATE_MALFORMED_ITEM);

  frame = make_valget_frame(bad_storage, (uint16_t)sizeof(bad_storage));
  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator), UBX_CFG_DECODE_OK);
  CHECK_EQ(ubx_cfg_valget_iterator_next(&iterator, &item),
           UBX_CFG_ITERATE_UNSUPPORTED_STORAGE_SIZE);

  frame = make_valget_frame(bad_version, (uint16_t)sizeof(bad_version));
  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator),
           UBX_CFG_DECODE_WRONG_VERSION);

  frame = make_valget_frame(bad_layer, (uint16_t)sizeof(bad_layer));
  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator),
           UBX_CFG_DECODE_INVALID_LAYER);

  frame = make_valget_frame(short_header, (uint16_t)sizeof(short_header));
  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator),
           UBX_CFG_DECODE_WRONG_LENGTH);

  frame = make_valget_frame(NULL, 4U);
  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator),
           UBX_CFG_DECODE_NULL_ARGUMENT);
  CHECK_EQ(ubx_cfg_valget_iterator_init(NULL, &iterator),
           UBX_CFG_DECODE_NULL_ARGUMENT);

  frame.message_id = 0x00U;
  frame.payload = bad_version;
  CHECK_EQ(ubx_cfg_valget_iterator_init(&frame, &iterator),
           UBX_CFG_DECODE_WRONG_MESSAGE);
}

int main(void) {
  test_valset_layout();
  test_valset_rejects_bad_input();
  test_valset_item_limit();
  test_valget_request();
  test_valget_iterator();
  test_i1_conversion_edges();
  test_valget_iterator_malformed();

  return TEST_RESULT();
}
