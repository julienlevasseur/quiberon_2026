#include "calories.h"

#include "config.h"

namespace qb {

namespace {
// Mechanical millijoules per burnt kcal: 4184 J x 1000 x EFFICIENCY_PERCENT / 100.
const uint32_t MJ_PER_KCAL = 4184UL * 10UL * cfg::EFFICIENCY_PERCENT;
}  // namespace

uint16_t Calories::watts(uint8_t level) {
    if (level <= cfg::POWER_LEVEL[0]) return cfg::POWER_WATTS[0];
    for (uint8_t i = 1; i < cfg::POWER_POINTS; ++i) {
        if (level <= cfg::POWER_LEVEL[i]) {
            const uint16_t l0 = cfg::POWER_LEVEL[i - 1];
            const uint16_t l1 = cfg::POWER_LEVEL[i];
            const int32_t w0 = cfg::POWER_WATTS[i - 1];
            const int32_t w1 = cfg::POWER_WATTS[i];
            return static_cast<uint16_t>(
                w0 + (w1 - w0) * static_cast<int32_t>(level - l0) / static_cast<int32_t>(l1 - l0));
        }
    }
    return cfg::POWER_WATTS[cfg::POWER_POINTS - 1];
}

void Calories::addEnergy(uint16_t w, uint16_t dtMs) {
    if (!running_) return;
    const uint32_t add = static_cast<uint32_t>(w) * dtMs;
    millijoules_ = (millijoules_ > 0xFFFFFFFFUL - add) ? 0xFFFFFFFFUL : millijoules_ + add;
}

uint16_t Calories::kcal() const {
    const uint32_t k = (millijoules_ / 2 + MJ_PER_KCAL / 4) / (MJ_PER_KCAL / 2);
    return k > 0xFFFFu ? 0xFFFFu : static_cast<uint16_t>(k);
}

}  // namespace qb
