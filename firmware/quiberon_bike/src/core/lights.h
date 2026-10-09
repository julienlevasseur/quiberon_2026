// Non-blocking LED effects for the garland: one frame per render() call.
//
// Every effect draws into a caller-owned buffer of qb::Rgb pixels (the sketch
// passes its FastLED CRGB array, same layout). Nothing here blocks, allocates
// or uses floats; one frame is O(count) cheap integer work.
//
// Brightness cap: the effects themselves never write a channel above
// cfg::MAX_BRIGHTNESS, so the sketch must leave FastLED's global brightness at
// 255 (FastLED.setBrightness(255), the default) to avoid scaling twice.
//
// Index 0 is the bottom of the garland (the start of the strip); "up" means
// towards higher indices.
//
// Plain C++11, no Arduino dependency.
#pragma once

#include <stdint.h>

#include "config.h"
#include "game.h"
#include "hal.h"

namespace qb {
// What the garland shows. The integrator maps the game phase onto it.
enum LightMode {
    LIGHT_IDLE = 0,
    LIGHT_COUNTDOWN,
    LIGHT_RIDE,
    LIGHT_FINALE
};

// Tiny xorshift32 PRNG: deterministic, no rand(), 4 bytes of state.
class Prng {
public:
    explicit Prng(uint32_t seed = 1) { reseed(seed); }
    void reseed(uint32_t seed) { state_ = seed ? seed : 0x9E3779B9UL; }
    uint32_t next();
    // Uniform-ish value in [lo, hi] (hi >= lo).
    uint16_t range(uint16_t lo, uint16_t hi);
private:
    uint32_t state_;
};

// Fireworks: rockets rise from the bottom, burst into sparks that fly apart,
// slow down and fade. The pixel buffer itself holds the trails (it is faded
// every frame), so the buffer must not be touched by anyone else between two
// frames of this effect.
class Fireworks {
public:
    static const uint8_t MAX_PARTICLES = 4;

    Fireworks() { begin(1, 0); }

    // Restart the show at `now`. Rockets are launched until now + launchWindowMs.
    void begin(uint32_t seed, uint32_t now,
               uint32_t launchWindowMs = cfg::FW_LAUNCH_WINDOW_MS);
    void render(Rgb* pixels, uint16_t count, uint32_t now);

    // For tests and the bench log.
    uint8_t rising() const;     // rockets currently climbing
    uint8_t bursting() const;   // bursts currently alive
    uint16_t bursts() const { return bursts_; } // bursts since begin()

private:
    enum State { P_FREE = 0, P_RISING, P_BURST };
    struct Particle {
        uint16_t pos16;   // rocket height, 1/16 LED
        uint16_t peak;    // burst height, LED
        uint16_t age;     // ms since the burst
        uint8_t speed;    // rocket speed, LED/s; burst: reach in LEDs
        uint8_t hue;
        uint8_t state;
        uint8_t salt;     // per-burst twinkle variation
    };

    void launch(uint16_t count);
    static void drawSpark(Rgb* pixels, uint16_t count, int16_t at, Rgb c);

    Particle p_[MAX_PARTICLES];
    Prng rng_;
    uint32_t start_;
    uint32_t last_;
    uint16_t launchWindowMs_; // capped at 60 s
    uint16_t nextLaunchMs_;   // ms since start_ of the next launch
    uint16_t bursts_;
    bool fresh_;              // first frame: clear the buffer
};

// The garland: one effect per mode, all frame-based.
//
// Per loop iteration (every cfg::FRAME_MS):   lights.render(pixels, NUM_LEDS, now, level);
// On a phase change:                          lights.setPhase(game.phase(), now);
// Each render() overwrites the whole buffer except in LIGHT_FINALE, where the
// first frame clears it and later frames build on it (trails).
class Lights {
public:
    explicit Lights(uint32_t seed = 1);

    void setMode(LightMode mode, uint32_t now);
    // Same, from the game phase (PHASE_IDLE -> LIGHT_IDLE, ...).
    void setPhase(Phase phase, uint32_t now);
    LightMode mode() const { return static_cast<LightMode>(mode_); }

    // level: speed level 0..255 (only used in LIGHT_RIDE).
    void render(Rgb* pixels, uint16_t count, uint32_t now, uint8_t level);

    const Fireworks& fireworks() const { return fireworks_; }

private:
    void renderIdle(Rgb* pixels, uint16_t count, uint32_t now) const;
    void renderCountdown(Rgb* pixels, uint16_t count, uint32_t now) const;
    void renderRide(Rgb* pixels, uint16_t count, uint32_t now, uint8_t level);

    Fireworks fireworks_;
    Prng seeds_;          // one fresh seed per finale
    uint32_t start_;      // when the current mode began
    uint32_t last_;       // previous render, for the ride animation
    uint16_t ridePhase_;  // comet offset, 1/1000 LED, modulo one comet period
    uint8_t mode_;
};

// Hue wheel (0 red, 85 green, 170 blue), r + g + b == 255, scaled by v.
Rgb hueColor(uint8_t hue, uint8_t v);

}  // namespace qb
