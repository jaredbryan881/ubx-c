#ifndef UBX_MON_H
#define UBX_MON_H

#include "ubx_parser.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UBX_MON_CLASS 0x0AU

// UBX-MON-VER
#define UBX_MON_VER_ID                  0x04U
#define UBX_MON_VER_BASE_PAYLOAD_LENGTH 40U
#define UBX_MON_VER_SW_VERSION_LENGTH   30U
#define UBX_MON_VER_HW_VERSION_LENGTH   10U
#define UBX_MON_VER_EXTENSION_LENGTH    30U

// UBX-MON-RXR
#define UBX_MON_RXR_ID             0x21U
#define UBX_MON_RXR_PAYLOAD_LENGTH 1U
#define UBX_MON_RXR_FLAG_AWAKE     0x01U

// UBX-MON-COMMS
#define UBX_MON_COMMS_ID             0x36U
#define UBX_MON_COMMS_VERSION_0      0x00U
#define UBX_MON_COMMS_HEADER_LENGTH  8U
#define UBX_MON_COMMS_PORT_LENGTH    40U
#define UBX_MON_COMMS_PROTOCOL_COUNT 4U

#define UBX_MON_COMMS_TX_ERROR_MEMORY     0x01U
#define UBX_MON_COMMS_TX_ERROR_ALLOCATION 0x02U

#define UBX_MON_COMMS_PROTOCOL_UBX    0x00U
#define UBX_MON_COMMS_PROTOCOL_NMEA   0x01U
#define UBX_MON_COMMS_PROTOCOL_RTCM2  0x02U
#define UBX_MON_COMMS_PROTOCOL_RTCM3  0x05U
#define UBX_MON_COMMS_PROTOCOL_SPARTN 0x06U
#define UBX_MON_COMMS_PROTOCOL_NONE   0xFFU

// UBX-MON-RF
#define UBX_MON_RF_ID            0x38U
#define UBX_MON_RF_VERSION_0     0x00U
#define UBX_MON_RF_HEADER_LENGTH 4U
#define UBX_MON_RF_BLOCK_LENGTH  24U
#define UBX_MON_RF_JAMMING_STATE_MASK 0x03U

#define UBX_MON_RF_ANTENNA_STATUS_INIT    0U
#define UBX_MON_RF_ANTENNA_STATUS_UNKNOWN 1U
#define UBX_MON_RF_ANTENNA_STATUS_OK      2U
#define UBX_MON_RF_ANTENNA_STATUS_SHORT   3U
#define UBX_MON_RF_ANTENNA_STATUS_OPEN    4U

#define UBX_MON_RF_ANTENNA_POWER_OFF     0U
#define UBX_MON_RF_ANTENNA_POWER_ON      1U
#define UBX_MON_RF_ANTENNA_POWER_UNKNOWN 2U

#define UBX_MON_RF_GNSS_BAND_UNKNOWN 0U
#define UBX_MON_RF_GNSS_BAND_L1      1U
#define UBX_MON_RF_GNSS_BAND_L2      2U
#define UBX_MON_RF_GNSS_BAND_L3      3U
#define UBX_MON_RF_GNSS_BAND_L5      4U

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
	UBX_MON_DECODE_MALFORMED_PAYLOAD,
	UBX_MON_DECODE_INDEX_OUT_OF_RANGE
} ubx_mon_decode_result_t;

typedef struct {
	char software_version[UBX_MON_VER_SW_VERSION_LENGTH + 1U];
	char hardware_version[UBX_MON_VER_HW_VERSION_LENGTH + 1U];

	uint16_t extension_count;
} ubx_mon_ver_t;

typedef struct {
	// bit 0 set means receiver is awake, not in backup mode.
	uint8_t flags;
} ubx_mon_rxr_t;

typedef struct {
	uint8_t version;
	uint8_t port_count;
	uint8_t tx_errors;

	uint8_t protocol_ids[UBX_MON_COMMS_PROTOCOL_COUNT];
} ubx_mon_comms_t;

typedef struct {
	uint16_t port_id;

	uint16_t tx_pending;   // bytes
	uint32_t tx_bytes;     // bytes sent since startup
	uint8_t tx_usage;      // percent during the last monitoring period
	uint8_t tx_peak_usage; // percent since startup

	uint16_t rx_pending;   // bytes
	uint32_t rx_bytes;     // bytes received since startup
	uint8_t rx_usage;      // percent during the last monitoring period
	uint8_t rx_peak_usage; // percent since startup

	uint16_t overrun_error_count;

	uint16_t message_count[UBX_MON_COMMS_PROTOCOL_COUNT];

	uint32_t skipped_bytes;
} ubx_mon_comms_port_t;

typedef struct {
	uint8_t version;
	uint8_t block_count;
} ubx_mon_rf_t;

typedef struct {
	uint8_t block_id;

	uint8_t flags;
	uint8_t jamming_state;

	uint8_t antenna_status;
	uint8_t antenna_power;

	uint32_t post_status;

	uint16_t noise_per_ms;
	uint16_t agc_count;

	uint8_t cw_suppression;

	int8_t i_offset;
	uint8_t i_magnitude;
	int8_t q_offset;
	uint8_t q_magnitude;

	uint8_t gnss_band;
} ubx_mon_rf_block_t;

typedef struct {
	uint8_t version;
	uint8_t boot_type;

	uint8_t cpu_load;         // current cpu load [%]
	uint8_t cpu_load_max;     // max cpu load [%]

	uint8_t memory_usage;     // current memory usage [%]
	uint8_t memory_usage_max; // max memory usage [%]

	uint8_t io_usage;         // current i/o usage [%]
	uint8_t io_usage_max;     // max i/o usage [%]

	uint32_t run_time;        // time since receiver started [s]

	uint16_t notice_count;
	uint16_t warning_count;
	uint16_t error_count;

	// receiver-reported temperature [deg C]
	// MAX-M10S reports 0 because it doesn't report temperature, but others receivers may
	int8_t temperature;
} ubx_mon_sys_t;

// decode a completed UBX-MON-RXR frame
ubx_mon_decode_result_t ubx_mon_rxr_decode(const ubx_frame_t *frame, 
										   ubx_mon_rxr_t *output);

ubx_mon_decode_result_t ubx_mon_ver_decode(const ubx_frame_t *frame,
										   ubx_mon_ver_t *output);

ubx_mon_decode_result_t ubx_mon_ver_extension_decode(const ubx_frame_t *frame, 
													 uint16_t extension_index,
													 char output[UBX_MON_VER_EXTENSION_LENGTH + 1U]);

ubx_mon_decode_result_t ubx_mon_comms_decode(const ubx_frame_t *frame, 
											 ubx_mon_comms_t *output);

ubx_mon_decode_result_t ubx_mon_comms_port_decode(const ubx_frame_t *frame,
												  uint8_t port_index,
												  ubx_mon_comms_port_t *output);

ubx_mon_decode_result_t ubx_mon_rf_decode(const ubx_frame_t *frame, 
										  ubx_mon_rf_t *output);

ubx_mon_decode_result_t ubx_mon_rf_block_decode(const ubx_frame_t *frame, 
												uint8_t block_index, 
												ubx_mon_rf_block_t *output);

// decode a completed UBX-MON-SYS frame
ubx_mon_decode_result_t ubx_mon_sys_decode(const ubx_frame_t *frame, 
										   ubx_mon_sys_t *output);

#ifdef __cplusplus
}
#endif

#endif
