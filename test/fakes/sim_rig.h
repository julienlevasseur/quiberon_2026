// The whole firmware (qb::App) on a simulated bench: a scripted cyclist on
// the generator, a start button, a simulated MP3 module with its BUSY line,
// a recording display and the 300-pixel garland buffer.
//
// Used by the scenario tests and by the visual simulator (tools/sim).
#pragma once

#include <stdint.h>
#include <functional>
#include <vector>

#include "core/app.h"
#include "core/config.h"
#include "fakes/fakes.h"
#include "fakes/mp3_sim.h"

namespace fake {

struct Rig {
    Mp3Sim                mp3;
    qb::DFPlayer          player{mp3};
    RecordingDisplay      display;
    std::vector<qb::Rgb>  pixels = std::vector<qb::Rgb>(qb::cfg::NUM_LEDS, qb::Rgb{0, 0, 0});
    qb::App               app{player, display, pixels.data(), qb::cfg::NUM_LEDS};

    uint32_t now = 0;
    uint32_t loopMs = 10;    // simulated loop period
    uint8_t  pedal = 0;      // cyclist effort as a speed level 0..255
    bool     button = false;
    uint32_t frames = 0;

    // Called after every rendered frame (for the simulator and LED checks).
    std::function<void(const Rig&)> onFrame;

    Rig() {
        mp3.trackMs[qb::cfg::TRACK_COUNTDOWN] = 7000;
        mp3.trackMs[qb::cfg::TRACK_RIDE]      = 95000;
        mp3.trackMs[qb::cfg::TRACK_APPLAUSE]  = 8000;
        mp3.trackMs[qb::cfg::TRACK_WINNER]    = 60000;
    }

    // The generator voltage, as the ADC sees it, for an effort level.
    static uint16_t adcFor(uint8_t level) {
        return static_cast<uint16_t>(qb::cfg::ADC_REST +
            static_cast<uint32_t>(level) * (qb::cfg::ADC_MAX - qb::cfg::ADC_REST) / 255);
    }

    void step() {
        now += loopMs;
        mp3.now = now;
        qb::Inputs in = {now, adcFor(pedal), button, mp3.busy()};
        if (app.step(in)) {
            ++frames;
            if (onFrame) onFrame(*this);
        }
    }

    void run(uint32_t ms) {
        const uint32_t end = now + ms;
        while (static_cast<int32_t>(end - now) > 0) step();
    }

    // Run until the predicate holds, at most maxMs; returns the time it took.
    uint32_t runUntil(const std::function<bool()>& done, uint32_t maxMs) {
        const uint32_t start = now;
        while (!done() && now - start < maxMs) step();
        return now - start;
    }

    void press(uint32_t holdMs = 150) {
        button = true;
        run(holdMs);
        button = false;
    }

    qb::Phase phase() const { return app.game().phase(); }

    // Garland statistics on the current buffer.
    uint16_t lit(uint8_t threshold = 8) const {
        uint16_t n = 0;
        for (auto& p : pixels) n += (p.r > threshold || p.g > threshold || p.b > threshold);
        return n;
    }
    uint32_t brightnessSum() const {
        uint32_t s = 0;
        for (auto& p : pixels) s += p.r + p.g + p.b;
        return s;
    }
    uint8_t maxChannel() const {
        uint8_t m = 0;
        for (auto& p : pixels) {
            if (p.r > m) m = p.r;
            if (p.g > m) m = p.g;
            if (p.b > m) m = p.b;
        }
        return m;
    }
};

}  // namespace fake
