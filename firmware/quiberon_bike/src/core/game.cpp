#include "game.h"

#include "config.h"

namespace qb {

Game::Game() : phaseStartMs_(0), lastMs_(0), phase_(PHASE_IDLE), flags_(0), request_(0) {}

uint32_t Game::remainingMs() const {
    uint32_t total;
    switch (phase_) {
        case PHASE_COUNTDOWN: total = cfg::COUNTDOWN_TIMEOUT_MS; break;
        case PHASE_RIDE:      total = cfg::RIDE_MS; break;
        case PHASE_FINALE:    total = cfg::FINALE_MS; break;
        default:              return 0;
    }
    const uint32_t e = elapsedMs();
    return e >= total ? 0 : total - e;
}

void Game::enter(Phase p, uint32_t nowMs) {
    phase_ = p;
    phaseStartMs_ = nowMs;
    flags_ = static_cast<uint8_t>((flags_ | CHANGED) & ~SAW_BUSY);
    switch (p) {
        case PHASE_COUNTDOWN: calories_.reset(); break;   // display shows 0
        case PHASE_RIDE:      calories_.start(); break;
        default:              calories_.freeze(); break;  // keep the score
    }
}

void Game::tick(const Inputs& in) {
    const uint32_t now = in.nowMs;
    uint32_t dt32 = (flags_ & STARTED) ? now - lastMs_ : 0;
    if (!(flags_ & STARTED)) {
        flags_ |= STARTED;
        phaseStartMs_ = now;
    }
    const uint16_t dt = dt32 > 0xFFFFu ? 0xFFFFu : static_cast<uint16_t>(dt32);
    lastMs_ = now;
    flags_ = static_cast<uint8_t>(flags_ & ~CHANGED);

    speed_.update(in.generatorAdc, dt);
    // The ride's last interval is counted on the tick that leaves RIDE.
    if (phase_ == PHASE_RIDE) calories_.update(speed_.level(), dt);
    const bool pressed = button_.update(in.buttonDown, now);

    if (request_ & REQUEST) {
        const Phase p = static_cast<Phase>(request_ & ~REQUEST);
        request_ = 0;
        enter(p, now);
        return;
    }

    const uint32_t elapsed = now - phaseStartMs_;
    switch (phase_) {
        case PHASE_IDLE:
            if (pressed) enter(PHASE_COUNTDOWN, now);
            break;
        case PHASE_COUNTDOWN:
            if (in.mp3Busy) {
                if (elapsed >= cfg::BUSY_SETTLE_MS) flags_ |= SAW_BUSY;
            } else if (flags_ & SAW_BUSY) {
                enter(PHASE_RIDE, now);
                break;
            }
            if (elapsed >= cfg::COUNTDOWN_TIMEOUT_MS) enter(PHASE_RIDE, now);
            break;
        case PHASE_RIDE:
            if (elapsed >= cfg::RIDE_MS) enter(PHASE_FINALE, now);
            break;
        default:
            if (elapsed >= cfg::FINALE_MS) enter(PHASE_IDLE, now);
            break;
    }
}

}  // namespace qb
