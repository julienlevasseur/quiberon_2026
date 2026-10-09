#ifdef ARDUINO

#include "tx_serial.h"

#include <Arduino.h>
#include <util/delay_basic.h>

namespace qb {

namespace {

const uint32_t BAUD = 9600;
const uint16_t BIT_CYCLES = F_CPU / BAUD;  // 1666 at 16 MHz (104.17 us)
// Cycles spent per bit outside the delay (shift, test, port write, loop):
// 12 to 20 in the avr-gcc 7.3 -Os output, depending on the bit value. 14
// gives 413 loops, i.e. every bit within +-0.3 % of 104.17 us (a UART
// tolerates a few %). Re-check the disassembly if writeByte() changes.
const uint16_t BIT_OVERHEAD = 14;
// _delay_loop_2 burns exactly 4 cycles per count, independent of the
// compiler, unlike delayMicroseconds() whose call overhead varies.
const uint16_t BIT_LOOPS = (BIT_CYCLES - BIT_OVERHEAD) / 4;

}  // namespace

void TxSerial::begin(uint8_t pin) {
    out_ = portOutputRegister(digitalPinToPort(pin));
    mask_ = digitalPinToBitMask(pin);
    digitalWrite(pin, HIGH);  // idle level before becoming an output
    pinMode(pin, OUTPUT);
}

void TxSerial::write(const uint8_t* data, uint8_t len) {
    if (!out_) return;  // begin() not called
    for (uint8_t i = 0; i < len; ++i) writeByte(data[i]);
}

void TxSerial::writeByte(uint8_t b) {
    // Locals: stores through the volatile port would otherwise force the
    // compiler to reload out_ and mask_ for every bit.
    volatile uint8_t* const port = out_;
    const uint8_t set = mask_;
    const uint8_t clr = static_cast<uint8_t>(~mask_);
    const uint8_t sreg = SREG;
    // Interrupts off for the start and data bits only (9 bits = 937 us, less
    // than one 1024 us Timer0 overflow, so millis() never loses a tick). The
    // stop bit runs with interrupts back on: an ISR can only stretch it, which
    // the receiver sees as a longer idle line.
    cli();
    *port &= clr;  // start bit
    _delay_loop_2(BIT_LOOPS);
    for (uint8_t bit = 0; bit < 8; ++bit) {  // LSB first
        if (b & 1) *port |= set;
        else       *port &= clr;
        b >>= 1;
        _delay_loop_2(BIT_LOOPS);
    }
    *port |= set;  // stop bit
    SREG = sreg;
    _delay_loop_2(BIT_LOOPS);
}

}  // namespace qb

#endif  // ARDUINO
