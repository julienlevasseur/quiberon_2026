// Speed level of the bike, 0..255, from the generator voltage.
//
// The generator gives a DC voltage proportional to the pedalling speed. The
// raw ADC is mapped linearly between the noise floor (cfg::ADC_REST -> 0) and
// a hard sprint (cfg::ADC_MAX -> 255), then smoothed by a first-order low-pass
// with time constant cfg::SPEED_TAU_MS. The filter is driven by the elapsed
// time, not by the number of calls, so it behaves the same whatever the loop
// period (the LED refresh makes it vary). Integer only: the Uno has no FPU.
#pragma once

#include <stdint.h>

namespace qb {

class SpeedSensor {
public:
    SpeedSensor() : state_(0) {}

    // Unfiltered mapping of an averaged ADC reading to 0..255.
    static uint8_t rawLevel(uint16_t adc);

    // Advance the filter by dtMs toward the level of this reading.
    uint8_t update(uint16_t adc, uint16_t dtMs);

    // Filtered level, 0..255.
    uint8_t level() const { return static_cast<uint8_t>((state_ + 128u) >> 8); }

    void reset() { state_ = 0; }

private:
    uint16_t state_;  // filtered level, 8.8 fixed point
};

}  // namespace qb
