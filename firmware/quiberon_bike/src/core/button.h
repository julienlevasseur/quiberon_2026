// Debounced start button. The raw input bounces for a few ms on each press and
// release; the game wants exactly one 'pressed' event per real press, so a new
// level is only accepted once it has been stable for cfg::BUTTON_DEBOUNCE_MS.
// Plain C++11, no Arduino dependency.
#pragma once

#include <stdint.h>

namespace qb {

class Button {
public:
    Button();

    // Feed the raw level (true = pressed) once per loop. Returns true on the
    // single tick where a debounced press is recognised. The first call only
    // adopts the current level, so a button held at boot never fires.
    bool update(bool down, uint32_t nowMs);

    // Debounced level.
    bool isDown() const { return (flags_ & STABLE) != 0; }

private:
    enum { STABLE = 1, RAW = 2, INIT = 4 };
    uint8_t  flags_;
    uint16_t changedMs_;  // low 16 bits of millis() at the last raw change; only
                          // compared over a few 10 ms, so the 16-bit wrap is harmless
};

}  // namespace qb
