// The whole game wired together: what loop() does, minus the pins.
//
// The sketch samples the pins into Inputs, calls step(), and sends the pixel
// buffer to the LEDs when step() says a new frame was rendered. The scenario
// tests drive this same class with simulated inputs, so what they validate is
// the firmware's own wiring, not a copy of it. Plain C++11.
#pragma once

#include <stdint.h>

#include "audio.h"
#include "dfplayer.h"
#include "game.h"
#include "hal.h"
#include "lights.h"
#include "scoreboard.h"

namespace qb {

class App {
public:
    App(DFPlayer& player, NumberDisplay& display, Rgb* pixels, uint16_t count);

    // One loop iteration. Returns true when a new LED frame was rendered into
    // the pixel buffer and must be shown.
    bool step(const Inputs& in);

    Game&       game() { return game_; }
    const Game& game() const { return game_; }
    Audio&      audio() { return audio_; }
    const Scoreboard& scoreboard() const { return scoreboard_; }
    const Lights& lights() const { return lights_; }

private:
    Game       game_;
    Audio      audio_;
    Scoreboard scoreboard_;
    Lights     lights_;
    Rgb*       pixels_;
    uint16_t   count_;
    uint32_t   lastFrameMs_;
    bool       started_;
};

}  // namespace qb
