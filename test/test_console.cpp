#include <string>

#include "doctest.h"
#include "core/config.h"
#include "core/console.h"
#include "fakes/sim_rig.h"

using namespace qb;

namespace {

class StringOut : public TextOut {
public:
    std::string all;
    void text(const char* s) override { all += s; }
    void number(int32_t v) override { all += std::to_string(v); }
    void newline() override { all += "\n"; }
    std::string take() { std::string s = all; all.clear(); return s; }
};

// The rig with the console between the "serial port" and the App, wired
// exactly as in the sketch's loop().
struct Bench {
    fake::Rig rig;
    StringOut out;
    Console console{rig.app, rig.display, out};
    uint16_t realAdc = Bench::adcRest();
    bool overridden = false;

    static uint16_t adcRest() { return cfg::ADC_REST; }

    void type(const std::string& line) {
        for (char c : line) console.feed(c);
        console.feed('\n');
    }
    void run(uint32_t ms) {
        const uint32_t end = rig.now + ms;
        while (static_cast<int32_t>(end - rig.now) > 0) {
            rig.now += rig.loopMs;
            rig.mp3.now = rig.now;
            Inputs in = {rig.now, realAdc, false, rig.mp3.busy()};
            console.apply(in);
            if (rig.app.step(in)) overridden = console.overridePixels(rig.pixels.data(), cfg::NUM_LEDS);
            console.after(in);
        }
    }
};

}  // namespace

TEST_SUITE("console") {

TEST_CASE("help and unknown commands") {
    Bench b;
    b.type("help");
    CHECK(b.out.all.find("speed <0-255>") != std::string::npos);
    b.out.take();
    b.type("fly");
    CHECK(b.out.take() == "? unknown, try help\n");
    b.type("");
    CHECK(b.out.take().empty());
}

TEST_CASE("commands are case-insensitive and CRLF is fine") {
    Bench b;
    for (char c : std::string("STATUS\r\n")) b.console.feed(c);
    CHECK(b.out.take().rfind("phase idle", 0) == 0);
}

TEST_CASE("status reports phase, generator, calories and MP3") {
    Bench b;
    b.realAdc = 512;
    b.run(1000);
    b.type("status");
    const std::string s = b.out.take();
    CHECK(s.find("phase idle") != std::string::npos);
    CHECK(s.find("adc 512") != std::string::npos);
    CHECK(s.find("kcal 0") != std::string::npos);
    CHECK(s.find("busyok 0") != std::string::npos);
}

TEST_CASE("start presses the button: a game begins") {
    Bench b;
    b.run(1000);
    b.type("start");
    CHECK(b.out.take() == "ok\n");
    b.run(500);
    CHECK(b.rig.phase() == PHASE_COUNTDOWN);
}

TEST_CASE("speed injects a pedalling level, speed off gives the generator back") {
    Bench b;
    b.run(1000);
    b.type("speed 200");
    b.run(3000);
    CHECK(b.rig.app.game().speedLevel() >= 195);
    b.type("status");
    CHECK(b.out.take().find("[speed injected]") != std::string::npos);
    b.type("speed off");
    b.run(3000);
    CHECK(b.rig.app.game().speedLevel() <= 2);
    b.type("speed 300");
    CHECK(b.out.take().find("? speed") != std::string::npos);
}

TEST_CASE("phase jumps, with music and display following") {
    Bench b;
    b.run(3000);
    b.type("phase ride");
    b.run(1000);
    CHECK(b.rig.phase() == PHASE_RIDE);
    CHECK(b.rig.mp3.audible() == cfg::TRACK_RIDE);
    b.type("phase finale");
    b.run(1000);
    CHECK(b.rig.phase() == PHASE_FINALE);
    CHECK(b.rig.mp3.audible() == cfg::TRACK_APPLAUSE);
    b.type("phase idle");
    b.run(1000);
    CHECK(b.rig.phase() == PHASE_IDLE);
    CHECK(b.rig.mp3.audible() == 0);
    b.out.take();
    b.type("phase party");
    CHECK(b.out.take().find("? phase") != std::string::npos);
}

TEST_CASE("play a track, display a number") {
    Bench b;
    b.run(3000);
    b.type("play 4");
    b.run(500);
    CHECK(b.rig.mp3.audible() == 4);
    b.type("display 88");
    CHECK(b.rig.display.current() == 88);
    b.type("display blank");
    CHECK(b.rig.display.current() == -1);
    b.out.take();
    b.type("play 0");
    CHECK(b.out.take().find("? play") != std::string::npos);
}

TEST_CASE("led test patterns override the effects, auto gives them back") {
    Bench b;
    b.run(500);
    b.type("leds red");
    b.run(100);
    CHECK(b.overridden);
    for (auto& p : b.rig.pixels) {
        REQUIRE(p.r == cfg::MAX_BRIGHTNESS);
        REQUIRE(p.g == 0);
        REQUIRE(p.b == 0);
    }
    b.type("leds off");
    b.run(100);
    CHECK(b.rig.maxChannel() == 0);
    b.type("leds auto");
    b.run(100);
    CHECK_FALSE(b.overridden);
    CHECK(b.rig.lit() > 100);  // idle scrolling again
}

TEST_CASE("watch prints status every 500 ms until toggled off") {
    Bench b;
    b.type("watch");
    b.out.take();
    b.run(2600);
    std::string s = b.out.take();
    size_t lines = 0;
    for (char c : s) lines += c == '\n';
    CHECK(lines >= 5);
    CHECK(lines <= 6);
    b.type("watch");
    b.out.take();
    b.run(2000);
    CHECK(b.out.all.empty());
}

TEST_CASE("an over-long line is rejected, the next one works") {
    Bench b;
    b.type(std::string(60, 'x'));
    CHECK(b.out.take() == "? line too long\n");
    b.type("status");
    CHECK(b.out.take().rfind("phase", 0) == 0);
}

}  // TEST_SUITE
