#include "test_util.h"
#include "ubx_ack.h"
#include "ubx_nav.h"
#include "ubx_tim.h"

static ubx_frame_t make_frame(uint8_t message_class, uint8_t message_id, const uint8_t *payload, uint16_t length){
	ubx_frame_t frame;

	frame.message_class = message_class;
	frame.message_id = message_id;
	frame.payload_length = length;
	frame.payload = payload;

	return frame;
}

static void test_ack(void){
	static const uint8_t payload[2] = {0x06U, 0x8AU};
	ubx_ack_t ack;
	ubx_frame_t frame = make_frame(UBX_ACK_CLASS, UBX_ACK_ACK_ID, payload, 2U);

	CHECK_EQ(ubx_ack_decode(&frame, &ack), UBX_ACK_DECODE_OK);
	CHECK_EQ(ack.type, UBX_ACK_TYPE_ACK);
	CHECK_EQ(ack.acknowledged_class, 0x06U);
	CHECK_EQ(ack.acknowledged_id, 0x8AU);

	frame.message_id = UBX_ACK_NAK_ID;
	CHECK_EQ(ubx_ack_decode(&frame, &ack), UBX_ACK_DECODE_OK);
	CHECK_EQ(ack.type, UBX_ACK_TYPE_NAK);

	frame.message_id = 0x02U;
	CHECK_EQ(ubx_ack_decode(&frame, &ack), UBX_ACK_DECODE_WRONG_ID);
	frame.message_id = UBX_ACK_ACK_ID;
	frame.message_class = 0x01U;
	CHECK_EQ(ubx_ack_decode(&frame, &ack), UBX_ACK_DECODE_WRONG_CLASS);
	frame.message_class = UBX_ACK_CLASS;
	frame.payload_length = 3U;
	CHECK_EQ(ubx_ack_decode(&frame, &ack), UBX_ACK_DECODE_WRONG_LENGTH);
	CHECK_EQ(ubx_ack_decode(NULL, &ack), UBX_ACK_DECODE_NULL_ARGUMENT);
	CHECK_EQ(ubx_ack_decode(&frame, NULL), UBX_ACK_DECODE_NULL_ARGUMENT);
}

// Regression: a right-length frame with a NULL payload used to dereference NULL
static void test_ack_null_payload(void){
	ubx_ack_t ack;
	ubx_frame_t frame = make_frame(UBX_ACK_CLASS, UBX_ACK_ACK_ID, NULL, 2U);

	CHECK_EQ(ubx_ack_decode(&frame, &ack), UBX_ACK_DECODE_NULL_ARGUMENT);
}

static void test_tim_tp(void){
	static const uint8_t payload[16] = {
		0x78U, 0x56U, 0x34U, 0x12U,  // tow
		0x01U, 0x00U, 0x00U, 0x80U,  // tow_sub
		0xFEU, 0xFFU, 0xFFU, 0xFFU,  // quantization error = -2
		0x34U, 0x12U,                // week
		0x03U, 0x0AU                 // flags, reference info
	};
	ubx_tim_tp_t tp;
	ubx_frame_t frame = make_frame(UBX_TIM_CLASS, UBX_TIM_TP_ID, payload, 16U);

	CHECK_EQ(ubx_tim_tp_decode(&frame, &tp), UBX_TIM_TP_DECODE_OK);
	CHECK_EQ(tp.tow, 0x12345678UL);
	CHECK_EQ(tp.tow_sub, 0x80000001UL);
	CHECK_EQ(tp.quantization_error, -2);
	CHECK_EQ(tp.week, 0x1234U);
	CHECK_EQ(tp.flags, 0x03U);
	CHECK_EQ(tp.reference_info, 0x0AU);

	frame.payload_length = 15U;
	CHECK_EQ(ubx_tim_tp_decode(&frame, &tp), UBX_TIM_TP_DECODE_WRONG_LENGTH);
	frame.payload_length = 16U;
	frame.message_id = 0x02U;
	CHECK_EQ(ubx_tim_tp_decode(&frame, &tp), UBX_TIM_TP_DECODE_WRONG_MESSAGE);
	frame.message_id = UBX_TIM_TP_ID;
	frame.payload = NULL;
	CHECK_EQ(ubx_tim_tp_decode(&frame, &tp), UBX_TIM_TP_DECODE_NULL_ARGUMENT);
	CHECK_EQ(ubx_tim_tp_decode(NULL, &tp), UBX_TIM_TP_DECODE_NULL_ARGUMENT);
}

static void put_le(uint8_t *payload, size_t offset, uint32_t value, size_t bytes){
	size_t index;

	for (index = 0U; index < bytes; index++){
		payload[offset + index] = (uint8_t)(value >> (8U * index));
	}
}

static void test_nav_pvt(void){
	uint8_t payload[UBX_NAV_PVT_PAYLOAD_LENGTH] = {0};
	ubx_nav_pvt_t pvt;
	ubx_frame_t frame = make_frame(UBX_NAV_PVT_CLASS, UBX_NAV_PVT_ID, payload, UBX_NAV_PVT_PAYLOAD_LENGTH);

	put_le(payload, 0U, 123456U, 4U);
	put_le(payload, 4U, 2026U, 2U);
	payload[6] = 9U;
	payload[7] = 28U;
	payload[11] = 0x27U;   // valid bits + UTC standard 2 in the high nibble
	payload[20] = 3U;
	payload[23] = 17U;
	put_le(payload, 24U, (uint32_t)(int32_t)-1234567, 4U);   // lon
	put_le(payload, 28U, 515000000U, 4U);                     // lat
	put_le(payload, 56U, (uint32_t)(int32_t)-500, 4U);        // v down
	put_le(payload, 76U, 123U, 2U);
	put_le(payload, 84U, (uint32_t)(int32_t)-9000000, 4U);
	put_le(payload, 88U, (uint32_t)(uint16_t)(int16_t)-42, 2U);
	put_le(payload, 90U, 7U, 2U);

	CHECK_EQ(ubx_nav_pvt_decode(&frame, &pvt), UBX_NAV_PVT_DECODE_OK);
	CHECK_EQ(pvt.i_tow, 123456U);
	CHECK_EQ(pvt.year, 2026U);
	CHECK_EQ(pvt.month, 9U);
	CHECK_EQ(pvt.day, 28U);
	CHECK_EQ(pvt.fix_type, UBX_NAV_PVT_FIX_3D);
	CHECK_EQ(pvt.satellites_used, 17U);
	CHECK_EQ(pvt.longitude, -1234567);
	CHECK_EQ(pvt.latitude, 515000000);
	CHECK_EQ(pvt.velocity_down, -500);
	CHECK_EQ(pvt.p_dop, 123U);
	CHECK_EQ(pvt.heading_vehicle, -9000000);
	CHECK_EQ(pvt.magnetic_declination, -42);
	CHECK_EQ(pvt.magnetic_accuracy, 7U);
	CHECK_EQ(ubx_nav_pvt_get_utc_standard(pvt.valid), UBX_NAV_PVT_UTC_STANDARD_NIST);
	CHECK_EQ(ubx_nav_pvt_get_utc_standard(0xF0U), UBX_NAV_PVT_UTC_STANDARD_UNKNOWN);

	frame.payload_length = UBX_NAV_PVT_PAYLOAD_LENGTH - 1U;
	CHECK_EQ(ubx_nav_pvt_decode(&frame, &pvt), UBX_NAV_PVT_DECODE_WRONG_LENGTH);
	frame.payload_length = UBX_NAV_PVT_PAYLOAD_LENGTH;
	frame.payload = NULL;
	CHECK_EQ(ubx_nav_pvt_decode(&frame, &pvt), UBX_NAV_PVT_DECODE_NULL_ARGUMENT);
	frame.payload = payload;
	frame.message_id = 0x01U;
	CHECK_EQ(ubx_nav_pvt_decode(&frame, &pvt), UBX_NAV_PVT_DECODE_WRONG_MESSAGE);
}

int main(void){
	test_ack();
	test_ack_null_payload();
	test_tim_tp();
	test_nav_pvt();

	return TEST_RESULT();
}
