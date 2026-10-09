#include "doctest.h"

#include "core/calories.h"
#include "core/config.h"

namespace {

void ride(qb::Calories& c, uint16_t watts, uint32_t durationMs, uint16_t stepMs) {
    for (uint32_t t = 0; t < durationMs; t += stepMs) c.addEnergy(watts, stepMs);
}

}  // namespace

TEST_SUITE("calories") {

TEST_CASE("watts follow the calibration table") {
    using namespace qb::cfg;
    for (uint8_t i = 0; i < POWER_POINTS; ++i) CHECK(qb::Calories::watts(POWER_LEVEL[i]) == POWER_WATTS[i]);
    // Midpoint of the first segment.
    CHECK(qb::Calories::watts(32) == (POWER_WATTS[0] + POWER_WATTS[1]) / 2);
    uint16_t prev = 0;
    for (int l = 0; l <= 255; ++l) {
        const uint16_t w = qb::Calories::watts(static_cast<uint8_t>(l));
        CHECK(w >= prev);
        prev = w;
    }
}

TEST_CASE("0 W gives 0 kcal") {
    qb::Calories c;
    c.start();
    ride(c, 0, qb::cfg::RIDE_MS, 20);
    CHECK(c.millijoules() == 0);
    CHECK(c.kcal() == 0);
    for (uint32_t t = 0; t < qb::cfg::RIDE_MS; t += 20) c.update(0, 20);
    CHECK(c.kcal() == 0);
}

TEST_CASE("150 W for 180 s gives about 27 kcal") {
    qb::Calories c;
    c.start();
    ride(c, 150, 180000UL, 20);
    CHECK(c.millijoules() == 150UL * 180000UL);  // exact, no drift
    CHECK(c.kcal() >= 26);
    CHECK(c.kcal() <= 28);
    CHECK(c.kcal() == 27);  // 26.89 rounded
}

TEST_CASE("result does not depend on the loop rate") {
    const uint16_t steps[] = {1, 7, 20, 50, 1000};
    for (unsigned i = 0; i < sizeof steps / sizeof steps[0]; ++i) {
        qb::Calories c;
        c.start();
        ride(c, 220, 7000UL * steps[i], steps[i]);  // 7000 ticks each, durations differ
        CHECK(c.millijoules() == 220UL * 7000UL * steps[i]);
    }
    qb::Calories fast, slow;
    fast.start();
    slow.start();
    for (uint32_t t = 0; t < qb::cfg::RIDE_MS; ++t) fast.update(200, 1);
    for (uint32_t t = 0; t < qb::cfg::RIDE_MS; t += 50) slow.update(200, 50);
    CHECK(fast.millijoules() == slow.millijoules());
    CHECK(fast.kcal() == slow.kcal());
}

TEST_CASE("count is monotonic") {
    qb::Calories c;
    c.start();
    uint16_t prevK = 0;
    uint32_t prevMj = 0;
    for (uint32_t t = 0; t < qb::cfg::RIDE_MS; t += 20) {
        c.update(static_cast<uint8_t>((t / 20) % 256), 20);
        CHECK(c.millijoules() >= prevMj);
        CHECK(c.kcal() >= prevK);
        prevMj = c.millijoules();
        prevK = c.kcal();
    }
    CHECK(prevK > 0);
}

TEST_CASE("start, freeze and reset") {
    qb::Calories c;
    CHECK_FALSE(c.running());
    ride(c, 300, 10000, 20);  // not started: nothing counted
    CHECK(c.millijoules() == 0);
    c.start();
    ride(c, 300, 10000, 20);
    const uint32_t frozen = c.millijoules();
    CHECK(frozen == 3000000UL);
    c.freeze();
    CHECK_FALSE(c.running());
    ride(c, 300, 10000, 20);
    CHECK(c.millijoules() == frozen);
    c.start();  // a new ride starts from zero
    CHECK(c.millijoules() == 0);
    ride(c, 300, 1000, 20);
    c.reset();
    CHECK(c.millijoules() == 0);
    CHECK_FALSE(c.running());
}

TEST_CASE("max power for a whole ride stays well inside the accumulator") {
    qb::Calories c;
    c.start();
    for (uint32_t t = 0; t < qb::cfg::RIDE_MS; t += 20) c.update(255, 20);
    CHECK(c.millijoules() == 320UL * qb::cfg::RIDE_MS);
    CHECK(c.kcal() == 57);  // 57.6 kJ / 1004.16 J = 57.36
    c.addEnergy(0xFFFF, 0xFFFF);
    for (int i = 0; i < 2000; ++i) c.addEnergy(0xFFFF, 0xFFFF);  // saturates, never wraps
    CHECK(c.millijoules() == 0xFFFFFFFFUL);
}

}  // TEST_SUITE
