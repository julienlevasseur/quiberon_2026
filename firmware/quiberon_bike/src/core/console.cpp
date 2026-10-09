#include "console.h"

#include "config.h"

namespace qb {

namespace {

const uint16_t PRESS_MS = 150;  // longer than the button debounce
const uint16_t WATCH_MS = 500;

// Compares a word of the line with a literal (plain RAM string on the host,
// flash string on the Uno).
bool is(const char* word, const char* qbP) {
#ifdef __AVR__
    return strcmp_P(word, qbP) == 0;
#else
    while (*word && *word == *qbP) { ++word; ++qbP; }
    return *word == *qbP;
#endif
}

// Parses a decimal number; false if empty, not a number or above max.
bool parse(const char* s, uint16_t max, uint16_t& out) {
    if (!*s) return false;
    uint32_t v = 0;
    for (; *s; ++s) {
        if (*s < '0' || *s > '9') return false;
        v = v * 10 + static_cast<uint32_t>(*s - '0');
        if (v > max) return false;
    }
    out = static_cast<uint16_t>(v);
    return true;
}

}  // namespace

Console::Console(App& app, NumberDisplay& display, TextOut& out)
    : app_(app), display_(display), out_(out), nowMs_(0), pressUntilMs_(0), lastWatchMs_(0),
      rawAdc_(0), len_(0), leds_(LEDS_AUTO), speed_(0), flags_(0), busy_(false) {
    line_[0] = 0;
}

void Console::feed(char c) {
    if (c == '\r' || c == '\n') {
        if (flags_ & OVERFLOW) {
            out_.text(QB_P("? line too long"));
            out_.newline();
        } else if (len_ > 0) {
            line_[len_] = 0;
            execute();
        }
        len_ = 0;
        flags_ = static_cast<uint8_t>(flags_ & ~OVERFLOW);
        return;
    }
    if (len_ >= LINE_MAX - 1) {
        flags_ |= OVERFLOW;
        return;
    }
    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    line_[len_++] = c;
}

void Console::apply(Inputs& in) {
    nowMs_ = in.nowMs;
    rawAdc_ = in.generatorAdc;
    busy_ = in.mp3Busy;
    if (flags_ & PRESSING) {
        if (static_cast<int32_t>(pressUntilMs_ - in.nowMs) > 0) {
            in.buttonDown = true;
        } else {
            flags_ = static_cast<uint8_t>(flags_ & ~PRESSING);
        }
    }
    if (flags_ & SPEED_ON) {
        in.generatorAdc = static_cast<uint16_t>(cfg::ADC_REST +
            static_cast<uint32_t>(speed_) * (cfg::ADC_MAX - cfg::ADC_REST) / 255);
    }
}

void Console::after(const Inputs& in) {
    if ((flags_ & WATCH) && in.nowMs - lastWatchMs_ >= WATCH_MS) {
        lastWatchMs_ = in.nowMs;
        status();
    }
}

bool Console::overridePixels(Rgb* pixels, uint16_t count) const {
    if (leds_ == LEDS_AUTO) return false;
    const uint8_t v = cfg::MAX_BRIGHTNESS;
    Rgb c = {0, 0, 0};
    switch (leds_) {
        case LEDS_RED:   c.r = v; break;
        case LEDS_GREEN: c.g = v; break;
        case LEDS_BLUE:  c.b = v; break;
        case LEDS_WHITE: c.r = c.g = c.b = v; break;
        default: break;
    }
    for (uint16_t i = 0; i < count; ++i) pixels[i] = c;
    return true;
}

void Console::status() {
    const Game& g = app_.game();
    out_.text(QB_P("phase "));
    switch (g.phase()) {  // no table: it would live in RAM on the Uno
        case PHASE_IDLE:      out_.text(QB_P("idle")); break;
        case PHASE_COUNTDOWN: out_.text(QB_P("countdown")); break;
        case PHASE_RIDE:      out_.text(QB_P("ride")); break;
        case PHASE_FINALE:    out_.text(QB_P("finale")); break;
    }
    out_.text(QB_P(" t "));
    out_.number(static_cast<int32_t>(g.elapsedMs() / 1000));
    out_.text(QB_P(" left "));
    out_.number(static_cast<int32_t>(g.remainingMs() / 1000));
    out_.text(QB_P(" adc "));
    out_.number(rawAdc_);
    out_.text(QB_P(" level "));
    out_.number(g.speedLevel());
    out_.text(QB_P(" kcal "));
    out_.number(g.kcal());
    out_.text(QB_P(" busy "));
    out_.number(busy_ ? 1 : 0);
    out_.text(QB_P(" busyok "));
    out_.number(app_.audio().busyWorks() ? 1 : 0);
    if (flags_ & SPEED_ON) out_.text(QB_P(" [speed injected]"));
    if (leds_ != LEDS_AUTO) out_.text(QB_P(" [led test]"));
    out_.newline();
}

void Console::help() {
    out_.text(QB_P("help status watch start | phase idle|countdown|ride|finale | speed <0-255>|off"));
    out_.newline();
    out_.text(QB_P("play <n> | display <n>|blank | leds red|green|blue|white|off|auto"));
    out_.newline();
}

void Console::execute() {
    // Split "word arg" in place.
    char* arg = line_;
    while (*arg && *arg != ' ') ++arg;
    if (*arg) *arg++ = 0;
    while (*arg == ' ') ++arg;
    char* end = arg;
    while (*end && *end != ' ') ++end;
    *end = 0;

    const char* cmd = line_;
    uint16_t n = 0;
    if (is(cmd, QB_P("help"))) {
        help();
    } else if (is(cmd, QB_P("status"))) {
        status();
    } else if (is(cmd, QB_P("watch"))) {
        flags_ ^= WATCH;
        ok();
    } else if (is(cmd, QB_P("start"))) {
        flags_ |= PRESSING;
        pressUntilMs_ = nowMs_ + PRESS_MS;
        ok();
    } else if (is(cmd, QB_P("phase"))) {
        if (is(arg, QB_P("idle")))           app_.game().requestPhase(PHASE_IDLE);
        else if (is(arg, QB_P("countdown"))) app_.game().requestPhase(PHASE_COUNTDOWN);
        else if (is(arg, QB_P("ride")))      app_.game().requestPhase(PHASE_RIDE);
        else if (is(arg, QB_P("finale")))    app_.game().requestPhase(PHASE_FINALE);
        else { out_.text(QB_P("? phase idle|countdown|ride|finale")); out_.newline(); return; }
        ok();
    } else if (is(cmd, QB_P("speed"))) {
        if (is(arg, QB_P("off"))) {
            flags_ = static_cast<uint8_t>(flags_ & ~SPEED_ON);
        } else if (parse(arg, 255, n)) {
            speed_ = static_cast<uint8_t>(n);
            flags_ |= SPEED_ON;
        } else {
            out_.text(QB_P("? speed <0-255>|off"));
            out_.newline();
            return;
        }
        ok();
    } else if (is(cmd, QB_P("play"))) {
        if (!parse(arg, 255, n) || n == 0) { out_.text(QB_P("? play <1-255>")); out_.newline(); return; }
        app_.audio().playNow(static_cast<uint8_t>(n));
        ok();
    } else if (is(cmd, QB_P("display"))) {
        if (is(arg, QB_P("blank"))) display_.blank();
        else if (parse(arg, 9999, n)) display_.showNumber(n);
        else { out_.text(QB_P("? display <n>|blank")); out_.newline(); return; }
        ok();
    } else if (is(cmd, QB_P("leds"))) {
        if (is(arg, QB_P("auto")))       leds_ = LEDS_AUTO;
        else if (is(arg, QB_P("red")))   leds_ = LEDS_RED;
        else if (is(arg, QB_P("green"))) leds_ = LEDS_GREEN;
        else if (is(arg, QB_P("blue")))  leds_ = LEDS_BLUE;
        else if (is(arg, QB_P("white"))) leds_ = LEDS_WHITE;
        else if (is(arg, QB_P("off")))   leds_ = LEDS_OFF;
        else { out_.text(QB_P("? leds red|green|blue|white|off|auto")); out_.newline(); return; }
        ok();
    } else {
        out_.text(QB_P("? unknown, try help"));
        out_.newline();
    }
}

}  // namespace qb
