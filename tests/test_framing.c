#include "test_util.h"
#include "ubx_encoder.h"
#include "ubx_parser.h"

// Feed a buffer, return the last non-IN_PROGRESS result and count completed
// frames
static size_t feed_all(ubx_parser_t *parser, const uint8_t *data, size_t length,
                       ubx_frame_t *frame, ubx_parse_result_t *last) {
  size_t frames = 0U;
  size_t index;

  for (index = 0U; index < length; index++) {
    const ubx_parse_result_t result =
        ubx_parser_feed(parser, data[index], frame);

    if (result != UBX_PARSE_IN_PROGRESS) {
      *last = result;
    }
    if (result == UBX_PARSE_FRAME_COMPLETE) {
      frames++;
    }
  }

  return frames;
}

// Published UBX-MON-VER poll from the u-blox interface description
static void test_encode_known_vector(void) {
  static const uint8_t expected[] = {0xB5U, 0x62U, 0x0AU, 0x04U,
                                     0x00U, 0x00U, 0x0EU, 0x34U};
  uint8_t out[16];
  size_t length = 0U;

  CHECK_EQ(ubx_encode_frame(0x0AU, 0x04U, NULL, 0U, out, sizeof(out), &length),
           UBX_ENCODE_OK);
  CHECK_EQ(length, sizeof(expected));
  CHECK_BYTES(out, expected, sizeof(expected));
}

static void test_encode_errors(void) {
  uint8_t out[16];
  static const uint8_t payload[4] = {1U, 2U, 3U, 4U};
  size_t length = 99U;

  CHECK_EQ(ubx_encode_frame(1U, 2U, payload, 4U, out, sizeof(out), NULL),
           UBX_ENCODE_NULL_ARGUMENT);
  CHECK_EQ(ubx_encode_frame(1U, 2U, payload, 4U, NULL, sizeof(out), &length),
           UBX_ENCODE_NULL_ARGUMENT);
  CHECK_EQ(length, 0U);
  CHECK_EQ(ubx_encode_frame(1U, 2U, NULL, 4U, out, sizeof(out), &length),
           UBX_ENCODE_PAYLOAD_REQUIRED);
  CHECK_EQ(ubx_encode_frame(1U, 2U, payload, 4U, out, 11U, &length),
           UBX_ENCODE_BUFFER_TOO_SMALL);
  CHECK_EQ(ubx_encode_frame(1U, 2U, payload, 4U, out, 12U, &length),
           UBX_ENCODE_OK);
  CHECK_EQ(length, 12U);
  CHECK_EQ(ubx_encode_frame(1U, 2U, payload, (size_t)UINT16_MAX + 1U, out,
                            sizeof(out), &length),
           UBX_ENCODE_PAYLOAD_TOO_LARGE);
}

static void test_roundtrip_all_small_lengths(void) {
  uint8_t payload[300];
  uint8_t frame_bytes[320];
  uint8_t parser_buffer[300];
  ubx_parser_t parser;
  ubx_frame_t frame;
  ubx_parse_result_t last = UBX_PARSE_IN_PROGRESS;
  size_t length;
  size_t index;

  for (index = 0U; index < sizeof(payload); index++) {
    payload[index] = (uint8_t)(index * 7U + 3U);
  }

  ubx_parser_init(&parser, parser_buffer, sizeof(parser_buffer));

  for (length = 0U; length <= sizeof(payload); length++) {
    size_t encoded = 0U;
    size_t frames;

    CHECK_EQ(ubx_encode_frame(0x01U, 0x07U, payload, length, frame_bytes,
                              sizeof(frame_bytes), &encoded),
             UBX_ENCODE_OK);
    frames = feed_all(&parser, frame_bytes, encoded, &frame, &last);

    CHECK_EQ(frames, 1U);
    CHECK_EQ(last, UBX_PARSE_FRAME_COMPLETE);
    CHECK_EQ(frame.message_class, 0x01U);
    CHECK_EQ(frame.message_id, 0x07U);
    CHECK_EQ(frame.payload_length, length);
    CHECK_BYTES(frame.payload, payload, length);
  }
}

static void test_garbage_and_resync(void) {
  // Noise, a lone sync char, a doubled first sync char, then a real frame
  static const uint8_t noise[] = {0x00U, 0xFFU, 0xB5U, 0x00U, 0xB5U, 0xB5U};
  uint8_t frame_bytes[16];
  uint8_t buffer[8];
  ubx_parser_t parser;
  ubx_frame_t frame;
  ubx_parse_result_t last = UBX_PARSE_IN_PROGRESS;
  size_t encoded = 0U;

  ubx_parser_init(&parser, buffer, sizeof(buffer));
  CHECK_EQ(ubx_encode_frame(0x05U, 0x01U, NULL, 0U, frame_bytes,
                            sizeof(frame_bytes), &encoded),
           UBX_ENCODE_OK);

  CHECK_EQ(feed_all(&parser, noise, sizeof(noise), &frame, &last), 0U);
  // The trailing 0xB5 in the noise plus the frame's own 0xB5 means the parser
  // sees B5 B5 62 ...
  CHECK_EQ(feed_all(&parser, frame_bytes, encoded, &frame, &last), 1U);
}

static void test_checksum_error_then_recovery(void) {
  uint8_t good[16];
  uint8_t bad[16];
  uint8_t buffer[8];
  ubx_parser_t parser;
  ubx_frame_t frame;
  ubx_parse_result_t last = UBX_PARSE_IN_PROGRESS;
  static const uint8_t payload[2] = {0x06U, 0x8AU};
  size_t encoded = 0U;

  ubx_parser_init(&parser, buffer, sizeof(buffer));
  CHECK_EQ(
      ubx_encode_frame(0x05U, 0x01U, payload, 2U, good, sizeof(good), &encoded),
      UBX_ENCODE_OK);
  (void)memcpy(bad, good, encoded);
  bad[encoded - 1U] ^= 0x01U;

  CHECK_EQ(feed_all(&parser, bad, encoded, &frame, &last), 0U);
  CHECK_EQ(last, UBX_PARSE_CHECKSUM_ERROR);
  CHECK_EQ(feed_all(&parser, good, encoded, &frame, &last), 1U);
  CHECK_EQ(last, UBX_PARSE_FRAME_COMPLETE);
}

static void test_oversize_discards_and_recovers(void) {
  uint8_t big_payload[32] = {0};
  uint8_t big[64];
  uint8_t small[16];
  uint8_t buffer[8];
  ubx_parser_t parser;
  ubx_frame_t frame;
  ubx_parse_result_t last = UBX_PARSE_IN_PROGRESS;
  size_t big_length = 0U;
  size_t small_length = 0U;
  size_t oversize_seen = 0U;
  size_t index;

  ubx_parser_init(&parser, buffer, sizeof(buffer));
  CHECK_EQ(ubx_encode_frame(0x01U, 0x07U, big_payload, sizeof(big_payload), big,
                            sizeof(big), &big_length),
           UBX_ENCODE_OK);
  CHECK_EQ(ubx_encode_frame(0x05U, 0x01U, NULL, 0U, small, sizeof(small),
                            &small_length),
           UBX_ENCODE_OK);

  for (index = 0U; index < big_length; index++) {
    if (ubx_parser_feed(&parser, big[index], &frame) == UBX_PARSE_OVERSIZE) {
      oversize_seen++;
    }
  }
  CHECK_EQ(oversize_seen, 1U);
  CHECK_EQ(feed_all(&parser, small, small_length, &frame, &last), 1U);
}

static void test_null_and_zero_capacity(void) {
  uint8_t frame_bytes[16];
  static const uint8_t payload[1] = {0x55U};
  ubx_parser_t parser;
  ubx_frame_t frame;
  ubx_parse_result_t last = UBX_PARSE_IN_PROGRESS;
  size_t encoded = 0U;

  // NULL parser must not crash
  ubx_parser_init(NULL, NULL, 0U);
  ubx_parser_reset(NULL);
  CHECK_EQ(ubx_parser_feed(NULL, 0xB5U, &frame), UBX_PARSE_IN_PROGRESS);

  // NULL buffer with a claimed capacity is treated as capacity zero
  ubx_parser_init(&parser, NULL, 100U);
  CHECK_EQ(ubx_encode_frame(0x01U, 0x02U, payload, 1U, frame_bytes,
                            sizeof(frame_bytes), &encoded),
           UBX_ENCODE_OK);
  (void)feed_all(&parser, frame_bytes, encoded, &frame, &last);
  CHECK_EQ(last, UBX_PARSE_OVERSIZE);

  // Empty payloads still work with no buffer, and a NULL frame out-pointer is
  // allowed
  CHECK_EQ(ubx_encode_frame(0x01U, 0x02U, NULL, 0U, frame_bytes,
                            sizeof(frame_bytes), &encoded),
           UBX_ENCODE_OK);
  for (size_t index = 0U; index < encoded; index++) {
    last = ubx_parser_feed(&parser, frame_bytes[index], NULL);
  }
  CHECK_EQ(last, UBX_PARSE_FRAME_COMPLETE);
}

int main(void) {
  test_encode_known_vector();
  test_encode_errors();
  test_roundtrip_all_small_lengths();
  test_garbage_and_resync();
  test_checksum_error_then_recovery();
  test_oversize_discards_and_recovers();
  test_null_and_zero_capacity();

  return TEST_RESULT();
}
