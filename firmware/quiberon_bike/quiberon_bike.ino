// Jeu de vélo du sapin de Noël 2026 de Saint-Pierre-Quiberon.
// Arduino Uno. See README.md and docs/.
//
// The game logic lives in src/core (tested on a PC, see test/); this file only
// samples the pins, steps the core once per loop and pushes the pixels to the
// LEDs. Nothing here may block: the loop must keep running for the button,
// the timers and the sound.

#include <FastLED.h>

#include "src/core/app.h"
#include "src/core/config.h"
#include "src/core/console.h"
#include "src/core/dfplayer.h"
#include "src/core/hal.h"
#include "src/hal/arduino_io.h"
#include "src/hal/tx_serial.h"

using namespace qb;

static_assert(sizeof(CRGB) == sizeof(Rgb), "core pixels must alias FastLED pixels");

CRGB leds[cfg::NUM_LEDS];  // both garlands, wired in parallel on PIN_LEDS

TxSerial      mp3Tx;
DFPlayer      mp3(mp3Tx);
SerialDisplay display;
App           app(mp3, display, reinterpret_cast<Rgb*>(leds), cfg::NUM_LEDS);
SerialOut     serialOut;
Console       console(app, display, serialOut);  // bench commands, see docs/bench-test.md

static void logPhase(const Game& game) {
    Serial.print(F("phase "));
    switch (game.phase()) {  // F() keeps the strings in flash: RAM is the scarce resource
        case PHASE_IDLE:      Serial.print(F("IDLE")); break;
        case PHASE_COUNTDOWN: Serial.print(F("COUNTDOWN")); break;
        case PHASE_RIDE:      Serial.print(F("RIDE")); break;
        case PHASE_FINALE:    Serial.print(F("FINALE")); break;
    }
    Serial.print(F(" kcal "));
    Serial.print(game.kcal());
    Serial.print(F(" stack headroom "));  // bench: must stay well above 100
    Serial.println(stackHeadroom());
}

void setup() {
    paintStack();
    Serial.begin(115200);
    Serial.print(F("quiberon_bike ready, free RAM "));
    Serial.println(freeRam());
    Serial.println(F("type help for the bench commands"));
    beginInputs();
    mp3Tx.begin(cfg::PIN_MP3_TX);
    // The effects apply cfg::MAX_BRIGHTNESS themselves: keep FastLED at 255.
    FastLED.addLeds<WS2812B, cfg::PIN_LEDS, GRB>(leds, cfg::NUM_LEDS);
    FastLED.clear(true);
}

void loop() {
    while (Serial.available() > 0) console.feed(static_cast<char>(Serial.read()));
    Inputs in = readInputs();
    console.apply(in);
    const bool frame = app.step(in);
    if (app.game().changed()) logPhase(app.game());
    if (frame) {
        console.overridePixels(reinterpret_cast<Rgb*>(leds), cfg::NUM_LEDS);
        FastLED.show();  // ~9 ms with interrupts off for 300 LEDs
    }
    console.after(in);
}
