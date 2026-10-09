// Arduino side of src/core/hal.h: samples the pins into qb::Inputs, and a
// stand-in for the calorie display until its hardware is chosen.
#pragma once

#ifdef ARDUINO

#include <Arduino.h>

#include "../core/config.h"
#include "../core/console.h"
#include "../core/dfplayer.h"
#include "../core/hal.h"

extern int __heap_start, *__brkval;  // avr-libc heap bounds, for freeRam()
extern uint8_t _end;                 // end of .bss, start of the heap

namespace qb {

inline void beginInputs() {
    pinMode(cfg::PIN_START_BUTTON, INPUT_PULLUP);
    pinMode(cfg::PIN_MP3_BUSY, INPUT_PULLUP);  // idle (not playing) if unwired
}

inline Inputs readInputs() {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < cfg::ADC_SAMPLES; ++i) sum += analogRead(A0 + cfg::PIN_GENERATOR);
    Inputs in;
    in.nowMs = millis();
    in.generatorAdc = sum / cfg::ADC_SAMPLES;
    in.buttonDown = digitalRead(cfg::PIN_START_BUTTON) == LOW;
    in.mp3Busy = DFPlayer::playingFromBusyPin(digitalRead(cfg::PIN_MP3_BUSY) == HIGH);
    return in;
}

// Prints the value on the USB serial until the real display driver exists
// (board item display-driver).
class SerialDisplay : public NumberDisplay {
public:
    void showNumber(uint16_t value) override {
        Serial.print(F("display "));
        Serial.println(value);
    }
    void blank() override { Serial.println(F("display blank")); }
};

// Console output on the USB serial; the console's strings are in flash.
class SerialOut : public TextOut {
public:
    void text(const char* qbP) override { Serial.print(reinterpret_cast<const __FlashStringHelper*>(qbP)); }
    void number(int32_t value) override { Serial.print(value); }
    void newline() override { Serial.println(); }
};

// Stack high-water mark: paintStack() fills the free RAM with a pattern at
// boot; stackHeadroom() counts the bytes the stack has never touched since.
// Call paintStack() first thing in setup().
const uint8_t STACK_PAINT = 0xC5;

inline void paintStack() {
    // Above the heap, should a library ever malloc() before setup().
    uint8_t* p = __brkval ? reinterpret_cast<uint8_t*>(__brkval) : &_end;
    const uint8_t* top = reinterpret_cast<const uint8_t*>(SP) - 32;  // keep clear of this frame
    while (p < top) *p++ = STACK_PAINT;
}

inline uint16_t stackHeadroom() {
    const uint8_t* p = __brkval ? reinterpret_cast<const uint8_t*>(__brkval) : &_end;
    uint16_t n = 0;
    while (*p == STACK_PAINT && p < reinterpret_cast<const uint8_t*>(SP)) {
        ++p;
        ++n;
    }
    return n;
}

// Bytes between the heap and the stack: what is left for the stack.
inline int freeRam() {
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

}  // namespace qb

#endif  // ARDUINO
