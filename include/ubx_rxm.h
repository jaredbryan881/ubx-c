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

// build the UBX-RXM-PMREQ backup payload
ubx_rxm_pmreq_build_result_t
ubx_rxm_pmreq_build_backup(const ubx_rxm_pmreq_backup_t *request,
                           uint8_t *payload, size_t capacity,
                           size_t *payload_length);

#ifdef __cplusplus
}
#endif

#endif
