// Bench console on the USB serial (115200 baud), for testing the installation
// piece by piece without playing whole games. One command per line:
//
//   help                     list the commands
//   status                   one line: phase, times, generator, calories, MP3
//   watch                    status every 500 ms (again to stop): calibration
//   start                    press the start button
//   phase idle|countdown|ride|finale   jump to a phase
//   speed <0-255> | speed off          pretend the cyclist pedals at a level
//   play <n>                 play track n from /mp3 (until the next phase change)
//   display <n> | display blank        write the calorie display directly
//   leds red|green|blue|white|off|auto test patterns on the whole garland
//
// The text goes through TextOut; on the Uno the strings stay in flash (QB_P),
// since RAM is the scarce resource. Plain C++11.
#pragma once

#include <stdint.h>

#include "app.h"
#include "hal.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#define QB_P(s) PSTR(s)   // string kept in flash, TextOut reads it from there
#else
#define QB_P(s) (s)
#endif

namespace qb {

class TextOut {
public:
    virtual void text(const char* qbP) = 0;  // a QB_P() string
    virtual void number(int32_t value) = 0;
    virtual void newline() = 0;
protected:
    ~TextOut() {}
};

class Console {
public:
    Console(App& app, NumberDisplay& display, TextOut& out);

    // Each character received on the serial line.
    void feed(char c);
    // Before App::step(): injected button press and speed.
    void apply(Inputs& in);
    // After App::step(): periodic watch output.
    void after(const Inputs& in);
    // After a rendered frame: overrides the pixels with a test pattern.
    // Returns true when a pattern is active.
    bool overridePixels(Rgb* pixels, uint16_t count) const;

    static const uint8_t LINE_MAX = 24;

private:
    enum Leds { LEDS_AUTO = 0, LEDS_RED, LEDS_GREEN, LEDS_BLUE, LEDS_WHITE, LEDS_OFF };

    void execute();
    void status();
    void help();
    void ok() { out_.text(QB_P("ok")); out_.newline(); }

    App&           app_;
    NumberDisplay& display_;
    TextOut&       out_;
    uint32_t       nowMs_;
    uint32_t       pressUntilMs_;
    uint32_t       lastWatchMs_;
    uint16_t       rawAdc_;
    char           line_[LINE_MAX];
    uint8_t        len_;
    uint8_t        leds_;
    uint8_t        speed_;      // injected level
    uint8_t        flags_;
    bool           busy_;

    enum { SPEED_ON = 1, PRESSING = 2, WATCH = 4, OVERFLOW = 8 };
};

}  // namespace qb
