// End-to-end scenarios: the whole firmware (qb::App, the same object the
// sketch runs) on a simulated bench, checking the phases, the sound, the
// display and the garland together, as a visitor would experience them.
#include <string>
#include <vector>

#include "doctest.h"
#include "core/calories.h"
#include "core/config.h"
#include "fakes/sim_rig.h"

using namespace qb;
using fake::Rig;

namespace {

const std::vector<std::string> FULL_GAME_SOUND = {
    "vol 20", "play 1", "play 2", "repeat on", "repeat off", "play 3", "play 4", "stop"};

// Expected calories for a constant effort over the whole ride.
uint16_t expectedKcal(uint8_t level) {
    const uint32_t mj = static_cast<uint32_t>(Calories::watts(level)) * cfg::RIDE_MS;  // W * ms = mJ
    return static_cast<uint16_t>((mj + 502080UL) / 1004160UL);  // 1 kcal = 4184 J / 0.24
}

// Everything recorded per phase while a game is played.
struct Trace {
    uint32_t frames[4] = {0, 0, 0, 0};
    uint64_t brightness[4] = {0, 0, 0, 0};
    uint32_t lit[4] = {0, 0, 0, 0};
    uint8_t  maxChannel[4] = {0, 0, 0, 0};
    void attach(Rig& r) {
        r.onFrame = [this](const Rig& rig) {
            const int p = rig.phase();
            ++frames[p];
            brightness[p] += rig.brightnessSum();
            lit[p] += rig.lit();
            if (rig.maxChannel() > maxChannel[p]) maxChannel[p] = rig.maxChannel();
        };
    }
    double meanBrightness(Phase p) const { return frames[p] ? double(brightness[p]) / frames[p] : 0; }
    double meanLit(Phase p) const { return frames[p] ? double(lit[p]) / frames[p] : 0; }
};

// Plays one full game with a constant effort; returns the time of each phase.
struct Timeline { uint32_t countdown, ride, finale; };
Timeline playGame(Rig& r, uint8_t effort) {
    r.press();
    REQUIRE(r.phase() == PHASE_COUNTDOWN);
    Timeline t;
    t.countdown = r.runUntil([&] { return r.phase() != PHASE_COUNTDOWN; }, 60000) + 150;
    REQUIRE(r.phase() == PHASE_RIDE);
    r.pedal = effort;
    t.ride = r.runUntil([&] { return r.phase() != PHASE_RIDE; }, 400000);
    REQUIRE(r.phase() == PHASE_FINALE);
    t.finale = r.runUntil([&] { return r.phase() != PHASE_FINALE; }, 100000);
    REQUIRE(r.phase() == PHASE_IDLE);
    r.pedal = 0;
    return t;
}

}  // namespace

TEST_SUITE("scenario") {

TEST_CASE("nobody plays: calm idle garland, silence, blank display") {
    Rig r;
    Trace tr;
    tr.attach(r);
    r.run(10 * 60 * 1000);
    CHECK(r.phase() == PHASE_IDLE);
    CHECK(r.mp3.log == std::vector<std::string>({"vol 20"}));
    CHECK(r.mp3.audible() == 0);
    CHECK(r.display.history == std::vector<int>({-1}));
    CHECK(tr.frames[PHASE_IDLE] >= 10 * 60 * 1000 / cfg::FRAME_MS - 2);
    CHECK(tr.maxChannel[PHASE_IDLE] <= cfg::IDLE_BRIGHTNESS);
    CHECK(tr.meanLit(PHASE_IDLE) > 100);  // the scrolling is visible
}

TEST_CASE("someone just passing by pedals without pressing start: nothing starts") {
    Rig r;
    r.run(2000);
    r.pedal = 200;
    r.run(60000);
    CHECK(r.phase() == PHASE_IDLE);
    CHECK(r.mp3.log.size() == 1);
    CHECK(r.display.current() == -1);
}

TEST_CASE("steady rider: full timeline, sound, display and lights") {
    Rig r;
    Trace tr;
    tr.attach(r);
    r.run(3000);
    const Timeline t = playGame(r, 128);

    // Countdown lasts the countdown track (7 s) plus the module latency.
    CHECK(t.countdown >= 7000);
    CHECK(t.countdown <= 7000 + 1000);
    CHECK(t.ride >= cfg::RIDE_MS);
    CHECK(t.ride <= cfg::RIDE_MS + 20);
    CHECK(t.finale >= cfg::FINALE_MS);
    CHECK(t.finale <= cfg::FINALE_MS + 20);

    CHECK(r.mp3.log == FULL_GAME_SOUND);
    CHECK(r.mp3.badFrames == 0);

    const int score = r.display.current();
    CHECK(score == Scoreboard::clamp(r.app.game().kcal()));
    // ~130 W for 3 minutes, minus the speed filter's ramp-up.
    CHECK(score >= expectedKcal(128) - 1);
    CHECK(score <= expectedKcal(128));

    // Every phase was drawn, the effects match the brief.
    CHECK(tr.frames[PHASE_COUNTDOWN] > 0);
    CHECK(tr.meanBrightness(PHASE_RIDE) > tr.meanBrightness(PHASE_IDLE));  // the tree comes alive
    CHECK(tr.maxChannel[PHASE_FINALE] > 150);           // bright bursts
    for (int p = 0; p < 4; ++p) CHECK(tr.maxChannel[p] <= cfg::MAX_BRIGHTNESS);
    // Back to idle: the garland scrolls calmly again.
    r.run(2000);
    CHECK(r.maxChannel() <= cfg::IDLE_BRIGHTNESS);
    CHECK(r.mp3.audible() == 0);
}

TEST_CASE("sprinter beats a slow rider, on the display and on the garland") {
    Rig slow, fast;
    Trace ts, tf;
    ts.attach(slow);
    tf.attach(fast);
    slow.run(3000);
    fast.run(3000);
    playGame(slow, 40);
    playGame(fast, 240);
    CHECK(fast.display.current() > slow.display.current() + 20);
    CHECK(tf.meanBrightness(PHASE_RIDE) > 1.5 * ts.meanBrightness(PHASE_RIDE));
    CHECK(tf.meanLit(PHASE_RIDE) > 1.5 * ts.meanLit(PHASE_RIDE));
}

TEST_CASE("rider stops halfway: calories stop, garland calms down, game still ends on time") {
    Rig r;
    r.run(3000);
    r.press();
    r.runUntil([&] { return r.phase() == PHASE_RIDE; }, 60000);
    r.pedal = 200;
    r.run(60000);
    const uint32_t bright = r.brightnessSum();
    r.pedal = 0;
    r.run(5000);
    const int kcalAtStop = r.display.current();
    CHECK(r.brightnessSum() < bright / 3);
    r.run(60000);
    CHECK(r.display.current() == kcalAtStop);
    CHECK(r.phase() == PHASE_RIDE);
    r.runUntil([&] { return r.phase() != PHASE_RIDE; }, cfg::RIDE_MS);
    CHECK(r.phase() == PHASE_FINALE);
    CHECK(r.display.current() == kcalAtStop);
}

TEST_CASE("start pressed again during countdown, ride and finale: ignored") {
    Rig ref, r;
    ref.run(3000);
    r.run(3000);
    const Timeline a = playGame(ref, 128);

    r.press();
    r.run(2000);
    r.press();                    // countdown
    r.runUntil([&] { return r.phase() == PHASE_RIDE; }, 60000);
    r.pedal = 128;
    for (int i = 0; i < 10; ++i) {  // impatient visitor
        r.run(10000);
        r.press(300);
    }
    r.runUntil([&] { return r.phase() == PHASE_FINALE; }, 400000);
    r.run(5000);
    r.press();                    // finale
    r.runUntil([&] { return r.phase() == PHASE_IDLE; }, 100000);
    CHECK(r.mp3.log == FULL_GAME_SOUND);
    CHECK(r.display.current() == ref.display.current());
    (void)a;
}

TEST_CASE("two games in a row: second starts from 0, music and fireworks again") {
    Rig r;
    r.run(3000);
    playGame(r, 200);
    const int first = r.display.current();
    CHECK(first > 0);
    r.run(20000);
    CHECK(r.display.current() == first);  // last score shown while idle
    playGame(r, 60);
    CHECK(r.display.current() < first);
    // display went back to 0 at the second countdown
    bool sawZero = false;
    for (size_t i = 1; i < r.display.history.size(); ++i)
        sawZero |= r.display.history[i] == 0 && r.display.history[i - 1] == first;
    CHECK(sawZero);
    std::vector<std::string> twice = FULL_GAME_SOUND;
    twice.insert(twice.end(), FULL_GAME_SOUND.begin() + 1, FULL_GAME_SOUND.end());
    CHECK(r.mp3.log == twice);
}

TEST_CASE("slow loop (LED refresh + MP3 frames) gives the same game as a fast loop") {
    Rig fast, slow;
    fast.loopMs = 1;
    slow.loopMs = 23;  // ~9 ms FastLED.show + ~10 ms MP3 frame + the rest
    fast.run(3000);
    slow.run(3000);
    const Timeline a = playGame(fast, 150);
    const Timeline b = playGame(slow, 150);
    CHECK(fast.mp3.log == slow.mp3.log);
    const int df = fast.display.current(), ds = slow.display.current();
    CHECK(df - ds <= 1);
    CHECK(ds - df <= 1);
    CHECK(b.ride - a.ride <= 25);
    CHECK(b.finale - a.finale <= 25);
}

TEST_CASE("degraded MP3 (BUSY unwired, repeat ignored): the game still runs to the end") {
    Rig r;
    r.mp3.busyWired = false;
    r.mp3.ignoresRepeat = true;
    r.run(3000);
    const Timeline t = playGame(r, 128);
    CHECK(t.countdown >= cfg::COUNTDOWN_TIMEOUT_MS);
    CHECK(t.countdown <= cfg::COUNTDOWN_TIMEOUT_MS + 200);
    CHECK(r.mp3.log == FULL_GAME_SOUND);
    CHECK(r.display.current() > 0);
}

TEST_CASE("power-on with the button stuck down does not start a game") {
    Rig r;
    r.button = true;
    r.run(10000);
    CHECK(r.phase() == PHASE_IDLE);
    r.button = false;
    r.run(1000);
    r.press();
    CHECK(r.phase() == PHASE_COUNTDOWN);
}

}  // TEST_SUITE
