#include "ubx_rxm.h"
#include "ubx_internal.h"

ubx_rxm_pmreq_build_result_t
ubx_rxm_pmreq_build_backup(const ubx_rxm_pmreq_backup_t *request,
                           uint8_t *payload, size_t capacity,
                           size_t *payload_length) {
  uint32_t flags;

  if (payload_length == NULL) {
    return UBX_RXM_PMREQ_BUILD_NULL_ARGUMENT;
  }

  *payload_length = 0U;

  if ((request == NULL) || (payload == NULL)) {
    return UBX_RXM_PMREQ_BUILD_NULL_ARGUMENT;
  }

  if (capacity < UBX_RXM_PMREQ_PAYLOAD_LENGTH) {
    return UBX_RXM_PMREQ_BUILD_BUFFER_TOO_SMALL;
  }

  if (request->duration > UBX_RXM_PMREQ_MAX_DURATION) {
    return UBX_RXM_PMREQ_BUILD_INVALID_DURATION;
  }

  if (request->force > 1U) {
    return UBX_RXM_PMREQ_BUILD_INVALID_FORCE;
  }

  if ((request->wakeup_sources & (uint32_t)~UBX_RXM_PMREQ_WAKEUP_MASK) != 0U) {
    return UBX_RXM_PMREQ_BUILD_INVALID_WAKEUP_SOURCES;
  }

  flags = UBX_RXM_PMREQ_FLAG_BACKUP;

  if (request->force != 0U) {
    flags |= UBX_RXM_PMREQ_FLAG_FORCE;
  }

  payload[0] = UBX_RXM_PMREQ_VERSION_0;
  payload[1] = 0U; // reserved
  payload[2] = 0U; // reserved
  payload[3] = 0U; // reserved

  // Multi-byte PMREQ
  ubx_write_u32_le(&payload[4], request->duration);
  ubx_write_u32_le(&payload[8], flags);
  ubx_write_u32_le(&payload[12], request->wakeup_sources);

  *payload_length = UBX_RXM_PMREQ_PAYLOAD_LENGTH;

  return UBX_RXM_PMREQ_BUILD_OK;
}
