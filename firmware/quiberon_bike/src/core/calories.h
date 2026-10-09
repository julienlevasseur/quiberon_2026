// Calorie counter (brief section 3).
//
// The filtered speed level gives a mechanical power through the calibration
// table cfg::POWER_LEVEL -> cfg::POWER_WATTS (linear interpolation). Energy is
// integrated exactly in millijoules (W x ms) in an integer, so the result does
// not drift and does not depend on the loop period; burnt kcal =
// J / (4184 x cfg::EFFICIENCY_PERCENT / 100). Started at RIDE, frozen at FINALE.
#pragma once

#include <stdint.h>

namespace qb {

class Calories {
public:
    Calories() : millijoules_(0), running_(false) {}

    // Mechanical power (W) for a speed level, from the calibration table.
    static uint16_t watts(uint8_t level);

    void reset() { millijoules_ = 0; running_ = false; }  // 0 kcal, stopped
    void start() { millijoules_ = 0; running_ = true; }   // 0 kcal, counting
    void freeze() { running_ = false; }                    // keep the value
    bool running() const { return running_; }

    // Integrate dtMs at this speed level (ignored when not running).
    void update(uint8_t level, uint16_t dtMs) { addEnergy(watts(level), dtMs); }
    void addEnergy(uint16_t watts, uint16_t dtMs);

    uint32_t millijoules() const { return millijoules_; }
    // Burnt kcal, rounded to the nearest unit (not clamped for the display).
    uint16_t kcal() const;

private:
    uint32_t millijoules_;  // mechanical energy; 320 W x 3 min = 5.8e7, far from 4.3e9
    bool     running_;
};

}  // namespace qb
