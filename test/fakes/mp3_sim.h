// A simulated DFPlayer: decodes the frames the firmware sends, keeps a
// readable command log, and drives a realistic BUSY line (start latency,
// track length, single-track repeat). Clone quirks can be switched on.
#pragma once

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "core/hal.h"

namespace fake {

class Mp3Sim : public qb::ByteSink {
public:
    uint32_t now = 0;                     // set by the test before each step
    uint32_t latencyMs = 200;             // play command -> BUSY
    std::map<int, uint32_t> trackMs;      // track length; default below
    uint32_t defaultTrackMs = 60000;
    bool ignoresRepeat = false;           // clone quirk
    bool busyWired = true;                // false: BUSY never goes active

    std::vector<std::string> log;         // "vol 20", "play 2", "stop", "repeat on"...
    std::vector<uint32_t> logMs;          // when each command arrived
    int badFrames = 0;

    void write(const uint8_t* d, uint8_t len) override {
        if (len != 10 || d[0] != 0x7E || d[1] != 0xFF || d[2] != 0x06 || d[9] != 0xEF) {
            ++badFrames;
            return;
        }
        uint16_t sum = 0;
        for (int i = 1; i <= 6; ++i) sum = static_cast<uint16_t>(sum + d[i]);
        if (static_cast<uint16_t>(sum + ((d[7] << 8) | d[8])) != 0) {
            ++badFrames;
            return;
        }
        const int param = (d[5] << 8) | d[6];
        std::string entry;
        switch (d[3]) {
            case 0x12: entry = "play " + std::to_string(param); track_ = param; startMs_ = now; playing_ = true; break;
            case 0x16: entry = "stop"; playing_ = false; break;
            case 0x06: entry = "vol " + std::to_string(param); break;
            case 0x19: entry = param == 0 ? "repeat on" : "repeat off"; repeat_ = param == 0 && !ignoresRepeat; break;
            case 0x0C: entry = "reset"; playing_ = false; break;
            default:   entry = "cmd " + std::to_string(d[3]); break;
        }
        log.push_back(entry);
        logMs.push_back(now);
    }

    uint32_t lengthOf(int track) const {
        auto it = trackMs.find(track);
        return it == trackMs.end() ? defaultTrackMs : it->second;
    }

    // What the BUSY pin reports right now (true = playing).
    bool busy() const {
        if (!busyWired || !playing_) return false;
        const uint32_t t = now - startMs_;
        if (t < latencyMs) return false;
        return repeat_ || t < latencyMs + lengthOf(track_);
    }

    // The track audible right now, 0 if none.
    int audible() const { return busy() ? track_ : 0; }

private:
    int track_ = 0;
    uint32_t startMs_ = 0;
    bool playing_ = false;
    bool repeat_ = false;
};

}  // namespace fake
