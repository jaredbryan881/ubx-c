#ifndef UBX_NAV_H
#define UBX_NAV_H

#include <stdint.h>
#include "ubx_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * UBX-NAV-PVT (0x01 0x07)
 * Navigation Position Velocity Time Solution
 */
// Message identification and payload size.
#define UBX_NAV_PVT_CLASS          0x01U
#define UBX_NAV_PVT_ID             0x07U
#define UBX_NAV_PVT_PAYLOAD_LENGTH 92U

// Bits in the valid field
#define UBX_NAV_PVT_VALID_DATE          0x01U
#define UBX_NAV_PVT_VALID_TIME          0x02U
#define UBX_NAV_PVT_FULLY_RESOLVED      0x04U
#define UBX_NAV_PVT_VALID_MAGNETIC_DECL 0x08U

// UTC standard identifier in the upper four bits of the valid field
#define UBX_NAV_PVT_UTC_STANDARD_MASK   0xF0U
#define UBX_NAV_PVT_UTC_STANDARD_SHIFT  4U

// Bits in the flags field
#define UBX_NAV_PVT_FLAG_GNSS_FIX_OK    0x01U
#define UBX_NAV_PVT_FLAG_DIFFERENTIAL   0x02U
#define UBX_NAV_PVT_FLAG_PSM_STATE_MASK 0x1CU
#define UBX_NAV_PVT_FLAG_HEAD_VEH_VALID 0x20U
#define UBX_NAV_PVT_FLAG_CARRIER_MASK   0xC0U

// Bits in the flags2 field
#define UBX_NAV_PVT_FLAG2_CONFIRMED_AVAILABLE 0x20U
#define UBX_NAV_PVT_FLAG2_CONFIRMED_DATE      0x40U
#define UBX_NAV_PVT_FLAG2_CONFIRMED_TIME      0x80U

// Values in the UTC standard portion of the valid field
typedef enum {
	UBX_NAV_PVT_UTC_STANDARD_UNAVAILABLE = 0,
	UBX_NAV_PVT_UTC_STANDARD_CRL         = 1,
	UBX_NAV_PVT_UTC_STANDARD_NIST        = 2,
	UBX_NAV_PVT_UTC_STANDARD_USNO        = 3,
	UBX_NAV_PVT_UTC_STANDARD_BIPM        = 4,
	UBX_NAV_PVT_UTC_STANDARD_EUROPE      = 5,
	UBX_NAV_PVT_UTC_STANDARD_SU          = 6,
	UBX_NAV_PVT_UTC_STANDARD_NTSC        = 7,
	UBX_NAV_PVT_UTC_STANDARD_NPLI        = 8,
	UBX_NAV_PVT_UTC_STANDARD_UNKNOWN     = 15
} ubx_nav_pvt_utc_standard_t;

static inline ubx_nav_pvt_utc_standard_t ubx_nav_pvt_get_utc_standard(uint8_t valid){
	return (ubx_nav_pvt_utc_standard_t)((valid & UBX_NAV_PVT_UTC_STANDARD_MASK) >> UBX_NAV_PVT_UTC_STANDARD_SHIFT);
}

// Values in the fix_type field
typedef enum {
	UBX_NAV_PVT_FIX_NONE             = 0,
	UBX_NAV_PVT_FIX_DEAD_RECKONING   = 1,
	UBX_NAV_PVT_FIX_2D               = 2,
	UBX_NAV_PVT_FIX_3D               = 3,
	UBX_NAV_PVT_FIX_GNSS_DR_COMBINED = 4,
	UBX_NAV_PVT_FIX_TIME_ONLY        = 5
} ubx_nav_pvt_fix_type_t;

typedef struct {
	uint32_t i_tow; // GPS time of week [ms]

	uint16_t year;  // UTC Year
	uint8_t month;  // UTC month [1, 12]
	uint8_t day;    // UTC day of month [1, 31]
	uint8_t hour;   // UTC hour [0, 23]
	uint8_t minute; // UTC minute [0, 59]
	uint8_t second; // UTC second [0, 60]

	uint8_t valid; // UBX_NAV_PVT_VALID_* bit mask

	uint32_t time_accuracy; // Time accuracy estimate [ns]
	int32_t nanosecond;     // Fractional UTC second [ns]

	uint8_t fix_type;
	uint8_t flags; // UBX_NAV_PVT_FLAG_* bit mask
	uint8_t flags2;
	uint8_t satellites_used;

	int32_t longitude; // Scale: 1e-7 degrees
	int32_t latitude;  // Scale: 1e-7 degrees

	int32_t height_ellipsoid; // [mm]
	int32_t height_msl;       // [mm]

	uint32_t horizontal_accuracy; // [mm]
	uint32_t vertical_accuracy;   // [mm]

	int32_t velocity_north; // [mm/s]
	int32_t velocity_east;  // [mm/s]
	int32_t velocity_down;  // [mm/s]
	int32_t ground_speed;   // [mm/s]

	int32_t heading_motion; // Scale: 1e-5 degrees

	uint32_t speed_accuracy; // [mm/s]
	uint32_t heading_accuracy; // Scale: 1e-5 degrees

	uint16_t p_dop; // Position DOP, scale: 0.01
	uint16_t flags3;

	int32_t heading_vehicle; // Scale: 1e-5 degrees

	int16_t magnetic_declination; // Scale: 1e-2 degrees
	uint16_t magnetic_accuracy;   // Scale: 1e-2 degrees
} ubx_nav_pvt_t;

typedef enum {
	UBX_NAV_PVT_DECODE_OK = 0,
	UBX_NAV_PVT_DECODE_NULL_ARGUMENT,
	UBX_NAV_PVT_DECODE_WRONG_MESSAGE,
	UBX_NAV_PVT_DECODE_WRONG_LENGTH
} ubx_nav_pvt_decode_result_t;

// Decode a UBX-NAV-PVT frame
ubx_nav_pvt_decode_result_t ubx_nav_pvt_decode(const ubx_frame_t *frame, ubx_nav_pvt_t *output);

#ifdef __cplusplus
}
#endif

#endif