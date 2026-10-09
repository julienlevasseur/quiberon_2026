#include "doctest.h"
#include "core/config.h"
#include "core/scoreboard.h"
#include "fakes/fakes.h"

using namespace qb;

namespace {

struct Rig {
    fake::RecordingDisplay display;
    Game game;
    Scoreboard board{display};
    uint32_t now = 1000;
    uint16_t adc = 0;
    bool button = false;

    void run(uint32_t ms, uint32_t stepMs = 10) {
        for (uint32_t t = 0; t < ms; t += stepMs) {
            now += stepMs;
            Inputs in = {now, adc, button, false};
            game.tick(in);
            board.update(game, now);
        }
    }
    // A button held at boot is ignored by Game, so let it run first.
    void press() {
        run(200);
        button = true;
        run(100);
        button = false;
    }
};

}  // namespace

TEST_SUITE("scoreboard") {

TEST_CASE("blank at boot and in IDLE before the first game, written once") {
    Rig r;
    r.run(5000);
    CHECK(r.display.history == std::vector<int>({-1}));
    CHECK(r.board.shown() == Scoreboard::BLANK);
}

TEST_CASE("full game: 0 in countdown, live during ride, frozen in finale, kept in idle") {
    Rig r;
    r.run(500);
    r.press();
    CHECK(r.game.phase() == PHASE_COUNTDOWN);
    CHECK(r.display.current() == 0);
    r.run(cfg::COUNTDOWN_TIMEOUT_MS);
    REQUIRE(r.game.phase() == PHASE_RIDE);

    r.adc = cfg::ADC_MAX;  // sprint for the whole ride
    int last = 0;
    bool grew = false;
    while (r.game.phase() == PHASE_RIDE) {
        r.run(1000);
        CHECK(r.display.current() >= last);  // never goes back
        grew |= r.display.current() > last;
        last = r.display.current();
    }
    CHECK(grew);
    REQUIRE(r.game.phase() == PHASE_FINALE);
    const int final = r.display.current();
    CHECK(final == Scoreboard::clamp(r.game.kcal()));
    CHECK(final > 0);

    const size_t writes = r.display.history.size();
    r.run(cfg::FINALE_MS - 1000);  // still pedalling: value must not move
    CHECK(r.display.history.size() == writes);
    CHECK(r.display.current() == final);

    r.run(2000);
    CHECK(r.game.phase() == PHASE_IDLE);
    CHECK(r.display.current() == (cfg::IDLE_SHOWS_LAST_SCORE ? final : -1));
}

TEST_CASE("the final value equals the game's calories, not a stale rate-limited one") {
    Rig r;
    r.press();
    r.run(cfg::COUNTDOWN_TIMEOUT_MS);
    r.adc = cfg::ADC_MAX / 2;
    r.run(cfg::RIDE_MS + 50, 7);  // odd step: the last live write is not at the end
    REQUIRE(r.game.phase() == PHASE_FINALE);
    CHECK(r.display.current() == Scoreboard::clamp(r.game.kcal()));
}

TEST_CASE("live updates are rate limited") {
    Rig r;
    r.press();
    r.run(cfg::COUNTDOWN_TIMEOUT_MS);
    r.adc = cfg::ADC_MAX;
    const size_t before = r.display.history.size();
    r.run(60000, 1);  // 1 ms loops for a minute
    const size_t writes = r.display.history.size() - before;
    CHECK(writes <= 60000 / cfg::DISPLAY_REFRESH_MS + 1);
    CHECK(writes >= 10);  // but the value does move
}

TEST_CASE("values are clamped to the display") {
    CHECK(Scoreboard::clamp(0) == 0);
    CHECK(Scoreboard::clamp(cfg::DISPLAY_MAX) == cfg::DISPLAY_MAX);
    CHECK(Scoreboard::clamp(cfg::DISPLAY_MAX + 1) == cfg::DISPLAY_MAX);
    CHECK(Scoreboard::clamp(65000) == cfg::DISPLAY_MAX);
}

TEST_CASE("a new game restarts from 0") {
    Rig r;
    r.press();
    r.run(cfg::COUNTDOWN_TIMEOUT_MS);
    r.adc = cfg::ADC_MAX;
    r.run(cfg::RIDE_MS + cfg::FINALE_MS + 100);
    REQUIRE(r.game.phase() == PHASE_IDLE);
    REQUIRE(r.display.current() > 0);
    r.press();
    CHECK(r.display.current() == 0);
}

}  // TEST_SUITE
