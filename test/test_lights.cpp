#include "doctest.h"

#include <string.h>
#include <algorithm>
#include <vector>

#include "core/lights.h"

namespace {

const uint16_t N = qb::cfg::NUM_LEDS;
const uint8_t GUARD = 16;
const uint8_t SENTINEL = 0xA5;

// A pixel buffer with guard pixels on both sides to catch out-of-range writes.
struct Strip {
    std::vector<qb::Rgb> mem;
    uint16_t count;
    explicit Strip(uint16_t n = N) : mem(n + 2 * GUARD), count(n) {
        memset(&mem[0], SENTINEL, mem.size() * sizeof(qb::Rgb));
    }
    qb::Rgb* px() { return &mem[GUARD]; }
    const qb::Rgb& operator[](uint16_t i) const { return mem[GUARD + i]; }
    bool guardsIntact() const {
        for (uint16_t i = 0; i < GUARD; ++i) {
            const qb::Rgb& a = mem[i];
            const qb::Rgb& b = mem[GUARD + count + i];
            if (a.r != SENTINEL || a.g != SENTINEL || a.b != SENTINEL) return false;
            if (b.r != SENTINEL || b.g != SENTINEL || b.b != SENTINEL) return false;
        }
        return true;
    }
    uint8_t maxChannel() const {
        uint8_t m = 0;
        for (uint16_t i = 0; i < count; ++i) {
            const qb::Rgb& p = (*this)[i];
            if (p.r > m) m = p.r;
            if (p.g > m) m = p.g;
            if (p.b > m) m = p.b;
        }
        return m;
    }
    uint16_t lit() const {
        uint16_t n = 0;
        for (uint16_t i = 0; i < count; ++i) {
            const qb::Rgb& p = (*this)[i];
            n += (p.r | p.g | p.b) != 0;
        }
        return n;
    }
    uint32_t energy() const {
        uint32_t e = 0;
        for (uint16_t i = 0; i < count; ++i) e += (*this)[i].r + (*this)[i].g + (*this)[i].b;
        return e;
    }
    std::vector<uint8_t> bytes() const {
        const uint8_t* b = reinterpret_cast<const uint8_t*>(&mem[GUARD]);
        return std::vector<uint8_t>(b, b + count * 3);
    }
};

const qb::LightMode ALL_MODES[] = {qb::LIGHT_IDLE, qb::LIGHT_COUNTDOWN, qb::LIGHT_RIDE, qb::LIGHT_FINALE};

}  // namespace

TEST_CASE("lights: hue wheel keeps r+g+b constant and scales by v") {
    for (int h = 0; h < 256; ++h) {
        const qb::Rgb c = qb::hueColor(static_cast<uint8_t>(h), 255);
        CHECK(c.r + c.g + c.b == 255);
        const qb::Rgb d = qb::hueColor(static_cast<uint8_t>(h), 0);
        CHECK(d.r + d.g + d.b == 0);
    }
}

TEST_CASE("lights: prng is deterministic and stays in range") {
    qb::Prng a(42), b(42), c(43);
    bool differs = false;
    for (int i = 0; i < 100; ++i) {
        const uint32_t x = a.next();
        CHECK(x == b.next());
        differs |= x != c.next();
        const uint16_t r = a.range(10, 20);
        b.range(10, 20);
        CHECK(r >= 10);
        CHECK(r <= 20);
    }
    CHECK(differs);
    qb::Prng zero(0);
    CHECK(zero.next() != 0);  // seed 0 must not lock xorshift at 0
}

TEST_CASE("lights: render never touches pixels out of range, cap respected, all modes") {
    const uint16_t sizes[] = {N, 1, 2, 7, 23, 64};
    for (uint16_t n : sizes) {
        for (qb::LightMode m : ALL_MODES) {
            CAPTURE(n);
            CAPTURE(static_cast<int>(m));
            Strip s(n);
            qb::Lights lights(7);
            lights.setMode(m, 1000);
            for (uint32_t t = 1000; t < 1000 + 32000; t += qb::cfg::FRAME_MS) {
                lights.render(s.px(), n, t, static_cast<uint8_t>((t / 37) & 0xFF));
                REQUIRE(s.guardsIntact());
                REQUIRE(s.maxChannel() <= qb::cfg::MAX_BRIGHTNESS);
            }
        }
    }
}

TEST_CASE("lights: zero count and null buffer are ignored") {
    qb::Lights lights;
    for (qb::LightMode m : ALL_MODES) {
        lights.setMode(m, 0);
        lights.render(0, N, 20, 100);
        Strip s(0);
        lights.render(s.px(), 0, 40, 100);
        CHECK(s.guardsIntact());
    }
}

TEST_CASE("lights: idle scrolls up by exactly one LED per IDLE_STEP_MS") {
    qb::Lights lights;
    lights.setMode(qb::LIGHT_IDLE, 5000);
    Strip a, b;
    for (uint32_t k = 0; k < 50; ++k) {
        const uint32_t t = 5000 + k * qb::cfg::IDLE_STEP_MS;
        lights.render(a.px(), N, t, 0);
        lights.render(b.px(), N, t + qb::cfg::IDLE_STEP_MS, 0);
        for (uint16_t i = 1; i < N; ++i) {
            REQUIRE(b[i].r == a[i - 1].r);
            REQUIRE(b[i].g == a[i - 1].g);
            REQUIRE(b[i].b == a[i - 1].b);
        }
        // Within one step the image does not move.
        Strip c;
        lights.render(c.px(), N, t + qb::cfg::IDLE_STEP_MS - 1, 0);
        CHECK(c.bytes() == a.bytes());
    }
    CHECK(a.maxChannel() <= qb::cfg::IDLE_BRIGHTNESS);
    CHECK(a.lit() == N);  // the whole tree glows while waiting
}

TEST_CASE("lights: idle is calm and independent of the speed level and of frame times") {
    qb::Lights x, y;
    x.setMode(qb::LIGHT_IDLE, 0);
    y.setMode(qb::LIGHT_IDLE, 0);
    Strip a, b;
    for (uint32_t t = 0; t <= 4000; t += 10) {
        x.render(a.px(), N, t, 0);
        if (t % 40 == 0) {
            y.render(b.px(), N, t, 255);
            REQUIRE(a.bytes() == b.bytes());
        }
    }
}

TEST_CASE("lights: countdown fills the garland in steps then flashes green") {
    qb::Lights lights;
    lights.setMode(qb::LIGHT_COUNTDOWN, 100);
    Strip s;
    uint16_t prev = 0;
    for (uint8_t step = 0; step < qb::cfg::COUNTDOWN_SEGMENTS; ++step) {
        const uint32_t t = 100 + step * qb::cfg::COUNTDOWN_STEP_MS;
        lights.render(s.px(), N, t, 0);
        const uint16_t lit = s.lit();
        CHECK(lit == N * (step + 1) / qb::cfg::COUNTDOWN_SEGMENTS);
        CHECK(lit > prev);
        prev = lit;
        // Lit pixels are contiguous from the bottom.
        for (uint16_t i = 0; i < N; ++i) CHECK(((s[i].r | s[i].g | s[i].b) != 0) == (i < lit));
        // Each step starts with a pulse that then decays.
        const uint32_t e0 = s.energy();
        lights.render(s.px(), N, t + qb::cfg::COUNTDOWN_STEP_MS - 50, 0);
        CHECK(s.energy() < e0);
        CHECK(s.lit() == lit);
    }
    // First step red, last green.
    lights.render(s.px(), N, 100, 0);
    CHECK(s[0].r > s[0].g);
    lights.render(s.px(), N, 100 + qb::cfg::COUNTDOWN_SEGMENTS * qb::cfg::COUNTDOWN_STEP_MS, 0);
    CHECK(s.lit() == N);
    CHECK(s[0].g == qb::cfg::MAX_BRIGHTNESS);
    CHECK(s[0].r == 0);
    // ... and keeps flashing until the game moves on (BUSY or timeout).
    uint32_t lo = 0xFFFFFFFF, hi = 0;
    for (uint32_t t = 0; t < 1000; t += 20) {
        lights.render(s.px(), N, 100 + 10000 + t, 0);
        const uint32_t e = s.energy();
        lo = e < lo ? e : lo;
        hi = e > hi ? e : hi;
    }
    CHECK(hi > 2 * lo);
}

namespace {
struct RideStats { double section; double mean; };

// Averages over ~10 s of 20 ms frames: comet-section size and r+g+b per frame.
RideStats stats(qb::LightMode mode, uint8_t level) {
    qb::Lights lights;
    lights.setMode(mode, 0);
    Strip s;
    double section = 0, mean = 0;
    const int frames = 500;
    for (int f = 0; f < frames; ++f) {
        lights.render(s.px(), N, static_cast<uint32_t>(f) * qb::cfg::FRAME_MS, level);
        section += s.lit();
        mean += static_cast<double>(s.energy());
    }
    RideStats r = {section / frames, mean / frames};
    return r;
}
}  // namespace

TEST_CASE("lights: ride section size and mean brightness grow monotonically with level") {
    const uint8_t levels[] = {0, 32, 64, 96, 128, 160, 192, 224, 255};
    RideStats prev = stats(qb::LIGHT_RIDE, levels[0]);
    for (size_t k = 1; k < sizeof(levels); ++k) {
        CAPTURE(static_cast<int>(levels[k]));
        const RideStats cur = stats(qb::LIGHT_RIDE, levels[k]);
        CHECK(cur.section > prev.section);
        CHECK(cur.mean > prev.mean);
        prev = cur;
    }
    // The section growth is readable: glow dots plus a short section at level
    // 0, the whole garland at 255.
    const uint16_t dots = (N + qb::cfg::RIDE_BASE_SPACING - 1) / qb::cfg::RIDE_BASE_SPACING;
    CHECK(stats(qb::LIGHT_RIDE, 0).section < dots + qb::cfg::RIDE_MIN_LEN);
    CHECK(stats(qb::LIGHT_RIDE, 255).section == N);
}

TEST_CASE("lights: riding is never darker than idle, at any level") {
    const double idle = stats(qb::LIGHT_IDLE, 0).mean;
    for (int level = 0; level <= 255; level += 32) {
        CAPTURE(level);
        CHECK(stats(qb::LIGHT_RIDE, static_cast<uint8_t>(level)).mean >= idle);
    }
    CHECK(stats(qb::LIGHT_RIDE, 255).mean >= idle);
}

TEST_CASE("lights: ride level 0 is a calm dotted glow with a few running comets") {
    qb::Lights lights;
    lights.setMode(qb::LIGHT_RIDE, 0);
    Strip a, b;
    lights.render(a.px(), N, 0, 0);
    CHECK(a.maxChannel() <= std::max(qb::cfg::RIDE_MIN_BRIGHTNESS, qb::cfg::RIDE_BASE_MAX));
    // Above the comet section: one glow dot every RIDE_BASE_SPACING LEDs.
    const uint16_t first = (qb::cfg::RIDE_MIN_LEN / qb::cfg::RIDE_BASE_SPACING + 1)
        * qb::cfg::RIDE_BASE_SPACING;  // a glow dot past the section
    const qb::Rgb g = a[first];
    for (uint16_t i = qb::cfg::RIDE_MIN_LEN; i < N; ++i) {
        const bool isDot = i % qb::cfg::RIDE_BASE_SPACING == 0;
        REQUIRE(a[i].r == (isDot ? g.r : 0));
        REQUIRE(a[i].g == (isDot ? g.g : 0));
        REQUIRE(a[i].b == (isDot ? g.b : 0));
    }
    CHECK(g.r + g.g + g.b <= qb::cfg::RIDE_BASE_MAX);
    CHECK(g.r + g.g + g.b + 3 >= qb::cfg::RIDE_BASE_MIN);
    CHECK(a.lit() == qb::cfg::RIDE_MIN_LEN + (N - qb::cfg::RIDE_MIN_LEN) / qb::cfg::RIDE_BASE_SPACING);
    lights.render(b.px(), N, 500, 0);
    CHECK(a.bytes() != b.bytes());  // comets keep running
    // The glow breathes.
    Strip c;
    lights.render(c.px(), N, 2048, 0);
    CHECK(c[first].r + c[first].g + c[first].b > g.r + g.g + g.b + 8);
}

TEST_CASE("lights: ride at full level reaches the cap and lights the whole garland") {
    qb::Lights lights;
    lights.setMode(qb::LIGHT_RIDE, 0);
    Strip s;
    lights.render(s.px(), N, 0, 255);
    CHECK(s.lit() == N);
    CHECK(s.maxChannel() == qb::cfg::MAX_BRIGHTNESS);
}

TEST_CASE("lights: ride animation does not depend on the frame rate") {
    qb::Lights fast, slow;
    fast.setMode(qb::LIGHT_RIDE, 1000);
    slow.setMode(qb::LIGHT_RIDE, 1000);
    Strip a, b;
    for (uint32_t t = 1000; t <= 6000; t += 10) {
        const uint8_t level = static_cast<uint8_t>(t <= 3000 ? 40 : 200);
        fast.render(a.px(), N, t, level);
        if (t % 50 == 0) {
            slow.render(b.px(), N, t, level);
            REQUIRE(a.bytes() == b.bytes());
        }
    }
}

TEST_CASE("fireworks: bursts appear within the finale and the garland ends dark") {
    qb::Lights lights(1234);
    lights.setMode(qb::LIGHT_FINALE, 0);
    Strip s;
    uint32_t firstBurst = 0;
    for (uint32_t t = 0; t <= qb::cfg::FINALE_MS; t += qb::cfg::FRAME_MS) {
        lights.render(s.px(), N, t, 0);
        if (!firstBurst && lights.fireworks().bursts() > 0) firstBurst = t;
    }
    CHECK(firstBurst > 0);
    CHECK(firstBurst < 3000);
    CHECK(lights.fireworks().bursts() >= 15);
    // Everything has burst and faded by the end of the finale.
    CHECK(lights.fireworks().rising() == 0);
    CHECK(lights.fireworks().bursting() == 0);
    CHECK(s.lit() == 0);
}

TEST_CASE("fireworks: a rocket rises, bursts, and its sparks spread and fade") {
    qb::Fireworks fw;
    fw.begin(99, 0, 1);  // a single rocket
    Strip s;
    uint32_t t = 0;
    int16_t lastTop = -1;
    while (fw.bursts() == 0 && t < 5000) {
        fw.render(s.px(), N, t);
        if (fw.rising()) {
            int16_t top = -1;
            for (uint16_t i = 0; i < N; ++i) if (s[i].r == qb::cfg::MAX_BRIGHTNESS) top = static_cast<int16_t>(i);
            CHECK(top >= lastTop);  // the head climbs
            lastTop = top;
        }
        t += qb::cfg::FRAME_MS;
    }
    REQUIRE(fw.bursts() == 1);
    CHECK(lastTop >= N / 3 - 1);
    // Track brightness and spread over the burst.
    uint8_t peakEarly = 0, peakMid = 0, peakLate = 0;
    uint16_t spreadEarly = 0, spreadLate = 0;
    const uint32_t burstAt = t;
    for (; t < burstAt + 3000; t += qb::cfg::FRAME_MS) {
        fw.render(s.px(), N, t);
        const uint32_t age = t - burstAt;
        uint16_t above = 0;  // reach of the highest spark above the burst point
        for (uint16_t i = static_cast<uint16_t>(lastTop + 2); i < N; ++i)
            if ((s[i].r | s[i].g | s[i].b) != 0) above = static_cast<uint16_t>(i - lastTop);
        if (age < 200) { peakEarly = std::max(peakEarly, s.maxChannel()); spreadEarly = std::max(spreadEarly, above); }
        if (age >= 600 && age < 800) peakMid = std::max(peakMid, s.maxChannel());
        if (age >= 1200 && age < 1400) { peakLate = std::max(peakLate, s.maxChannel()); spreadLate = std::max(spreadLate, above); }
    }
    CHECK(peakEarly > peakMid);
    CHECK(peakMid > peakLate);
    CHECK(spreadLate > spreadEarly);  // sparks fly apart
    CHECK(s.lit() == 0);              // and are gone
    CHECK(fw.bursts() == 1);          // no second rocket after the window
}

TEST_CASE("fireworks: same seed gives identical frames, another seed differs") {
    qb::Lights a(77), b(77), c(78);
    a.setMode(qb::LIGHT_FINALE, 500);
    b.setMode(qb::LIGHT_FINALE, 500);
    c.setMode(qb::LIGHT_FINALE, 500);
    Strip sa, sb, sc;
    bool differs = false;
    for (uint32_t t = 500; t < 500 + 10000; t += qb::cfg::FRAME_MS) {
        a.render(sa.px(), N, t, 0);
        b.render(sb.px(), N, t, 0);
        c.render(sc.px(), N, t, 0);
        REQUIRE(sa.bytes() == sb.bytes());
        differs |= sa.bytes() != sc.bytes();
    }
    CHECK(differs);
    // A second finale on the same object uses a new seed: not a rerun.
    qb::Lights d(77);
    d.setMode(qb::LIGHT_FINALE, 0);
    Strip s1, s2;
    for (uint32_t t = 0; t < 5000; t += 20) d.render(s1.px(), N, t, 0);
    d.setMode(qb::LIGHT_FINALE, 0);
    for (uint32_t t = 0; t < 5000; t += 20) d.render(s2.px(), N, t, 0);
    CHECK(s1.bytes() != s2.bytes());
}

TEST_CASE("fireworks: show is close to frame-rate independent") {
    qb::Fireworks x, y;
    x.begin(5, 0);
    y.begin(5, 0);
    Strip a, b;
    for (uint32_t t = 0; t <= 20000; t += 10) {
        x.render(a.px(), N, t);
        if (t % 40 == 0) y.render(b.px(), N, t);
    }
    // Same schedule of bursts whatever the frame period.
    CHECK(x.bursts() == y.bursts());
}

TEST_CASE("lights: phase switches are clean (no leftovers from the previous effect)") {
    qb::Lights lights(3);
    Strip s;
    // RIDE at full speed -> FINALE: the first frame shows only the rocket.
    lights.setMode(qb::LIGHT_RIDE, 0);
    for (uint32_t t = 0; t < 1000; t += 20) lights.render(s.px(), N, t, 255);
    CHECK(s.lit() == N);
    lights.setMode(qb::LIGHT_FINALE, 1000);
    lights.render(s.px(), N, 1000, 255);
    CHECK(s.lit() <= 3);
    // FINALE -> IDLE: identical to a fresh idle.
    for (uint32_t t = 1000; t < 6000; t += 20) lights.render(s.px(), N, t, 0);
    CHECK(s.lit() > 0);
    lights.setMode(qb::LIGHT_IDLE, 6000);
    lights.render(s.px(), N, 6000, 0);
    qb::Lights fresh;
    Strip f;
    fresh.setMode(qb::LIGHT_IDLE, 6000);
    fresh.render(f.px(), N, 6000, 0);
    CHECK(s.bytes() == f.bytes());
    CHECK(lights.mode() == qb::LIGHT_IDLE);
    // IDLE -> COUNTDOWN: starts at the first step.
    lights.setMode(qb::LIGHT_COUNTDOWN, 7000);
    lights.render(s.px(), N, 7000, 0);
    CHECK(s.lit() == N / qb::cfg::COUNTDOWN_SEGMENTS);
    // COUNTDOWN -> RIDE: animation restarts from the first frame.
    lights.setMode(qb::LIGHT_RIDE, 9000);
    lights.render(s.px(), N, 9000, 0);
    qb::Lights fresh2;
    fresh2.setMode(qb::LIGHT_RIDE, 9000);
    fresh2.render(f.px(), N, 9000, 0);
    CHECK(s.bytes() == f.bytes());
}

TEST_CASE("lights: millis() wrap-around does not break the effects") {
    const uint32_t start = 0xFFFFFFFFUL - 500;
    for (qb::LightMode m : ALL_MODES) {
        qb::Lights lights;
        lights.setMode(m, start);
        Strip s;
        for (uint32_t k = 0; k < 100; ++k) {
            lights.render(s.px(), N, start + k * 20, 128);
            REQUIRE(s.guardsIntact());
            REQUIRE(s.maxChannel() <= qb::cfg::MAX_BRIGHTNESS);
        }
        if (m != qb::LIGHT_FINALE) CHECK(s.lit() > 0);
    }
    // Idle keeps scrolling regularly across the wrap.
    qb::Lights idle;
    idle.setMode(qb::LIGHT_IDLE, start);
    Strip a, b;
    idle.render(a.px(), N, start + 480, 0);
    idle.render(b.px(), N, start + 480 + qb::cfg::IDLE_STEP_MS, 0);
    for (uint16_t i = 1; i < N; ++i) CHECK(b[i].r == a[i - 1].r);
}

TEST_CASE("lights: effect state stays small (Uno RAM)") {
    CHECK(sizeof(qb::Fireworks) <= 60);
    CHECK(sizeof(qb::Lights) <= 80);
}

TEST_CASE("lights: game phases map onto light modes") {
    qb::Lights lights;
    lights.setPhase(qb::PHASE_COUNTDOWN, 0);
    CHECK(lights.mode() == qb::LIGHT_COUNTDOWN);
    lights.setPhase(qb::PHASE_RIDE, 0);
    CHECK(lights.mode() == qb::LIGHT_RIDE);
    lights.setPhase(qb::PHASE_FINALE, 0);
    CHECK(lights.mode() == qb::LIGHT_FINALE);
    lights.setPhase(qb::PHASE_IDLE, 0);
    CHECK(lights.mode() == qb::LIGHT_IDLE);
}
