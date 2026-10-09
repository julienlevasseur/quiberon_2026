#include "button.h"

#include "config.h"

namespace qb {

Button::Button() : flags_(0), changedMs_(0) {}

bool Button::update(bool down, uint32_t nowMs) {
    const uint16_t now16 = static_cast<uint16_t>(nowMs);
    if (!(flags_ & INIT)) {
        flags_ = static_cast<uint8_t>(INIT | (down ? (STABLE | RAW) : 0));
        changedMs_ = now16;
        return false;
    }
    const bool raw = (flags_ & RAW) != 0;
    if (down != raw) {
        // Any edge restarts the stability window.
        flags_ = static_cast<uint8_t>(down ? (flags_ | RAW) : (flags_ & ~RAW));
        changedMs_ = now16;
        return false;
    }
    if (down == isDown()) return false;
    if (static_cast<uint16_t>(now16 - changedMs_) < cfg::BUTTON_DEBOUNCE_MS) return false;
    flags_ = static_cast<uint8_t>(down ? (flags_ | STABLE) : (flags_ & ~STABLE));
    return down;
}

}  // namespace qb
