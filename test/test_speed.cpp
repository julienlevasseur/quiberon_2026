#include "doctest.h"

#include "core/config.h"
#include "core/speed.h"

#include <cmath>

namespace {

using qb::cfg::ADC_MAX;
using qb::cfg::ADC_REST;
using qb::cfg::SPEED_TAU_MS;

// Runs the filter for durationMs with ticks of stepMs at a constant ADC value.
void run(qb::SpeedSensor& s, uint16_t adc, uint32_t durationMs, uint16_t stepMs) {
    for (uint32_t t = 0; t < durationMs; t += stepMs) s.update(adc, stepMs);
}

uint16_t adcForLevel(uint8_t level) {
    return static_cast<uint16_t>(ADC_REST + (static_cast<uint32_t>(ADC_MAX - ADC_REST) * level) / 255);
}

}  // namespace

TEST_SUITE("speed") {

TEST_CASE("raw mapping: rest is 0, max voltage is 255, linear in between") {
    CHECK(qb::SpeedSensor::rawLevel(0) == 0);
    CHECK(qb::SpeedSensor::rawLevel(ADC_REST) == 0);
    CHECK(qb::SpeedSensor::rawLevel(ADC_MAX) == 255);
    CHECK(qb::SpeedSensor::rawLevel(1023) == 255);
    CHECK(qb::SpeedSensor::rawLevel((ADC_REST + ADC_MAX) / 2) == doctest::Approx(128).epsilon(0.01));
    uint8_t prev = 0;
    for (uint16_t adc = 0; adc <= 1023; ++adc) {
        const uint8_t l = qb::SpeedSensor::rawLevel(adc);
        CHECK(l >= prev);
        prev = l;
    }
}

TEST_CASE("noise below the floor is ignored") {
    qb::SpeedSensor s;
    for (int i = 0; i < 1000; ++i) s.update(static_cast<uint16_t>(i % (ADC_REST + 1)), 5);
    CHECK(s.level() == 0);
}

TEST_CASE("steady input settles to the mapped level") {
    qb::SpeedSensor s;
    run(s, ADC_MAX, 10 * SPEED_TAU_MS, 20);
    CHECK(s.level() == 255);
    run(s, ADC_REST, 10 * SPEED_TAU_MS, 20);
    CHECK(s.level() == 0);
    run(s, adcForLevel(100), 10 * SPEED_TAU_MS, 1);
    CHECK(s.level() == doctest::Approx(100).epsilon(0.01));
}

TEST_CASE("step response follows the time constant") {
    qb::SpeedSensor s;
    run(s, ADC_MAX, SPEED_TAU_MS, 10);
    // 1 - e^-1 = 63.2 % after one tau.
    CHECK(s.level() == doctest::Approx(255 * (1 - std::exp(-1.0))).epsilon(0.03));
    run(s, ADC_MAX, 4 * SPEED_TAU_MS, 10);  // 5 tau in total: within 1 %
    CHECK(s.level() >= 252);
}

TEST_CASE("step response is the same whatever the loop period") {
    const uint16_t steps[] = {1, 5, 20, 50, 100, 300};
    const uint32_t checkpoints[] = {100, 300, 600, 900, 1500};
    for (unsigned c = 0; c < sizeof checkpoints / sizeof checkpoints[0]; ++c) {
        const double expected = 255 * (1 - std::exp(-double(checkpoints[c]) / SPEED_TAU_MS));
        for (unsigned i = 0; i < sizeof steps / sizeof steps[0]; ++i) {
            if (checkpoints[c] % steps[i]) continue;
            qb::SpeedSensor s;
            run(s, ADC_MAX, checkpoints[c], steps[i]);
            CAPTURE(checkpoints[c]);
            CAPTURE(steps[i]);
            CHECK(std::fabs(s.level() - expected) <= 4.0);
        }
    }
}

TEST_CASE("decay is symmetric and also loop-rate independent") {
    qb::SpeedSensor a, b;
    run(a, ADC_MAX, 3000, 10);
    run(b, ADC_MAX, 3000, 50);
    run(a, ADC_REST, SPEED_TAU_MS, 1);
    run(b, ADC_REST, SPEED_TAU_MS, 50);
    const double expected = 255 * std::exp(-1.0);
    CHECK(std::fabs(a.level() - expected) <= 4.0);
    CHECK(std::fabs(b.level() - expected) <= 4.0);
}

TEST_CASE("a long gap jumps to the target, a zero dt changes nothing") {
    qb::SpeedSensor s;
    s.update(ADC_MAX, 0);
    CHECK(s.level() == 0);
    s.update(ADC_MAX, 65535);
    CHECK(s.level() == 255);
    s.update(ADC_REST, 8 * SPEED_TAU_MS);
    CHECK(s.level() == 0);
}

TEST_CASE("reset goes back to 0") {
    qb::SpeedSensor s;
    run(s, ADC_MAX, 2000, 20);
    s.reset();
    CHECK(s.level() == 0);
}

}  // TEST_SUITE
