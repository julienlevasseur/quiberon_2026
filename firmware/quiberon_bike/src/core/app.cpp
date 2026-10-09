#include "app.h"

#include "config.h"

namespace qb {

App::App(DFPlayer& player, NumberDisplay& display, Rgb* pixels, uint16_t count)
    : audio_(player), scoreboard_(display), pixels_(pixels), count_(count),
      lastFrameMs_(0), started_(false) {}

bool App::step(const Inputs& in) {
    if (!started_) {
        started_ = true;
        lights_.setPhase(PHASE_IDLE, in.nowMs);
        lastFrameMs_ = in.nowMs - cfg::FRAME_MS;  // first frame right away
    }
    game_.tick(in);
    if (game_.changed()) lights_.setPhase(game_.phase(), in.nowMs);
    audio_.update(game_, in.mp3Busy, in.nowMs);
    scoreboard_.update(game_, in.nowMs);

    if (in.nowMs - lastFrameMs_ < cfg::FRAME_MS) return false;
    lastFrameMs_ = in.nowMs;
    lights_.render(pixels_, count_, in.nowMs, game_.speedLevel());
    return true;
}

}  // namespace qb
