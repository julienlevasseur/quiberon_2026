#include "doctest.h"

#include "core/config.h"
#include "core/game.h"

#include <vector>

namespace {

using namespace qb;

struct Event {
    uint32_t atMs;
    Phase phase;
};

// Drives a Game with a fake clock and scripted inputs, recording phase changes.
struct Sim {
    Game game;
    Inputs in;
    std::vector<Event> events;
    uint16_t stepMs;

    explicit Sim(uint32_t startMs = 0, uint16_t step = 1) : stepMs(step) {
        in.nowMs = startMs;
        in.generatorAdc = 0;
        in.buttonDown = false;
        in.mp3Busy = false;
        tick();
    }
    void tick() {
        game.tick(in);
        if (game.changed()) events.push_back(Event{in.nowMs, game.phase()});
    }
    // Advances by durationMs (one tick per stepMs, first one after stepMs).
    void run(uint32_t durationMs) {
        for (uint32_t t = 0; t < durationMs; t += stepMs) {
            in.nowMs += stepMs;
            tick();
        }
    }
    void press() {
        in.buttonDown = true;
        run(100);
        in.buttonDown = false;
        run(100);
    }
    // Runs until the phase changes (or maxMs elapses); returns the time spent.
    uint32_t runUntilChange(uint32_t maxMs) {
        const uint32_t t0 = in.nowMs;
        for (uint32_t t = 0; t < maxMs; t += stepMs) {
            in.nowMs += stepMs;
            tick();
            if (game.changed()) break;
        }
        return in.nowMs - t0;
    }
    std::vector<Phase> phases() const {
        std::vector<Phase> p;
        for (size_t i = 0; i < events.size(); ++i) p.push_back(events[i].phase);
        return p;
    }
};

// A countdown track: BUSY rises 200 ms after the play command, falls after 6 s.
void playCountdownTrack(Sim& s) {
    s.run(200);
    s.in.mp3Busy = true;
    s.run(5800);
    s.in.mp3Busy = false;
}

}  // namespace

TEST_SUITE("game") {

TEST_CASE("starts in IDLE without an event and stays there") {
    Sim s(1234);
    CHECK(s.game.phase() == PHASE_IDLE);
    CHECK_FALSE(s.game.changed());
    s.in.generatorAdc = cfg::ADC_MAX;
    s.run(600000);
    CHECK(s.game.phase() == PHASE_IDLE);
    CHECK(s.events.empty());
    CHECK(s.game.remainingMs() == 0);
    CHECK(s.game.kcal() == 0);
}

TEST_CASE("full timeline with exact phase-change events") {
    Sim s(1000);
    s.in.buttonDown = true;
    const uint32_t pressAt = s.in.nowMs;
    s.runUntilChange(1000);
    REQUIRE(s.game.phase() == PHASE_COUNTDOWN);
    CHECK(s.in.nowMs - pressAt == 1 + cfg::BUTTON_DEBOUNCE_MS);  // seen on the next tick, then stable
    CHECK(s.game.entered(PHASE_COUNTDOWN));
    CHECK(s.game.elapsedMs() == 0);
    CHECK(s.game.kcal() == 0);
    s.in.buttonDown = false;
    const uint32_t countdownAt = s.in.nowMs;

    s.run(1);
    CHECK_FALSE(s.game.changed());  // the event lasts exactly one tick

    playCountdownTrack(s);
    s.runUntilChange(1000);
    REQUIRE(s.game.phase() == PHASE_RIDE);
    CHECK(s.in.nowMs - countdownAt == 6001 + 1);  // first tick with BUSY low
    const uint32_t rideAt = s.in.nowMs;
    CHECK(s.game.remainingMs() == cfg::RIDE_MS);

    s.run(60000);
    CHECK(s.game.elapsedMs() == 60000);
    CHECK(s.game.remainingMs() == cfg::RIDE_MS - 60000);

    s.runUntilChange(cfg::RIDE_MS);
    REQUIRE(s.game.phase() == PHASE_FINALE);
    CHECK(s.in.nowMs - rideAt == cfg::RIDE_MS);
    const uint32_t finaleAt = s.in.nowMs;
    CHECK(s.game.remainingMs() == cfg::FINALE_MS);

    s.runUntilChange(cfg::FINALE_MS + 10);
    REQUIRE(s.game.phase() == PHASE_IDLE);
    CHECK(s.in.nowMs - finaleAt == cfg::FINALE_MS);

    s.run(10000);
    const Phase expected[] = {PHASE_COUNTDOWN, PHASE_RIDE, PHASE_FINALE, PHASE_IDLE};
    CHECK(s.phases() == std::vector<Phase>(expected, expected + 4));
}

TEST_CASE("countdown ends when BUSY is released") {
    Sim s;
    s.press();
    REQUIRE(s.game.phase() == PHASE_COUNTDOWN);
    s.in.mp3Busy = true;
    s.run(3000);
    CHECK(s.game.phase() == PHASE_COUNTDOWN);
    s.in.mp3Busy = false;
    s.run(1);
    CHECK(s.game.entered(PHASE_RIDE));
}

TEST_CASE("countdown ends on timeout when BUSY never reports") {
    Sim s;
    s.in.buttonDown = true;
    s.runUntilChange(1000);
    REQUIRE(s.game.phase() == PHASE_COUNTDOWN);
    s.in.buttonDown = false;
    CHECK(s.game.remainingMs() == cfg::COUNTDOWN_TIMEOUT_MS);
    CHECK(s.runUntilChange(cfg::COUNTDOWN_TIMEOUT_MS + 100) == cfg::COUNTDOWN_TIMEOUT_MS);
    CHECK(s.game.entered(PHASE_RIDE));
}

TEST_CASE("countdown ends on timeout when BUSY stays high") {
    Sim s;
    s.in.mp3Busy = true;
    s.in.buttonDown = true;
    s.runUntilChange(1000);
    s.in.buttonDown = false;
    CHECK(s.runUntilChange(cfg::COUNTDOWN_TIMEOUT_MS + 100) == cfg::COUNTDOWN_TIMEOUT_MS);
    CHECK(s.game.phase() == PHASE_RIDE);
}

TEST_CASE("a BUSY gap right after the play command does not end the countdown") {
    // The previous track is still playing when COUNTDOWN starts; BUSY blips
    // low while the DFPlayer switches to the countdown track.
    Sim s;
    s.in.mp3Busy = true;
    s.in.buttonDown = true;
    s.runUntilChange(1000);
    s.in.buttonDown = false;
    s.run(50);
    s.in.mp3Busy = false;
    s.run(cfg::BUSY_SETTLE_MS / 2);
    s.in.mp3Busy = true;
    s.run(4000);
    CHECK(s.game.phase() == PHASE_COUNTDOWN);
    s.in.mp3Busy = false;
    s.run(1);
    CHECK(s.game.entered(PHASE_RIDE));
}

TEST_CASE("button is ignored outside IDLE") {
    Sim s;
    s.press();
    REQUIRE(s.game.phase() == PHASE_COUNTDOWN);
    s.press();
    s.press();
    CHECK(s.game.phase() == PHASE_COUNTDOWN);
    s.runUntilChange(cfg::COUNTDOWN_TIMEOUT_MS);
    REQUIRE(s.game.phase() == PHASE_RIDE);
    for (int i = 0; i < 20; ++i) s.press();
    CHECK(s.game.phase() == PHASE_RIDE);
    s.runUntilChange(cfg::RIDE_MS);
    REQUIRE(s.game.phase() == PHASE_FINALE);
    s.press();
    CHECK(s.game.phase() == PHASE_FINALE);
    // Held through the end of the finale: no restart when IDLE comes back.
    s.in.buttonDown = true;
    s.runUntilChange(cfg::FINALE_MS);
    REQUIRE(s.game.phase() == PHASE_IDLE);
    s.run(5000);
    CHECK(s.game.phase() == PHASE_IDLE);
    s.in.buttonDown = false;
    s.run(100);
    s.press();  // a fresh press in IDLE starts a new game
    CHECK(s.game.phase() == PHASE_COUNTDOWN);
    const Phase expected[] = {PHASE_COUNTDOWN, PHASE_RIDE, PHASE_FINALE, PHASE_IDLE, PHASE_COUNTDOWN};
    CHECK(s.phases() == std::vector<Phase>(expected, expected + 5));
}

TEST_CASE("calories count only during RIDE and are frozen afterwards") {
    Sim s(0, 10);
    s.in.generatorAdc = cfg::ADC_MAX;  // pedalling hard in IDLE: not counted
    s.run(5000);
    CHECK(s.game.speedLevel() == 255);
    s.press();
    REQUIRE(s.game.phase() == PHASE_COUNTDOWN);
    s.runUntilChange(cfg::COUNTDOWN_TIMEOUT_MS);
    REQUIRE(s.game.entered(PHASE_RIDE));
    CHECK(s.game.millijoules() == 0);  // countdown pedalling not counted
    s.run(10);
    CHECK(s.game.millijoules() == 320UL * 10);
    s.runUntilChange(cfg::RIDE_MS);
    REQUIRE(s.game.entered(PHASE_FINALE));
    CHECK(s.game.millijoules() == 320UL * cfg::RIDE_MS);  // the whole ride, exactly
    const uint16_t score = s.game.kcal();
    CHECK(score == 57);
    s.run(20000);
    CHECK(s.game.kcal() == score);
    s.runUntilChange(cfg::FINALE_MS);
    REQUIRE(s.game.phase() == PHASE_IDLE);
    s.run(60000);
    CHECK(s.game.kcal() == score);  // last score kept in IDLE
    s.press();
    REQUIRE(s.game.phase() == PHASE_COUNTDOWN);
    CHECK(s.game.kcal() == 0);  // reset for the new game
}

TEST_CASE("no pedalling gives 0 kcal, a stop mid-ride stops the count") {
    Sim s(0, 20);
    s.press();
    s.runUntilChange(cfg::COUNTDOWN_TIMEOUT_MS);
    REQUIRE(s.game.phase() == PHASE_RIDE);
    s.run(60000);
    CHECK(s.game.millijoules() == 0);
    s.in.generatorAdc = cfg::ADC_MAX;
    s.run(30000);
    s.in.generatorAdc = 0;
    s.run(5000);  // speed decays
    const uint32_t afterStop = s.game.millijoules();
    s.run(30000);
    CHECK(s.game.millijoules() == afterStop);
    CHECK(s.game.kcal() == doctest::Approx(320.0 * 30 / 1004.16).epsilon(0.05));
}

TEST_CASE("whole game is independent of the loop rate") {
    // Same pedalling profile, ticks every 1 ms vs every 50 ms.
    uint16_t results[2];
    uint32_t rideStart[2];
    const uint16_t steps[] = {1, 50};
    for (int k = 0; k < 2; ++k) {
        Sim s(500, steps[k]);
        s.in.buttonDown = true;
        s.runUntilChange(1000);
        s.in.buttonDown = false;
        s.runUntilChange(cfg::COUNTDOWN_TIMEOUT_MS);
        REQUIRE(s.game.phase() == PHASE_RIDE);
        rideStart[k] = s.in.nowMs;
        const uint32_t t0 = s.in.nowMs;
        while (s.game.phase() == PHASE_RIDE) {
            // Sawtooth effort with a 4 s period.
            const uint32_t t = (s.in.nowMs - t0) % 4000;
            s.in.generatorAdc = static_cast<uint16_t>(cfg::ADC_REST + (cfg::ADC_MAX - cfg::ADC_REST) * t / 4000);
            s.run(steps[k]);
        }
        results[k] = s.game.kcal();
        CHECK(s.game.millijoules() > 0);
    }
    CHECK(results[0] > 10);
    CHECK(int(results[0]) - int(results[1]) <= 1);
    CHECK(int(results[1]) - int(results[0]) <= 1);
    CHECK(rideStart[1] - rideStart[0] <= 100u);  // debounce + timeout quantised to the tick
}

TEST_CASE("millis() wrap-around during a game") {
    Sim s(0xFFFFFFFFUL - 20000, 20);
    s.in.generatorAdc = cfg::ADC_MAX;
    s.press();
    REQUIRE(s.game.phase() == PHASE_COUNTDOWN);
    playCountdownTrack(s);
    s.runUntilChange(1000);
    REQUIRE(s.game.phase() == PHASE_RIDE);
    const uint32_t rideAt = s.in.nowMs;
    CHECK(rideAt > 0xFFFF0000UL);  // the ride starts before the wrap...
    s.run(60000);
    CHECK(s.in.nowMs < 100000UL);  // ...and the clock has wrapped
    CHECK(s.game.phase() == PHASE_RIDE);
    CHECK(s.game.elapsedMs() == 60000);
    CHECK(s.game.remainingMs() == cfg::RIDE_MS - 60000);
    CHECK(s.game.millijoules() == 320UL * 60000);
    s.runUntilChange(cfg::RIDE_MS);
    REQUIRE(s.game.phase() == PHASE_FINALE);
    CHECK(s.in.nowMs - rideAt == cfg::RIDE_MS);
    CHECK(s.game.millijoules() == 320UL * cfg::RIDE_MS);
    CHECK(s.runUntilChange(cfg::FINALE_MS) == cfg::FINALE_MS);
    CHECK(s.game.phase() == PHASE_IDLE);
}

TEST_CASE("countdown timeout across the wrap-around") {
    Sim s(0xFFFFFFFFUL - 3000, 10);
    s.in.buttonDown = true;
    s.runUntilChange(1000);
    s.in.buttonDown = false;
    REQUIRE(s.game.phase() == PHASE_COUNTDOWN);
    CHECK(s.runUntilChange(cfg::COUNTDOWN_TIMEOUT_MS + 100) == cfg::COUNTDOWN_TIMEOUT_MS);
    CHECK(s.game.phase() == PHASE_RIDE);
}

TEST_CASE("requestPhase jumps on the next tick with the entry actions") {
    Sim s;
    s.game.requestPhase(PHASE_RIDE);
    CHECK(s.game.phase() == PHASE_IDLE);
    s.run(1);
    CHECK(s.game.entered(PHASE_RIDE));
    s.in.generatorAdc = cfg::ADC_MAX;
    s.run(10000);
    CHECK(s.game.millijoules() > 0);
    s.game.requestPhase(PHASE_FINALE);
    s.run(1);
    CHECK(s.game.entered(PHASE_FINALE));
    const uint32_t frozen = s.game.millijoules();
    s.run(1000);
    CHECK(s.game.millijoules() == frozen);
    s.game.requestPhase(static_cast<Phase>(7));  // invalid: ignored
    s.run(1);
    CHECK(s.game.phase() == PHASE_FINALE);
    CHECK_FALSE(s.game.changed());
}

TEST_CASE("game state stays small") {
    MESSAGE("sizeof(qb::Game) on the host = " << sizeof(Game));
    CHECK(sizeof(Game) <= 32);
}

}  // TEST_SUITE
