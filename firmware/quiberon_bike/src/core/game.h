// The game timeline: IDLE -> COUNTDOWN -> RIDE -> FINALE -> IDLE (brief section 1).
//
// One tick() per loop with the sampled inputs. The start button only counts in
// IDLE. The countdown lasts as long as its track: it ends when the DFPlayer
// BUSY line drops after having been seen high (edges in the first
// cfg::BUSY_SETTLE_MS are ignored, the previous track may still be finishing),
// or after cfg::COUNTDOWN_TIMEOUT_MS if BUSY never reports. Calories are reset
// at COUNTDOWN, counted during RIDE only and frozen afterwards.
//
// At most one phase change per tick, reported by changed() during that tick so
// lights, sound and display can react. Times use unsigned differences, so the
// millis() wrap-around after 49 days is harmless. Plain C++11.
#pragma once

#include <stdint.h>

#include "button.h"
#include "calories.h"
#include "hal.h"
#include "speed.h"

namespace qb {

enum Phase : uint8_t { PHASE_IDLE = 0, PHASE_COUNTDOWN, PHASE_RIDE, PHASE_FINALE };

class Game {
public:
    Game();

    // Advance the game with the inputs of this loop iteration.
    void tick(const Inputs& in);

    Phase phase() const { return static_cast<Phase>(phase_); }
    // True only on the tick where phase() was entered.
    bool changed() const { return (flags_ & CHANGED) != 0; }
    bool entered(Phase p) const { return changed() && phase_ == p; }

    // Time spent in the current phase, as of the last tick.
    uint32_t elapsedMs() const { return lastMs_ - phaseStartMs_; }
    // Time left in RIDE/FINALE; for COUNTDOWN the timeout bound; 0 in IDLE.
    uint32_t remainingMs() const;

    uint8_t  speedLevel() const { return speed_.level(); }
    uint16_t kcal() const { return calories_.kcal(); }  // not clamped for the display
    uint32_t millijoules() const { return calories_.millijoules(); }

    // Bench console: jump to a phase on the next tick, with the usual entry
    // actions and change event.
    void requestPhase(Phase p) {
        if (p <= PHASE_FINALE) request_ = static_cast<uint8_t>(p | REQUEST);
    }

private:
    enum { STARTED = 1, CHANGED = 2, SAW_BUSY = 4 };
    enum { REQUEST = 0x80 };

    void enter(Phase p, uint32_t nowMs);

    uint32_t    phaseStartMs_;
    uint32_t    lastMs_;
    SpeedSensor speed_;
    Calories    calories_;
    Button      button_;
    uint8_t     phase_;
    uint8_t     flags_;
    uint8_t     request_;
};

}  // namespace qb
