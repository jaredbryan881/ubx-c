#ifndef UBX_TIM_H
#define UBX_TIM_H

#include <stdint.h>
#include "ubx_parser.h"

#define UBX_TIM_CLASS             0x0DU
#define UBX_TIM_TP_ID             0x01U
#define UBX_TIM_TP_PAYLOAD_LENGTH 16U

// UBX-TIM-TP flags
#define UBX_TIM_TP_FLAG_TIME_BASE_UTC 0x01U
#define UBX_TIM_TP_FLAG_UTC_AVAILABLE 0x02U
#define UBX_TIM_TP_FLAG_RAIM_MASK     0x0CU
#define UBX_TIM_TP_FLAG_RAIM_SHIFT    2U
#define UBX_TIM_TP_FLAG_QERR_INVALID  0x10U
#define UBX_TIM_TP_FLAG_NOT_LOCKED    0x20U

// Values of the two-bit RAIM field.
#define UBX_TIM_TP_RAIM_UNAVAILABLE 0U
#define UBX_TIM_TP_RAIM_INACTIVE    1U
#define UBX_TIM_TP_RAIM_ACTIVE      2U

// UBX-TIM-TP reference info fields
#define UBX_TIM_TP_REF_GNSS_MASK 0x0FU
#define UBX_TIM_TP_REF_UTC_MASK  0xF0U
#define UBX_TIM_TP_REF_UTC_SHIFT 4U

typedef struct {
	uint32_t tow; // time of week [ms]
	uint32_t tow_sub; // time of week (sub-millisecond portion), scale: 2^-32 ms
	int32_t quantization_error; // quantization error of the next timepulse [ps]
	uint16_t week; // week number

	uint8_t flags;
	uint8_t reference_info;
} ubx_tim_tp_t;

typedef enum {
	UBX_TIM_TP_DECODE_OK = 0,
	UBX_TIM_TP_DECODE_NULL_ARGUMENT,
	UBX_TIM_TP_DECODE_WRONG_MESSAGE,
	UBX_TIM_TP_DECODE_WRONG_LENGTH
} ubx_tim_tp_decode_result_t;

ubx_tim_tp_decode_result_t ubx_tim_tp_decode(const ubx_frame_t *frame, ubx_tim_tp_t *output);

#endif