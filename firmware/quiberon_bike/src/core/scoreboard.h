// What the calorie display shows:
//   boot / IDLE before any game   blank
//   COUNTDOWN                     0
//   RIDE                          the calories, live
//   FINALE                        the final calories, frozen
//   IDLE after a game             the last score (cfg::IDLE_SHOWS_LAST_SCORE),
//                                 or blank
// Values are clamped to cfg::DISPLAY_MAX. The display is only written when
// what it shows changes, and live values at most every
// cfg::DISPLAY_REFRESH_MS (some display drivers flicker or block when
// rewritten every loop). Plain C++11.
#pragma once

#include <stdint.h>

#include "game.h"
#include "hal.h"

namespace qb {

class Scoreboard {
public:
    explicit Scoreboard(NumberDisplay& display);

    // Once per loop, right after Game::tick(), with the same nowMs.
    void update(const Game& game, uint32_t nowMs);

    // What the display currently shows; BLANK when blank.
    static const uint16_t BLANK = 0xFFFF;
    uint16_t shown() const { return shown_; }

    static uint16_t clamp(uint16_t kcal);

private:
    void show(uint16_t value, uint32_t nowMs);

    NumberDisplay& display_;
    uint32_t       lastWriteMs_;
    uint16_t       shown_;
    bool           started_;
    bool           played_;  // at least one game since boot
};

}  // namespace qb
