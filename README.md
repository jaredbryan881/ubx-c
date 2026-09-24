# ubx-c

A basic C library for communicating with u-blox GNSS receivers using the UBX binary protocol.

Planned functionality includes:

- Streaming UBX frame parsing and checksum validation
- UBX message encoding
- Navigation, position, and timing message decoding
- Receiver configuration and acknowledgement handling
- Raw GNSS measurement decoding for receivers with that capability (e.g. ZED-F9P)
