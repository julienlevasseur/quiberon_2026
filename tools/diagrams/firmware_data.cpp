// Runs pieces of the real firmware core on the PC and prints, as JSON, the data
// the documentation pictures are drawn from: LED frames of every effect, the
// speed filter's step response and the power curve of the calorie model.
// Built and run by tools/diagrams/build_guide.py.
#include <cstdio>
#include <vector>

#include "core/calories.h"
#include "core/config.h"
#include "core/lights.h"
#include "core/speed.h"

using namespace qb;

namespace {

std::vector<Rgb> buf(cfg::NUM_LEDS);

void dump(const char* name, bool& first) {
    std::printf("%s{\"name\":\"%s\",\"px\":\"", first ? "" : ",", name);
    for (const Rgb& p : buf) std::printf("%02x%02x%02x", p.r, p.g, p.b);
    std::printf("\"}");
    first = false;
}

// Renders `mode` from t=0 every frame until `at` (ms), at a constant level.
void frameAt(LightMode mode, uint32_t at, uint8_t level, const char* name, bool& first) {
    Lights lights(7);
    for (Rgb& p : buf) p = Rgb{0, 0, 0};
    lights.setMode(mode, 0);
    for (uint32_t t = 0; t <= at; t += cfg::FRAME_MS) lights.render(buf.data(), cfg::NUM_LEDS, t, level);
    dump(name, first);
}

uint16_t adcFor(uint8_t level) {
    return static_cast<uint16_t>(cfg::ADC_REST + static_cast<uint32_t>(level) * (cfg::ADC_MAX - cfg::ADC_REST) / 255);
}

}  // namespace

int main() {
    std::printf("{\"effects\":[");
    bool first = true;
    frameAt(LIGHT_IDLE, 2000, 0, "idle", first);
    const uint32_t cd[] = {500, 1500, 2500, 3500, 4500, 5250};
    const char* cdn[] = {"cd1", "cd2", "cd3", "cd4", "cd5", "cdgo"};
    for (int i = 0; i < 6; ++i) frameAt(LIGHT_COUNTDOWN, cd[i], 0, cdn[i], first);
    const uint8_t lv[] = {0, 64, 128, 192, 255};
    const char* lvn[] = {"ride0", "ride64", "ride128", "ride192", "ride255"};
    for (int i = 0; i < 5; ++i) frameAt(LIGHT_RIDE, 3000, lv[i], lvn[i], first);
    const uint32_t fw[] = {260, 700, 1500, 2600, 9000};
    const char* fwn[] = {"fw1", "fw2", "fw3", "fw4", "fw5"};
    for (int i = 0; i < 5; ++i) frameAt(LIGHT_FINALE, fw[i], 0, fwn[i], first);
    std::printf("],");

    // Speed filter: the cyclist jumps to level 200 at t=0, stops at t=1500.
    std::printf("\"filter\":[");
    SpeedSensor s;
    for (uint32_t t = 0; t <= 3000; t += 20) {
        const uint8_t target = t < 1500 ? 200 : 0;
        if (t > 0) s.update(adcFor(target), 20);
        std::printf("%s[%u,%u,%u]", t ? "," : "", t, target, s.level());
    }
    std::printf("],");

    std::printf("\"watts\":[");
    for (int l = 0; l <= 255; ++l) std::printf("%s%u", l ? "," : "", Calories::watts(static_cast<uint8_t>(l)));
    std::printf("],");

    // kcal after a full ride at a constant level.
    std::printf("\"kcal\":[");
    for (int l = 0; l <= 255; l += 5) {
        Calories c;
        c.start();
        for (uint32_t t = 0; t < cfg::RIDE_MS; t += 20) c.update(static_cast<uint8_t>(l), 20);
        std::printf("%s[%d,%u]", l ? "," : "", l, c.kcal());
    }
    std::printf("]}\n");
    return 0;
}
