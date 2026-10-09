// Transmit-only bit-banged UART, 9600 8N1, idle high, on any digital pin.
//
// Replaces SoftwareSerial for the DFPlayer: no 64-byte RX buffer, no pin
// change interrupt, 3 bytes of state (+ the vtable pointer). write() blocks
// ~1.04 ms per byte, ~10.4 ms for a DFPlayer frame: send between LED frames.
#pragma once

#ifdef ARDUINO

#include <stdint.h>

#include "../core/hal.h"

namespace qb {

class TxSerial : public ByteSink {
public:
    TxSerial() : out_(0), mask_(0) {}

    // Sets the pin as output, idle high. Call once in setup().
    void begin(uint8_t pin);

    void write(const uint8_t* data, uint8_t len) override;

private:
    void writeByte(uint8_t b);

    volatile uint8_t* out_;  // PORTx of the pin
    uint8_t mask_;           // its bit in PORTx
};

}  // namespace qb

#endif  // ARDUINO
