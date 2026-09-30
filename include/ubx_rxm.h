#ifndef UBX_RXM_H
#define UBX_RXM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UBX_RXM_CLASS 0x02U

#define UBX_RXM_PMREQ_ID 0x41U
#define UBX_RXM_PMREQ_VERSION_0 0x00U
#define UBX_RXM_PMREQ_PAYLOAD_LENGTH 16U

// max timed backup duration [ms]
#define UBX_RXM_PMREQ_MAX_DURATION UINT32_C(1036800000)
#define UBX_RXM_PMREQ_DURATION_UNTIL_WAKEUP UINT32_C(0)

// UBX-RXM-PMREQ task flags
#define UBX_RXM_PMREQ_FLAG_BACKUP UINT32_C(0x00000002)
#define UBX_RXM_PMREQ_FLAG_FORCE UINT32_C(0x00000004)

// UBX-RXM-PMREQ wakeupSources bits
#define UBX_RXM_PMREQ_WAKEUP_UART_RX UINT32_C(0x00000008)
#define UBX_RXM_PMREQ_WAKEUP_EXTINT0 UINT32_C(0x00000020)
#define UBX_RXM_PMREQ_WAKEUP_EXTINT1 UINT32_C(0x00000040)
#define UBX_RXM_PMREQ_WAKEUP_SPI_CS UINT32_C(0x00000080)

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

// build the UBX-RXM-PMREQ backup payload
ubx_rxm_pmreq_build_result_t
ubx_rxm_pmreq_build_backup(const ubx_rxm_pmreq_backup_t *request,
                           uint8_t *payload, size_t capacity,
                           size_t *payload_length);

#ifdef __cplusplus
}
#endif

#endif