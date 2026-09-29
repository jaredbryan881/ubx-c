# ubx-c

A basic C library for communicating with u-blox GNSS receivers using the UBX binary protocol.

## Implemented functionality

- Streaming UBX frame parsing and checksum validation
- UBX message encoding
- Receiver configuration and acknowledgement handling
- Navigation and timing
- Power management
- Receiver health monitoring

## Planned work:

- ZED-F9P compatibility testing
- UBX-RXM-RAWX (raw pseudorange, carrier-phase, and Doppler) decoding
- UBX-RXM-SFRBX (navigation message) decoding
- Additional GNSS timing and lead second messages
- Additional signal quality messages
