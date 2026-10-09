// Test doubles for the hardware interfaces of src/core/hal.h.
#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "core/hal.h"

namespace fake {

// Records every write() as one frame.
class RecordingSink : public qb::ByteSink {
public:
    std::vector<std::vector<uint8_t> > frames;
    void write(const uint8_t* data, uint8_t len) override {
        frames.push_back(std::vector<uint8_t>(data, data + len));
    }
};

// Records every display update; -1 stands for blank.
class RecordingDisplay : public qb::NumberDisplay {
public:
    std::vector<int> history;
    void showNumber(uint16_t value) override { history.push_back(value); }
    void blank() override { history.push_back(-1); }
    int current() const { return history.empty() ? -2 : history.back(); }
};

}  // namespace fake
