#include "doctest.h"

#include "core/button.h"
#include "core/config.h"

namespace {

// Feeds a level for durationMs, ticking every stepMs; returns the number of press events.
int feed(qb::Button& b, uint32_t& now, bool down, uint32_t durationMs, uint32_t stepMs = 1) {
    int events = 0;
    for (uint32_t t = 0; t < durationMs; t += stepMs) {
        if (b.update(down, now)) ++events;
        now += stepMs;
    }
    return events;
}

}  // namespace

TEST_SUITE("button") {

TEST_CASE("clean press gives one event after the debounce delay") {
    qb::Button b;
    uint32_t now = 1000;
    CHECK(feed(b, now, false, 100) == 0);
    CHECK_FALSE(b.isDown());
    // No event before the level has been stable for BUTTON_DEBOUNCE_MS.
    CHECK(feed(b, now, true, qb::cfg::BUTTON_DEBOUNCE_MS) == 0);
    CHECK(b.update(true, now) == true);
    CHECK(b.isDown());
    CHECK(feed(b, now, true, 100) == 0);
    CHECK(feed(b, now, false, 100) == 0);  // release is not an event
    CHECK_FALSE(b.isDown());
}

TEST_CASE("bounce noise on press and release gives a single event") {
    qb::Button b;
    uint32_t now = 0;
    int events = feed(b, now, false, 50);
    // Contact bounce: toggles every 1..3 ms for 15 ms.
    const uint8_t bounce[] = {1, 2, 1, 3, 1, 1, 2, 1, 3};
    bool level = true;
    for (unsigned i = 0; i < sizeof bounce; ++i) {
        events += feed(b, now, level, bounce[i]);
        level = !level;
    }
    events += feed(b, now, true, 500);
    CHECK(events == 1);
    // Release bounce must not create a new press.
    level = false;
    for (unsigned i = 0; i < sizeof bounce; ++i) {
        events += feed(b, now, level, bounce[i]);
        level = !level;
    }
    events += feed(b, now, false, 500);
    CHECK(events == 1);
}

TEST_CASE("held button gives one event") {
    qb::Button b;
    uint32_t now = 0;
    feed(b, now, false, 10);
    CHECK(feed(b, now, true, 60000, 7) == 1);
}

TEST_CASE("quick double press within the debounce window gives one event") {
    qb::Button b;
    uint32_t now = 0;
    int events = feed(b, now, false, 50);
    events += feed(b, now, true, 40);
    events += feed(b, now, false, qb::cfg::BUTTON_DEBOUNCE_MS - 5);  // too short to count
    events += feed(b, now, true, 200);
    CHECK(events == 1);
}

TEST_CASE("two real presses give two events") {
    qb::Button b;
    uint32_t now = 0;
    int events = feed(b, now, false, 50);
    events += feed(b, now, true, 100);
    events += feed(b, now, false, 100);
    events += feed(b, now, true, 100);
    CHECK(events == 2);
}

TEST_CASE("press shorter than the debounce is ignored") {
    qb::Button b;
    uint32_t now = 0;
    int events = feed(b, now, false, 50);
    events += feed(b, now, true, qb::cfg::BUTTON_DEBOUNCE_MS - 1);
    events += feed(b, now, false, 200);
    CHECK(events == 0);
}

TEST_CASE("button held at boot does not fire until released and pressed again") {
    qb::Button b;
    uint32_t now = 0;
    CHECK(feed(b, now, true, 1000) == 0);
    CHECK(b.isDown());
    CHECK(feed(b, now, false, 100) == 0);
    CHECK(feed(b, now, true, 100) == 1);
}

TEST_CASE("debounce works across the millis() wrap-around") {
    qb::Button b;
    uint32_t now = 0xFFFFFFFFUL - 100;
    int events = feed(b, now, false, 90);
    events += feed(b, now, true, 100);  // crosses 0xFFFFFFFF -> 0
    CHECK(events == 1);
    CHECK(now < 1000);
}

TEST_CASE("slow loop still debounces") {
    qb::Button b;
    uint32_t now = 0;
    int events = feed(b, now, false, 100, 25);
    events += feed(b, now, true, 200, 25);
    CHECK(events == 1);
}

}  // TEST_SUITE
