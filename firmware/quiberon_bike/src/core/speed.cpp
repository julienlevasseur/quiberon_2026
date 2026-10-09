#include "speed.h"

#include "config.h"

namespace qb {

namespace {
// Longest step integrated at once; longer gaps are split so the discrete
// filter stays accurate (and positive) for any loop period.
const uint16_t MAX_STEP_MS = 64;
}  // namespace

uint8_t SpeedSensor::rawLevel(uint16_t adc) {
    if (adc <= cfg::ADC_REST) return 0;
    if (adc >= cfg::ADC_MAX) return 255;
    const uint32_t span = cfg::ADC_MAX - cfg::ADC_REST;
    return static_cast<uint8_t>(
        (static_cast<uint32_t>(adc - cfg::ADC_REST) * 255u + span / 2) / span);
}

uint8_t SpeedSensor::update(uint16_t adc, uint16_t dtMs) {
    const uint16_t target = static_cast<uint16_t>(rawLevel(adc) << 8);
    const uint32_t tau = cfg::SPEED_TAU_MS;
    if (tau == 0 || dtMs >= 8u * tau) {  // fully settled (e^-8 < 1/2000)
        state_ = target;
        return level();
    }
    while (dtMs > 0 && state_ != target) {
        const uint16_t dt = dtMs > MAX_STEP_MS ? MAX_STEP_MS : dtMs;
        dtMs = static_cast<uint16_t>(dtMs - dt);
        // Bilinear (Tustin) discretisation of d/dt s = (target - s) / tau:
        // gain 2dt / (2tau + dt) matches exp(-dt/tau) closely for dt <= tau.
        const int32_t diff = static_cast<int32_t>(target) - static_cast<int32_t>(state_);
        const int32_t num = 2 * static_cast<int32_t>(dt);
        const int32_t den = static_cast<int32_t>(2 * tau + dt);
        int32_t step = (diff * num + (diff > 0 ? den / 2 : -den / 2)) / den;
        if (step == 0) step = diff > 0 ? 1 : -1;  // never stall short of the target
        state_ = static_cast<uint16_t>(static_cast<int32_t>(state_) + step);
    }
    return level();
}

}  // namespace qb
