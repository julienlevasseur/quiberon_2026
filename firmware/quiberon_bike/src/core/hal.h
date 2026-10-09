// The only doors between the game logic and the hardware.
//
// Inputs are sampled once per loop into a plain struct; outputs go through two
// tiny interfaces. The Arduino adapters live in src/hal/, the test fakes in
// test/fakes/. Plain C++11, no Arduino dependency.
#pragma once

#include <stdint.h>

namespace qb {

// Everything the game reads, sampled once per loop() iteration.
struct Inputs {
    uint32_t nowMs;       // millis(), wraps after ~49 days
    uint16_t generatorAdc; // averaged raw ADC of the generator, 0..1023
    bool     buttonDown;  // start button currently pressed (already inverted from INPUT_PULLUP)
    bool     mp3Busy;     // DFPlayer is playing (already inverted from the BUSY pin)
};

// One pixel, same memory layout as FastLED's CRGB (r, g, b).
struct Rgb {
    uint8_t r, g, b;
};

// Raw bytes to the DFPlayer (hal/tx_serial on the Uno). Send-only: FastLED
// blocks interrupts while it refreshes, so replies cannot be trusted.
class ByteSink {
public:
    virtual void write(const uint8_t* data, uint8_t len) = 0;
protected:
    ~ByteSink() {}
};

// The calorie display, whatever hardware it ends up being.
class NumberDisplay {
public:
    virtual void showNumber(uint16_t value) = 0;
    virtual void blank() = 0;
protected:
    ~NumberDisplay() {}
};

}  // namespace qb
