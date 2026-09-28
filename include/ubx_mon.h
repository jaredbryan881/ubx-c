#ifndef UBX_MON_H
#define UBX_MON_H

#include <stdint.h>

#include "ubx_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UBX_MON_CLASS 0x0AU

// UBX-MON-RXR
#define UBX_MON_RXR_ID             0x21U
#define UBX_MON_RXR_PAYLOAD_LENGTH 1U
#define UBX_MON_RXR_FLAG_AWAKE     0x01U

// UBX-MON-SYS
#define UBX_MON_SYS_ID             0x39U
#define UBX_MON_SYS_PAYLOAD_LENGTH 24U
#define UBX_MON_SYS_VERSION_1      0x01U

// UBX-MON-SYS bootType values
typedef enum {
	UBX_MON_SYS_BOOT_UNKNOWN = 0,
	UBX_MON_SYS_BOOT_COLD_START = 1,
	UBX_MON_SYS_BOOT_WATCHDOG = 2,
	UBX_MON_SYS_BOOT_HARDWARE_RESET = 3,
	UBX_MON_SYS_BOOT_HARDWARE_BACKUP = 4,
	UBX_MON_SYS_BOOT_SOFTWARE_BACKUP = 5,
	UBX_MON_SYS_BOOT_SOFTWARE_RESET = 6,
	UBX_MON_SYS_BOOT_VIO_FAILURE = 7,
	UBX_MON_SYS_BOOT_VDD_X_FAILURE = 8,
	UBX_MON_SYS_BOOT_VDD_RF_FAILURE = 9,
	UBX_MON_SYS_BOOT_V_CORE_HIGH_FAILURE = 10,
	UBX_MON_SYS_BOOT_SYSTEM_RESET = 11
} ubx_mon_sys_boot_type_t;

typedef enum {
	UBX_MON_DECODE_OK = 0,
	UBX_MON_DECODE_NULL_ARGUMENT,
	UBX_MON_DECODE_WRONG_MESSAGE,
	UBX_MON_DECODE_WRONG_LENGTH,
	UBX_MON_DECODE_UNSUPPORTED_VERSION,
	UBX_MON_DECODE_MALFORMED_PAYLOAD
} ubx_mon_decode_result_t;

typedef struct {
	// bit 0 set means receiver is awake, not in backup mode.
	uint8_t flags;
} ubx_mon_rxr_t;

typedef struct {
	uint8_t version;
	uint8_t boot_type;

	uint8_t cpu_load;         // current cpu load [%]
	uint8_t cpu_load_max;     // max cpu load [%]

	uint8_t memory_usage;     // current memory usage [%]
	uint8_t memory_usage_max; // max memory usage [%]

	uint8_t io_usage;         // current i/o usage [%]
	uint8_t io_usage_max;     // max i/o usage [%]

	uint32_t run_time;        // time since receiver started [%]

	uint16_t notice_count;
	uint16_t warning_count;
	uint16_t error_count;

	// receiver-reported temperature [deg C]
	// MAX-M10S reports 0 because it doesn't report temperature, but others receivers may
	int8_t temperature;
} ubx_mon_sys_t;

// decode a completed UBX-MON-RXR frame
ubx_mon_decode_result_t ubx_mon_rxr_decode(const ubx_frame_t *frame, ubx_mon_rxr_t *output);

// decode a completed UBX-MON-SYS frame
ubx_mon_decode_result_t ubx_mon_sys_decode(const ubx_frame_t *frame, ubx_mon_sys_t *output);

#ifdef __cplusplus
}
#endif

#endif