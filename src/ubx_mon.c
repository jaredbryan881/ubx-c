#include "ubx_mon.h"
#include "ubx_internal.h"
#include <string.h>

static ubx_mon_decode_result_t
ubx_mon_validate_message(const ubx_frame_t *frame, uint8_t message_id) {
  if ((frame == NULL) || (frame->payload == NULL)) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  if ((frame->message_class != UBX_MON_CLASS) ||
      (frame->message_id != message_id)) {
    return UBX_MON_DECODE_WRONG_MESSAGE;
  }

  return UBX_MON_DECODE_OK;
}

static void ubx_mon_copy_string(char *destination, const uint8_t *source,
                                uint16_t field_length) {
  (void)memcpy(destination, source, field_length);
  destination[field_length] = '\0';
}

static ubx_mon_decode_result_t ubx_mon_ver_validate(const ubx_frame_t *frame,
                                                    uint16_t *extension_count) {
  ubx_mon_decode_result_t result;
  uint16_t extension_bytes;

  if (extension_count == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_validate_message(frame, UBX_MON_VER_ID);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  if (frame->payload_length < UBX_MON_VER_BASE_PAYLOAD_LENGTH) {
    return UBX_MON_DECODE_WRONG_LENGTH;
  }

  extension_bytes =
      (uint16_t)(frame->payload_length - UBX_MON_VER_BASE_PAYLOAD_LENGTH);

  if ((extension_bytes % UBX_MON_VER_EXTENSION_LENGTH) != 0U) {
    return UBX_MON_DECODE_MALFORMED_PAYLOAD;
  }

  *extension_count = (uint16_t)(extension_bytes / UBX_MON_VER_EXTENSION_LENGTH);

  return UBX_MON_DECODE_OK;
}

static ubx_mon_decode_result_t ubx_mon_rf_validate(const ubx_frame_t *frame,
                                                   uint8_t *block_count) {
  ubx_mon_decode_result_t result;
  uint8_t count;
  uint32_t expected_length;

  if (block_count == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_validate_message(frame, UBX_MON_RF_ID);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  if (frame->payload_length < UBX_MON_RF_HEADER_LENGTH) {
    return UBX_MON_DECODE_WRONG_LENGTH;
  }

  if (frame->payload[0] != UBX_MON_RF_VERSION_0) {
    return UBX_MON_DECODE_UNSUPPORTED_VERSION;
  }

  count = frame->payload[1];

  expected_length = (uint32_t)UBX_MON_RF_HEADER_LENGTH +
                    ((uint32_t)count * (uint32_t)UBX_MON_RF_BLOCK_LENGTH);

  if ((uint32_t)frame->payload_length != expected_length) {
    return UBX_MON_DECODE_MALFORMED_PAYLOAD;
  }

  *block_count = count;

  return UBX_MON_DECODE_OK;
}

static ubx_mon_decode_result_t ubx_mon_comms_validate(const ubx_frame_t *frame,
                                                      uint8_t *port_count) {
  ubx_mon_decode_result_t result;
  uint8_t count;
  uint32_t expected_length;

  if (port_count == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_validate_message(frame, UBX_MON_COMMS_ID);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  if (frame->payload_length < UBX_MON_COMMS_HEADER_LENGTH) {
    return UBX_MON_DECODE_WRONG_LENGTH;
  }

  if (frame->payload[0] != UBX_MON_COMMS_VERSION_0) {
    return UBX_MON_DECODE_UNSUPPORTED_VERSION;
  }

  count = frame->payload[1];

  expected_length = (uint32_t)UBX_MON_COMMS_HEADER_LENGTH +
                    ((uint32_t)count * (uint32_t)UBX_MON_COMMS_PORT_LENGTH);

  if ((uint32_t)frame->payload_length != expected_length) {
    return UBX_MON_DECODE_MALFORMED_PAYLOAD;
  }

  *port_count = count;

  return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t ubx_mon_ver_decode(const ubx_frame_t *frame,
                                           ubx_mon_ver_t *output) {
  ubx_mon_decode_result_t result;
  uint16_t extension_count;

  if (output == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_ver_validate(frame, &extension_count);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  ubx_mon_copy_string(output->software_version, &frame->payload[0],
                      UBX_MON_VER_SW_VERSION_LENGTH);

  ubx_mon_copy_string(output->hardware_version,
                      &frame->payload[UBX_MON_VER_SW_VERSION_LENGTH],
                      UBX_MON_VER_HW_VERSION_LENGTH);

  output->extension_count = extension_count;

  return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t
ubx_mon_ver_extension_decode(const ubx_frame_t *frame, uint16_t extension_index,
                             char output[UBX_MON_VER_EXTENSION_LENGTH + 1U]) {
  ubx_mon_decode_result_t result;
  uint16_t extension_count;
  uint32_t offset;

  if (output == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_ver_validate(frame, &extension_count);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  if (extension_index >= extension_count) {
    return UBX_MON_DECODE_INDEX_OUT_OF_RANGE;
  }

  offset = (uint32_t)UBX_MON_VER_BASE_PAYLOAD_LENGTH +
           ((uint32_t)extension_index * (uint32_t)UBX_MON_VER_EXTENSION_LENGTH);

  ubx_mon_copy_string(output, &frame->payload[offset],
                      UBX_MON_VER_EXTENSION_LENGTH);

  return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t ubx_mon_rxr_decode(const ubx_frame_t *frame,
                                           ubx_mon_rxr_t *output) {
  ubx_mon_decode_result_t result;

  if (output == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_validate_message(frame, UBX_MON_RXR_ID);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  if (frame->payload_length != UBX_MON_RXR_PAYLOAD_LENGTH) {
    return UBX_MON_DECODE_WRONG_LENGTH;
  }

  output->flags = frame->payload[0];

  return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t ubx_mon_comms_decode(const ubx_frame_t *frame,
                                             ubx_mon_comms_t *output) {
  ubx_mon_decode_result_t result;
  uint8_t port_count;

  if (output == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_comms_validate(frame, &port_count);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  output->version = frame->payload[0];
  output->port_count = port_count;
  output->tx_errors = frame->payload[2];

  (void)memcpy(output->protocol_ids, &frame->payload[4],
               UBX_MON_COMMS_PROTOCOL_COUNT);

  return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t
ubx_mon_comms_port_decode(const ubx_frame_t *frame, uint8_t port_index,
                          ubx_mon_comms_port_t *output) {
  ubx_mon_decode_result_t result;
  uint8_t port_count;
  uint8_t protocol_index;
  uint32_t offset;
  const uint8_t *port;

  if (output == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_comms_validate(frame, &port_count);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  if (port_index >= port_count) {
    return UBX_MON_DECODE_INDEX_OUT_OF_RANGE;
  }

  offset = (uint32_t)UBX_MON_COMMS_HEADER_LENGTH +
           ((uint32_t)port_index * (uint32_t)UBX_MON_COMMS_PORT_LENGTH);

  port = &frame->payload[offset];

  output->port_id = ubx_read_u16_le(&port[0]);

  output->tx_pending = ubx_read_u16_le(&port[2]);

  output->tx_bytes = ubx_read_u32_le(&port[4]);

  output->tx_usage = port[8];
  output->tx_peak_usage = port[9];

  output->rx_pending = ubx_read_u16_le(&port[10]);

  output->rx_bytes = ubx_read_u32_le(&port[12]);

  output->rx_usage = port[16];
  output->rx_peak_usage = port[17];

  output->overrun_error_count = ubx_read_u16_le(&port[18]);

  for (protocol_index = 0U; protocol_index < UBX_MON_COMMS_PROTOCOL_COUNT;
       protocol_index++) {
    output->message_count[protocol_index] =
        ubx_read_u16_le(&port[20U + ((uint32_t)protocol_index * 2U)]);
  }

  output->skipped_bytes = ubx_read_u32_le(&port[36]);

  return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t ubx_mon_rf_decode(const ubx_frame_t *frame,
                                          ubx_mon_rf_t *output) {
  ubx_mon_decode_result_t result;
  uint8_t block_count;

  if (output == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_rf_validate(frame, &block_count);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  output->version = frame->payload[0];
  output->block_count = block_count;

  return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t ubx_mon_rf_block_decode(const ubx_frame_t *frame,
                                                uint8_t block_index,
                                                ubx_mon_rf_block_t *output) {
  ubx_mon_decode_result_t result;
  uint8_t block_count;
  uint32_t offset;
  const uint8_t *block;

  if (output == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_rf_validate(frame, &block_count);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  if (block_index >= block_count) {
    return UBX_MON_DECODE_INDEX_OUT_OF_RANGE;
  }

  offset = (uint32_t)UBX_MON_RF_HEADER_LENGTH +
           ((uint32_t)block_index * (uint32_t)UBX_MON_RF_BLOCK_LENGTH);

  block = &frame->payload[offset];

  output->block_id = block[0];

  output->flags = block[1];
  output->jamming_state = (uint8_t)(block[1] & UBX_MON_RF_JAMMING_STATE_MASK);

  output->antenna_status = block[2];
  output->antenna_power = block[3];

  output->post_status = ubx_read_u32_le(&block[4]);

  output->noise_per_ms = ubx_read_u16_le(&block[12]);

  output->agc_count = ubx_read_u16_le(&block[14]);

  output->cw_suppression = block[16];

  output->i_offset = ubx_read_i8(&block[17]);

  output->i_magnitude = block[18];

  output->q_offset = ubx_read_i8(&block[19]);

  output->q_magnitude = block[20];

  output->gnss_band = block[21];

  return UBX_MON_DECODE_OK;
}

ubx_mon_decode_result_t ubx_mon_sys_decode(const ubx_frame_t *frame,
                                           ubx_mon_sys_t *output) {
  ubx_mon_decode_result_t result;

  if (output == NULL) {
    return UBX_MON_DECODE_NULL_ARGUMENT;
  }

  result = ubx_mon_validate_message(frame, UBX_MON_SYS_ID);

  if (result != UBX_MON_DECODE_OK) {
    return result;
  }

  if (frame->payload_length != UBX_MON_SYS_PAYLOAD_LENGTH) {
    return UBX_MON_DECODE_WRONG_LENGTH;
  }

  if (frame->payload[0] != UBX_MON_SYS_VERSION_1) {
    return UBX_MON_DECODE_UNSUPPORTED_VERSION;
  }

  output->version = frame->payload[0];
  output->boot_type = frame->payload[1];

  output->cpu_load = frame->payload[2];
  output->cpu_load_max = frame->payload[3];

  output->memory_usage = frame->payload[4];
  output->memory_usage_max = frame->payload[5];

  output->io_usage = frame->payload[6];
  output->io_usage_max = frame->payload[7];

  output->run_time = ubx_read_u32_le(&frame->payload[8]);

  output->notice_count = ubx_read_u16_le(&frame->payload[12]);

  output->warning_count = ubx_read_u16_le(&frame->payload[14]);

  output->error_count = ubx_read_u16_le(&frame->payload[16]);

  output->temperature = ubx_read_i8(&frame->payload[18]);

  return UBX_MON_DECODE_OK;
}
