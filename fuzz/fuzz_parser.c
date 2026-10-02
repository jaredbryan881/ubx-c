// libFuzzer target: feed arbitrary bytes through the parser and every decoder.
// Any crash, sanitizer report or hang is a bug.
#include "ubx_ack.h"
#include "ubx_cfg.h"
#include "ubx_nav.h"
#include "ubx_parser.h"
#include "ubx_tim.h"

#include <stddef.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static void exercise_frame(const ubx_frame_t *frame){
	ubx_ack_t ack;
	ubx_nav_pvt_t pvt;
	ubx_tim_tp_t tp;
	ubx_cfg_valget_iterator_t iterator;
	ubx_cfg_valget_item_t item;
	uint8_t scratch_u1;
	uint16_t scratch_u2;
	uint32_t scratch_u4;
	int8_t scratch_i1;
	int16_t scratch_i2;
	int32_t scratch_i4;

	(void)ubx_ack_decode(frame, &ack);
	(void)ubx_nav_pvt_decode(frame, &pvt);
	(void)ubx_tim_tp_decode(frame, &tp);

	if (ubx_cfg_valget_iterator_init(frame, &iterator) == UBX_CFG_DECODE_OK){
		while (ubx_cfg_valget_iterator_next(&iterator, &item) == UBX_CFG_ITERATE_ITEM){
			(void)ubx_cfg_valget_item_get_l(&item, &scratch_u1);
			(void)ubx_cfg_valget_item_get_u1(&item, &scratch_u1);
			(void)ubx_cfg_valget_item_get_u2(&item, &scratch_u2);
			(void)ubx_cfg_valget_item_get_u4(&item, &scratch_u4);
			(void)ubx_cfg_valget_item_get_i1(&item, &scratch_i1);
			(void)ubx_cfg_valget_item_get_i2(&item, &scratch_i2);
			(void)ubx_cfg_valget_item_get_i4(&item, &scratch_i4);
		}
	}
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
	// Small buffer on purpose so the oversize/discard path is reached often
	uint8_t payload[128];
	ubx_parser_t parser;
	ubx_frame_t frame;
	size_t index;

	ubx_parser_init(&parser, payload, sizeof(payload));

	for (index = 0U; index < size; index++){
		if (ubx_parser_feed(&parser, data[index], &frame) == UBX_PARSE_FRAME_COMPLETE){
			exercise_frame(&frame);
		}
	}

	return 0;
}
