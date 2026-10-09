#include "lights.h"

namespace qb {

namespace {

// a * b / 256, with 255 standing for 1.0 (scale8(a, 255) == a).
inline uint8_t scale8(uint8_t a, uint8_t b) {
    return static_cast<uint8_t>((static_cast<uint16_t>(a) * (static_cast<uint16_t>(b) + 1)) >> 8);
}

inline Rgb rgb(uint8_t r, uint8_t g, uint8_t b) {
    Rgb c;
    c.r = r;
    c.g = g;
    c.b = b;
    return c;
}

inline Rgb scaled(Rgb c, uint8_t v) {
    return rgb(scale8(c.r, v), scale8(c.g, v), scale8(c.b, v));
}

// Cheap 8-bit hash for twinkles and glitter (no state, no division).
inline uint8_t hash8(uint16_t a, uint16_t b) {
    uint16_t x = static_cast<uint16_t>(a * 167u + b * 73u + 0x5Au);
    x ^= static_cast<uint16_t>(x >> 7);
    x = static_cast<uint16_t>(x * 0x2Du);
    x ^= static_cast<uint16_t>(x >> 8);
    return static_cast<uint8_t>(x);
}

}  // namespace

Rgb hueColor(uint8_t hue, uint8_t v) {
    uint8_t r, g, b;
    if (hue < 85) {
        g = static_cast<uint8_t>(hue * 3);
        r = static_cast<uint8_t>(255 - g);
        b = 0;
    } else if (hue < 170) {
        b = static_cast<uint8_t>((hue - 85) * 3);
        g = static_cast<uint8_t>(255 - b);
        r = 0;
    } else {
        r = static_cast<uint8_t>((hue - 170) * 3);
        b = static_cast<uint8_t>(255 - r);
        g = 0;
    }
    return rgb(scale8(r, v), scale8(g, v), scale8(b, v));
}

// ------------------------------------------------------------------- Prng

uint32_t Prng::next() {
    uint32_t x = state_;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state_ = x;
    return x;
}

uint16_t Prng::range(uint16_t lo, uint16_t hi) {
    const uint32_t span = static_cast<uint32_t>(hi) - lo + 1;
    return static_cast<uint16_t>(lo + (next() >> 8) % span);
}

// -------------------------------------------------------------- Fireworks

void Fireworks::begin(uint32_t seed, uint32_t now, uint32_t launchWindowMs) {
    rng_.reseed(seed);
    for (uint8_t i = 0; i < MAX_PARTICLES; ++i) p_[i].state = P_FREE;
    start_ = now;
    last_ = now;
    // nextLaunchMs_ is 16-bit: keep the window well inside it.
    launchWindowMs_ = static_cast<uint16_t>(launchWindowMs > 60000UL ? 60000UL : launchWindowMs);
    nextLaunchMs_ = 0;  // first rocket right away
    bursts_ = 0;
    fresh_ = true;
}

uint8_t Fireworks::rising() const {
    uint8_t n = 0;
    for (uint8_t i = 0; i < MAX_PARTICLES; ++i) n += p_[i].state == P_RISING;
    return n;
}

uint8_t Fireworks::bursting() const {
    uint8_t n = 0;
    for (uint8_t i = 0; i < MAX_PARTICLES; ++i) n += p_[i].state == P_BURST;
    return n;
}

void Fireworks::launch(uint16_t count) {
    for (uint8_t i = 0; i < MAX_PARTICLES; ++i) {
        Particle& p = p_[i];
        if (p.state != P_FREE) continue;
        p.state = P_RISING;
        p.pos16 = 0;
        p.peak = rng_.range(static_cast<uint16_t>(count / 3), static_cast<uint16_t>(count - 1));
        p.speed = static_cast<uint8_t>(rng_.range(cfg::FW_ROCKET_MIN_SPEED, cfg::FW_ROCKET_MAX_SPEED));
        p.hue = static_cast<uint8_t>(rng_.next() >> 24);
        p.salt = static_cast<uint8_t>(rng_.next() >> 24);
        p.age = 0;
        return;
    }
}

void Fireworks::drawSpark(Rgb* pixels, uint16_t count, int16_t at, Rgb c) {
    if (at < 0 || at >= static_cast<int16_t>(count)) return;
    Rgb& d = pixels[at];
    if (c.r > d.r) d.r = c.r;
    if (c.g > d.g) d.g = c.g;
    if (c.b > d.b) d.b = c.b;
}

void Fireworks::render(Rgb* pixels, uint16_t count, uint32_t now) {
    if (count == 0) return;
    if (fresh_) {
        for (uint16_t i = 0; i < count; ++i) pixels[i] = rgb(0, 0, 0);
        fresh_ = false;
        last_ = now;
    }
    uint32_t dt32 = now - last_;
    last_ = now;
    const uint16_t dt = static_cast<uint16_t>(dt32 > 1000UL ? 1000UL : dt32);

    // 1. Trails: fade what the previous frames drew, proportionally to dt.
    if (dt > 0) {
        const uint32_t loss = static_cast<uint32_t>(cfg::FW_FADE_PER_20MS) * dt / 20;
        const uint8_t keep = loss >= 256 ? 0 : static_cast<uint8_t>(255 - loss);
        uint8_t* bytes = reinterpret_cast<uint8_t*>(pixels);
        for (uint16_t i = 0; i < count * 3u; ++i) bytes[i] = scale8(bytes[i], keep);
    }

    // 2. Launch rockets on a random schedule while the window is open.
    const uint32_t elapsed = now - start_;
    while (elapsed < launchWindowMs_ && elapsed >= nextLaunchMs_) {
        const uint8_t before = rising() + bursting();
        launch(count);
        if (rising() + bursting() == before) break;  // all slots busy: retry next frame
        nextLaunchMs_ = static_cast<uint16_t>(
            nextLaunchMs_ + rng_.range(cfg::FW_LAUNCH_MIN_MS, cfg::FW_LAUNCH_MAX_MS));
    }

    // 3. Move and draw rockets and sparks.
    const uint8_t maxB = cfg::MAX_BRIGHTNESS;
    for (uint8_t i = 0; i < MAX_PARTICLES; ++i) {
        Particle& p = p_[i];
        if (p.state == P_RISING) {
            p.pos16 = static_cast<uint16_t>(p.pos16 + static_cast<uint32_t>(p.speed) * dt * 16 / 1000);
            const uint16_t at = static_cast<uint16_t>(p.pos16 >> 4);
            if (at >= p.peak) {
                p.state = P_BURST;
                p.age = 0;
                p.speed = static_cast<uint8_t>(rng_.range(cfg::FW_BURST_MIN_RADIUS, cfg::FW_BURST_MAX_RADIUS));
                ++bursts_;
            } else {
                drawSpark(pixels, count, static_cast<int16_t>(at), rgb(maxB, scale8(maxB, 180), scale8(maxB, 90)));
                if (at > 0) drawSpark(pixels, count, static_cast<int16_t>(at - 1), rgb(scale8(maxB, 120), scale8(maxB, 50), 0));
                continue;
            }
        }
        if (p.state != P_BURST) continue;
        p.age = static_cast<uint16_t>(p.age + dt);
        if (p.age >= cfg::FW_BURST_MS) {
            p.state = P_FREE;
            continue;
        }
        const uint8_t u = static_cast<uint8_t>(static_cast<uint32_t>(p.age) * 256 / cfg::FW_BURST_MS);
        const uint8_t ease = static_cast<uint8_t>((static_cast<uint16_t>(u) * (512u - u)) >> 9 << 1);
        const uint8_t v = scale8(maxB, static_cast<uint8_t>(255 - u));
        const int16_t c = static_cast<int16_t>(p.peak);
        if (p.age < 80) {  // white flash at the burst point
            drawSpark(pixels, count, c, rgb(maxB, maxB, maxB));
            drawSpark(pixels, count, static_cast<int16_t>(c - 1), rgb(v, v, v));
            drawSpark(pixels, count, static_cast<int16_t>(c + 1), rgb(v, v, v));
        }
        const uint8_t tick = static_cast<uint8_t>(p.age / 50);
        for (uint8_t k = 0; k < 3; ++k) {
            const uint8_t reach = static_cast<uint8_t>(p.speed * (k + 1) / 3);
            const int16_t r = scale8(reach, ease);
            const Rgb col = hueColor(static_cast<uint8_t>(p.hue + k * 14), v);
            for (uint8_t side = 0; side < 2; ++side) {
                // Fading sparks twinkle: in the second half each one blinks.
                if (u > 128 && (hash8(static_cast<uint16_t>(p.salt + k * 2 + side), tick) & 0x80)) continue;
                drawSpark(pixels, count, static_cast<int16_t>(side ? c + r : c - r), col);
            }
        }
    }
}

// ----------------------------------------------------------------- Lights

Lights::Lights(uint32_t seed)
    : seeds_(seed), start_(0), last_(0), ridePhase_(0), mode_(LIGHT_IDLE) {}

void Lights::setMode(LightMode mode, uint32_t now) {
    mode_ = static_cast<uint8_t>(mode);
    start_ = now;
    last_ = now;
    ridePhase_ = 0;
    if (mode == LIGHT_FINALE) fireworks_.begin(seeds_.next(), now);
}

void Lights::setPhase(Phase phase, uint32_t now) {
    switch (phase) {
    case PHASE_COUNTDOWN: setMode(LIGHT_COUNTDOWN, now); break;
    case PHASE_RIDE:      setMode(LIGHT_RIDE, now); break;
    case PHASE_FINALE:    setMode(LIGHT_FINALE, now); break;
    default:              setMode(LIGHT_IDLE, now); break;
    }
}

void Lights::render(Rgb* pixels, uint16_t count, uint32_t now, uint8_t level) {
    if (pixels == 0 || count == 0) return;
    switch (mode_) {
    case LIGHT_COUNTDOWN: renderCountdown(pixels, count, now); break;
    case LIGHT_RIDE:      renderRide(pixels, count, now, level); break;
    case LIGHT_FINALE:    fireworks_.render(pixels, count, now); break;
    default:              renderIdle(pixels, count, now); break;
    }
}

// Idle: trains of small comets (red, green, gold), 12 LEDs each, scrolling up
// by exactly one LED every cfg::IDLE_STEP_MS. Pure function of the time.
void Lights::renderIdle(Rgb* pixels, uint16_t count, uint32_t now) const {
    const uint8_t SEG = 12;
    const uint8_t PERIOD = 3 * SEG;
    const uint32_t offset = (now - start_) / cfg::IDLE_STEP_MS;
    uint8_t k = static_cast<uint8_t>((PERIOD - offset % PERIOD) % PERIOD);  // pattern index of pixel 0
    const Rgb colors[3] = {rgb(255, 0, 0), rgb(0, 255, 0), rgb(255, 150, 30)};
    Rgb base[3];
    for (uint8_t s = 0; s < 3; ++s) base[s] = scaled(colors[s], cfg::IDLE_BRIGHTNESS);
    uint8_t seg = static_cast<uint8_t>(k / SEG);
    uint8_t j = static_cast<uint8_t>(k % SEG);
    for (uint16_t i = 0; i < count; ++i) {
        const uint8_t ramp = static_cast<uint8_t>(52 + j * 18);  // 52..250, head on top
        pixels[i] = scaled(base[seg], scale8(ramp, ramp));
        if (++j == SEG) {
            j = 0;
            if (++seg == 3) seg = 0;
        }
    }
}

// Countdown: the garland fills from the bottom, one fifth per step, the
// colour going red -> orange -> yellow -> green and every step starting with a
// bright pulse; after the last step the whole garland flashes green ("GO").
void Lights::renderCountdown(Rgb* pixels, uint16_t count, uint32_t now) const {
    const uint32_t elapsed = now - start_;
    const uint32_t step = elapsed / cfg::COUNTDOWN_STEP_MS;
    const uint16_t frac = static_cast<uint16_t>(elapsed % cfg::COUNTDOWN_STEP_MS);
    uint16_t filled;
    uint8_t hue;
    uint8_t pulse;
    if (step < cfg::COUNTDOWN_SEGMENTS) {
        filled = static_cast<uint16_t>(static_cast<uint32_t>(count) * (step + 1) / cfg::COUNTDOWN_SEGMENTS);
        hue = static_cast<uint8_t>(step * 85 / cfg::COUNTDOWN_SEGMENTS);
        pulse = static_cast<uint8_t>(255 - static_cast<uint32_t>(frac) * 191 / cfg::COUNTDOWN_STEP_MS);
    } else {
        filled = count;
        hue = 85;
        const uint16_t quarter = static_cast<uint16_t>(frac % (cfg::COUNTDOWN_STEP_MS / 4));
        pulse = static_cast<uint8_t>(255 - static_cast<uint32_t>(quarter) * 4 * 191 / cfg::COUNTDOWN_STEP_MS);
    }
    const Rgb on = hueColor(hue, scale8(cfg::MAX_BRIGHTNESS, pulse));
    for (uint16_t i = 0; i < count; ++i) pixels[i] = i < filled ? on : rgb(0, 0, 0);
}

// Ride: comets running up a lit section of the garland. With the level, the
// section gets longer (RIDE_MIN_LEN .. count), brighter (RIDE_MIN_BRIGHTNESS
// .. MAX_BRIGHTNESS), faster, and the hue goes from blue to red; above
// RIDE_GLITTER_LEVEL white glitter sparkles in it. The top of the section is a
// white leading edge. Under it all, one LED in RIDE_BASE_SPACING breathes a
// calm violet glow (RIDE_BASE_MIN..RIDE_BASE_MAX, 4 s period), so that starting to ride
// never makes the tree darker than idle. The comet position integrates speed
// over time and the glow is a function of time, so the image does not depend
// on the frame rate.
void Lights::renderRide(Rgb* pixels, uint16_t count, uint32_t now, uint8_t level) {
    const uint16_t COMET_MILLI = 32000;  // comet period: 32 LEDs, in 1/1000 LED
    const uint8_t span = cfg::MAX_BRIGHTNESS - cfg::RIDE_MIN_BRIGHTNESS;
    const uint8_t bright = static_cast<uint8_t>(cfg::RIDE_MIN_BRIGHTNESS + static_cast<uint16_t>(span) * level / 255);
    const uint16_t minLen = cfg::RIDE_MIN_LEN < count ? cfg::RIDE_MIN_LEN : count;
    const uint16_t len = static_cast<uint16_t>(minLen + static_cast<uint32_t>(count - minLen) * level / 255);
    const uint16_t speed = static_cast<uint16_t>(
        cfg::RIDE_MIN_SPEED + static_cast<uint16_t>(cfg::RIDE_MAX_SPEED - cfg::RIDE_MIN_SPEED) * level / 255);

    const uint32_t dt = now - last_;
    last_ = now;
    ridePhase_ = static_cast<uint16_t>((ridePhase_ + (dt % COMET_MILLI) * speed) % COMET_MILLI);

    const uint16_t sub = static_cast<uint16_t>(static_cast<uint32_t>(ridePhase_) * 16 / 1000);  // 0..511
    uint16_t u = static_cast<uint16_t>((512 - sub) & 511);
    const uint8_t hueBase = static_cast<uint8_t>(160 - static_cast<uint16_t>(160) * level / 255);
    const uint8_t glitter = level > cfg::RIDE_GLITTER_LEVEL
        ? static_cast<uint8_t>((level - cfg::RIDE_GLITTER_LEVEL) / 4) : 0;
    const uint16_t tick = static_cast<uint16_t>(now / 60);

    const uint16_t breath = static_cast<uint16_t>((now - start_) & 4095);
    const uint16_t tri = breath < 2048 ? breath : static_cast<uint16_t>(4095 - breath);  // 0..2047
    const uint8_t baseV = static_cast<uint8_t>(
        cfg::RIDE_BASE_MIN + ((static_cast<uint32_t>(cfg::RIDE_BASE_MAX - cfg::RIDE_BASE_MIN) * tri) >> 11));
    const Rgb base = hueColor(cfg::RIDE_BASE_HUE, baseV);

    uint8_t dot = 0;  // i % RIDE_BASE_SPACING, without a division per pixel
    for (uint16_t i = 0; i < count; ++i, dot = static_cast<uint8_t>(dot + 1 == cfg::RIDE_BASE_SPACING ? 0 : dot + 1)) {
        const Rgb glow = dot == 0 ? base : rgb(0, 0, 0);
        if (i >= len) {
            pixels[i] = glow;
            continue;
        }
        if (i + 1 == len || (glitter && hash8(i, tick) < glitter)) {
            pixels[i] = rgb(bright, bright, bright);
        } else {
            const uint8_t saw = static_cast<uint8_t>(u >> 1);       // 0..255, head at the top
            const uint8_t shape = scale8(saw, saw);
            const uint8_t intensity = static_cast<uint8_t>(48 + scale8(shape, 207));
            Rgb c = hueColor(static_cast<uint8_t>(hueBase + (saw >> 3)), scale8(bright, intensity));
            if (glow.r > c.r) c.r = glow.r;  // comets on top of the glow
            if (glow.g > c.g) c.g = glow.g;
            if (glow.b > c.b) c.b = glow.b;
            pixels[i] = c;
        }
        u = static_cast<uint16_t>((u + 16) & 511);
    }
}

}  // namespace qb
