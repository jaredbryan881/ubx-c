#ifndef UBX_RXM_H
#define UBX_RXM_H

#include <stddef.h>
#include <stdint.h>
#include "ubx_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UBX_RXM_CLASS 0x02U

#define UBX_RXM_PMREQ_ID 0x41U
#define UBX_RXM_PMREQ_VERSION_0 0x00U
#define UBX_RXM_PMREQ_PAYLOAD_LENGTH 16U

// max timed backup duration [ms]
#define UBX_RXM_PMREQ_MAX_DURATION 1036800000U
#define UBX_RXM_PMREQ_DURATION_UNTIL_WAKEUP 0U

// UBX-RXM-PMREQ task flags
#define UBX_RXM_PMREQ_FLAG_BACKUP 0x00000002U
#define UBX_RXM_PMREQ_FLAG_FORCE 0x00000004U

// UBX-RXM-PMREQ wakeupSources bits
#define UBX_RXM_PMREQ_WAKEUP_UART_RX 0x00000008U
#define UBX_RXM_PMREQ_WAKEUP_EXTINT0 0x00000020U
#define UBX_RXM_PMREQ_WAKEUP_EXTINT1 0x00000040U
#define UBX_RXM_PMREQ_WAKEUP_SPI_CS 0x00000080U

#define UBX_RXM_PMREQ_WAKEUP_MASK                                              \
  (UBX_RXM_PMREQ_WAKEUP_UART_RX | UBX_RXM_PMREQ_WAKEUP_EXTINT0 |               \
   UBX_RXM_PMREQ_WAKEUP_EXTINT1 | UBX_RXM_PMREQ_WAKEUP_SPI_CS)

typedef struct {
  uint32_t duration;       // requested backup duration [ms]
  uint32_t wakeup_sources; // bitwise OR of UBX_RXM_PMREQ_WAKEUP_* values
  uint8_t force;           // 0 or 1
} ubx_rxm_pmreq_backup_t;

typedef enum {
  UBX_RXM_PMREQ_BUILD_OK = 0,
  UBX_RXM_PMREQ_BUILD_NULL_ARGUMENT,
  UBX_RXM_PMREQ_BUILD_BUFFER_TOO_SMALL,
  UBX_RXM_PMREQ_BUILD_INVALID_DURATION,
  UBX_RXM_PMREQ_BUILD_INVALID_FORCE,
  UBX_RXM_PMREQ_BUILD_INVALID_WAKEUP_SOURCES
} ubx_rxm_pmreq_build_result_t;

// UBX-RXM-RAWX
#define UBX_RXM_RAWX_ID                 0x015U
#define UBX_RXM_RAWX_VERSION_1          0x01U
#define UBX_RXM_RAWX_HEADER_LENGTH      16U
#define UBX_RXM_RAWX_MEASUREMENT_LENGTH 32U
#define UBX_RXM_RAWX_MAX_MEASUREMENTS   255U
#define UBX_RXM_RAWX_MAX_PAYLOAD_LENGTH 8176U
#define UBX_RXM_RAWX_MAX_FRAME_LENGTH   8184U

#define UBX_RXM_RAWX_REC_LEAP_SECONDS_VALID 0x01U
#define UBX_RXM_RAWX_REC_CLOCK_RESET        0x02U
#define UBX_RXM_RAWX_TRACK_PR_VALID         0x01U
#define UBX_RXM_RAWX_TRACK_CP_VALID         0x02U
#define UBX_RXM_RAWX_TRACK_HALF_CYCLE_VALID 0x04U
#define UBX_RXM_RAWX_TRACK_HALF_CYCLE_SUB   0x08U
#define UBX_RXM_RAWX_STDEV_INDEX_MASK       0x0FU
#define UBX_RXM_RAWX_CP_STDEV_INVALID       0x0FU

typedef enum {
	UBX_RXM_DECODE_OK = 0,
	UBX_RXM_DECODE_NULL_ARGUMENT,
	UBX_RXM_DECODE_WRONG_MESSAGE,
	UBX_RXM_DECODE_WRONG_LENGTH,
	UBX_RXM_DECODE_UNSUPPORTED_VERSION,
	UBX_RXM_DECODE_INDEX_OUT_OF_RANGE,
	UBX_RXM_DECODE_INVALID_VALUE
} ubx_rxm_decode_result_t;

typedef struct {
	double receiver_tow;
	uint16_t week;
	int8_t leap_seconds;
	uint8_t measurement_count;
	uint8_t receiver_status;
	uint8_t version;
} ubx_rxm_rawx_t;

typedef struct {
	double pseudorange;           // m
	double carrier_phase;         // cycles
	float doppler;                // Hz
	uint8_t gnss_id;
	uint8_t satellite_id;
	uint8_t signal_id;
	uint8_t frequency_id;
	uint16_t lock_time;           // ms
	uint8_t cno;                  // dB-Hz
	uint8_t pseudorange_stdev;
	uint8_t carrier_phase_stdev;
	uint8_t doppler_stdev;
	uint8_t tracking_status;
} ubx_rxm_rawx_measurement_t;

ubx_rxm_decode_result_t ubx_rxm_rawx_decode(const ubx_frame_t *frame, 
											ubx_rxm_rawx_t *output);
ubx_rxm_decode_result_t ubx_rxm_rawx_measurement_decode(const ubx_frame_t *frame, 
														uint16_t measurement_index, 
														ubx_rxm_rawx_measurement_t *output);

// scaled uncertainties
double ubx_rxm_rawx_pseudorange_stdev_m(uint8_t raw);
double ubx_rxm_rawx_doppler_stdev_hz(uint8_t raw);
ubx_rxm_decode_result_t ubx_rxm_rawx_carrier_phase_stdev_cycles(uint8_t raw, 
																double *cycles);

// build the UBX-RXM-PMREQ backup payload
ubx_rxm_pmreq_build_result_t
ubx_rxm_pmreq_build_backup(const ubx_rxm_pmreq_backup_t *request,
                           uint8_t *payload, size_t capacity,
                           size_t *payload_length);

// UBX-RXM-SFRBX
#define UBX_RXM_SFRBX_ID                 0x13U
#define UBX_RXM_SFRBX_VERSION_2          0x02U
#define UBX_RXM_SFRBX_HEADER_LENGTH      8U
#define UBX_RXM_SFRBX_WORD_LENGTH        4U
#define UBX_RXM_SFRBX_MAX_WORDS          255U
#define UBX_RXM_SFRBX_MAX_PAYLOAD_LENGTH 1028U
#define UBX_RXM_SFRBX_MAX_FRAME_LENGTH   1036U

typedef struct {
	uint8_t gnss_id;
	uint8_t satellite_id;
	uint8_t signal_id;
	uint8_t frequency_id;
	uint8_t word_count;
	uint8_t channel;
	uint8_t version;
} ubx_rxm_sfrbx_t;

ubx_rxm_decode_result_t ubx_rxm_sfrbx_decode(const ubx_frame_t *frame, 
											 ubx_rxm_sfrbx_t *output);

ubx_rxm_decode_result_t ubx_rxm_sfrbx_word_decode(const ubx_frame_t *frame, 
												  uint16_t word_index, 
												  uint32_t *output);

ubx_rxm_pmreq_build_result_t ubx_rxm_pmreq_build_backup(const ubx_rxm_pmreq_backup_t *request,
														uint8_t *payload,
														size_t capacity,
														size_t *payload_length);

#ifdef __cplusplus
}
#endif

#endif
