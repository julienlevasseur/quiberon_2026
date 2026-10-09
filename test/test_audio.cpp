#include <string>
#include <vector>

#include "doctest.h"
#include "core/audio.h"
#include "core/config.h"
#include "fakes/mp3_sim.h"

using namespace qb;

namespace {

// Game + Audio + simulated MP3 module, ticked every `stepMs`.
struct Rig {
    fake::Mp3Sim mp3;
    DFPlayer player{mp3};
    Game game;
    Audio audio{player};
    uint32_t now = 1000;
    bool button = false;

    void tick() {
        mp3.now = now;
        Inputs in = {now, 0, button, mp3.busy()};
        game.tick(in);
        audio.update(game, mp3.busy(), now);
    }
    void run(uint32_t ms, uint32_t stepMs = 10) {
        for (uint32_t t = 0; t < ms; t += stepMs) {
            now += stepMs;
            tick();
        }
    }
    void press() {
        button = true;
        run(100);
        button = false;
    }
    // Commands logged since index `from`.
    std::vector<std::string> since(size_t from) const {
        return std::vector<std::string>(mp3.log.begin() + from, mp3.log.end());
    }
};

}  // namespace

TEST_SUITE("audio") {

TEST_CASE("boot: volume only, after the module's boot delay, then silence in IDLE") {
    Rig r;
    r.run(cfg::MP3_BOOT_MS - 50);
    CHECK(r.mp3.log.empty());
    r.run(200);
    CHECK(r.mp3.log == std::vector<std::string>({"vol 20"}));
    r.run(60000);
    CHECK(r.mp3.log.size() == 1);
    CHECK(r.mp3.audible() == 0);
    CHECK(r.mp3.badFrames == 0);
}

TEST_CASE("full game: countdown, looped ride music, applause then winner, silence") {
    Rig r;
    r.mp3.trackMs[cfg::TRACK_COUNTDOWN] = 6000;
    r.mp3.trackMs[cfg::TRACK_RIDE] = 50000;     // shorter than the ride: must loop
    r.mp3.trackMs[cfg::TRACK_APPLAUSE] = 8000;
    r.run(3000);
    r.press();
    CHECK(r.game.phase() == PHASE_COUNTDOWN);
    r.run(1000);
    CHECK(r.mp3.audible() == cfg::TRACK_COUNTDOWN);

    r.run(6000);  // countdown track ends -> RIDE
    CHECK(r.game.phase() == PHASE_RIDE);
    r.run(2000);
    CHECK(r.mp3.audible() == cfg::TRACK_RIDE);
    // Music never stops during the whole ride, although the track is 50 s.
    for (int s = 0; s < 170; ++s) {
        r.run(1000);
        if (r.game.phase() != PHASE_RIDE) break;
        REQUIRE(r.mp3.audible() == cfg::TRACK_RIDE);
    }
    r.run(10000);
    CHECK(r.game.phase() == PHASE_FINALE);
    r.run(1000);
    CHECK(r.mp3.audible() == cfg::TRACK_APPLAUSE);
    r.run(9000);  // applause over -> winner
    CHECK(r.mp3.audible() == cfg::TRACK_WINNER);
    r.run(21000);
    CHECK(r.game.phase() == PHASE_IDLE);
    CHECK(r.mp3.audible() == 0);

    CHECK(r.mp3.log == std::vector<std::string>({
        "vol 20", "play 1", "play 2", "repeat on",
        "repeat off", "play 3", "play 4", "stop"}));
    CHECK(r.mp3.badFrames == 0);
}

TEST_CASE("commands are spaced by at least MP3_CMD_GAP_MS") {
    Rig r;
    r.run(3000);
    r.press();
    r.run(200000, 1);  // 1 ms loop: the fastest possible
    REQUIRE(r.mp3.logMs.size() >= 6);
    for (size_t i = 1; i < r.mp3.logMs.size(); ++i)
        CHECK(r.mp3.logMs[i] - r.mp3.logMs[i - 1] >= cfg::MP3_CMD_GAP_MS);
}

TEST_CASE("a button press during the boot delay still gives the volume first") {
    Rig r;
    r.run(100);
    r.press();
    r.run(3000);
    REQUIRE(r.mp3.log.size() >= 2);
    CHECK(r.mp3.log[0] == "vol 20");
    CHECK(r.mp3.log[1] == "play 1");
}

TEST_CASE("clone ignoring repeat: the ride music is replayed when it ends") {
    Rig r;
    r.mp3.ignoresRepeat = true;
    r.mp3.trackMs[cfg::TRACK_COUNTDOWN] = 5000;
    r.mp3.trackMs[cfg::TRACK_RIDE] = 40000;
    r.run(3000);
    r.press();
    r.run(6000);
    REQUIRE(r.game.phase() == PHASE_RIDE);
    uint32_t silentMs = 0;
    while (r.game.phase() == PHASE_RIDE) {
        r.run(100);
        if (r.mp3.audible() != cfg::TRACK_RIDE) silentMs += 100;
    }
    // 4 track ends during the ride, each silent for at most ~2.5 s.
    CHECK(silentMs <= 4 * (cfg::RIDE_REPLAY_IDLE_MS + 500) + 1000);
    int plays = 0;
    for (auto& c : r.mp3.log) plays += c == "play 2";
    CHECK(plays >= 4);
}

TEST_CASE("BUSY not wired: timeouts drive everything, ride music not restarted in a loop") {
    Rig r;
    r.mp3.busyWired = false;
    r.run(3000);
    r.press();
    r.run(cfg::COUNTDOWN_TIMEOUT_MS + 100);
    CHECK(r.game.phase() == PHASE_RIDE);
    r.run(cfg::RIDE_MS);
    CHECK(r.game.phase() == PHASE_FINALE);
    r.run(cfg::APPLAUSE_TIMEOUT_MS + 200);
    r.run(cfg::FINALE_MS);
    CHECK(r.game.phase() == PHASE_IDLE);
    CHECK(r.mp3.log == std::vector<std::string>({
        "vol 20", "play 1", "play 2", "repeat on",
        "repeat off", "play 3", "play 4", "stop"}));
    CHECK_FALSE(r.audio.busyWorks());
}

TEST_CASE("winner starts on timeout if the applause never ends") {
    Rig r;
    r.mp3.trackMs[cfg::TRACK_APPLAUSE] = 600000;
    r.run(3000);
    r.game.requestPhase(PHASE_FINALE);
    r.run(500);
    const size_t mark = r.mp3.log.size();
    r.run(cfg::APPLAUSE_TIMEOUT_MS - 1000);
    CHECK(r.since(mark).empty());
    r.run(1500);
    CHECK(r.since(mark) == std::vector<std::string>({"play 4"}));
}

TEST_CASE("forced phases (bench) leave no stale repeat") {
    Rig r;
    r.run(3000);
    r.game.requestPhase(PHASE_RIDE);
    r.run(3000);
    r.game.requestPhase(PHASE_IDLE);
    r.run(1000);
    CHECK(r.mp3.log.back() == "stop");
    CHECK(r.mp3.log[r.mp3.log.size() - 2] == "repeat off");
    CHECK(r.mp3.audible() == 0);
}

TEST_CASE("playNow plays a single track for the bench") {
    Rig r;
    r.run(3000);
    r.audio.playNow(3);
    r.run(500);
    CHECK(r.mp3.log.back() == "play 3");
    CHECK(r.mp3.audible() == 3);
}

TEST_CASE("Audio state is small") {
    CHECK(sizeof(Audio) <= 32);
}

}  // TEST_SUITE
