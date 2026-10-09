#include "scoreboard.h"

#include "config.h"

namespace qb {

const uint16_t Scoreboard::BLANK;  // C++11: needed when bound to a reference

Scoreboard::Scoreboard(NumberDisplay& display)
    : display_(display), lastWriteMs_(0), shown_(0), started_(false), played_(false) {}

uint16_t Scoreboard::clamp(uint16_t kcal) {
    return kcal > cfg::DISPLAY_MAX ? cfg::DISPLAY_MAX : kcal;
}

void Scoreboard::show(uint16_t value, uint32_t nowMs) {
    if (started_ && value == shown_) return;
    started_ = true;
    shown_ = value;
    lastWriteMs_ = nowMs;
    if (value == BLANK) {
        display_.blank();
    } else {
        display_.showNumber(value);
    }
}

void Scoreboard::update(const Game& game, uint32_t nowMs) {
    const uint16_t score = clamp(game.kcal());
    switch (game.phase()) {
        case PHASE_IDLE:
            show(played_ && cfg::IDLE_SHOWS_LAST_SCORE ? score : BLANK, nowMs);
            break;
        case PHASE_COUNTDOWN:
            played_ = true;
            show(0, nowMs);
            break;
        case PHASE_RIDE:
            // Live value, rate-limited; a phase change always shows at once.
            if (game.changed() || nowMs - lastWriteMs_ >= cfg::DISPLAY_REFRESH_MS) show(score, nowMs);
            break;
        case PHASE_FINALE:
            show(score, nowMs);  // frozen by Game: only written once
            break;
    }
}

}  // namespace qb
