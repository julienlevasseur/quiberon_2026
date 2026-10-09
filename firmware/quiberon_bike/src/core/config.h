// Every tunable value of the game lives here, so the bench can adjust the
// behaviour without touching the logic. Plain C++11, no Arduino dependency.
#pragma once

#include <stdint.h>

namespace qb {
namespace cfg {

// ---------------------------------------------------------------- pins (Uno)
const uint8_t PIN_START_BUTTON = 2;   // INPUT_PULLUP, pressed = LOW
const uint8_t PIN_MP3_BUSY     = 3;   // DFPlayer BUSY, LOW while a track plays
const uint8_t PIN_MP3_TX       = 5;   // TX-only serial (hal/tx_serial) -> 1 kOhm -> DFPlayer RX
const uint8_t PIN_LEDS         = 6;   // both garlands wired in parallel
const uint8_t PIN_GENERATOR    = 1;   // A1, through the signal adapter (0-5 V)

// ---------------------------------------------------------------- timings (ms)
const uint32_t COUNTDOWN_TIMEOUT_MS = 10000UL;  // fallback if BUSY never reports the end
const uint32_t RIDE_MS              = 180000UL; // 3 minutes of pedalling
const uint32_t FINALE_MS            = 30000UL;  // fireworks + applause + winner
const uint16_t BUTTON_DEBOUNCE_MS   = 30;
const uint16_t BUSY_SETTLE_MS       = 300;      // BUSY lags a few 100 ms after a play command
const uint16_t FRAME_MS             = 20;       // LED refresh period (50 fps)
const uint16_t DISPLAY_REFRESH_MS   = 200;      // at most 5 display updates per second

// ---------------------------------------------------------------- garland
const uint16_t NUM_LEDS        = 300;  // per garland; both show the same image
const uint8_t  MAX_BRIGHTNESS  = 180;  // global cap (power budget)
const uint8_t  IDLE_BRIGHTNESS = 60;
const uint16_t IDLE_STEP_MS    = 40;   // idle scrolling: one LED every 40 ms

// Effects (src/core/lights.cpp). LED 0 is the bottom of the garland.
const uint16_t COUNTDOWN_STEP_MS     = 1000; // one more fifth of the garland per step
const uint8_t  COUNTDOWN_SEGMENTS    = 5;    // "5-4-3-2-1": 5 fill steps, then green flashes
const uint8_t  RIDE_MIN_BRIGHTNESS   = 96;   // comet brightness at level 0
const uint8_t  RIDE_BASE_SPACING     = 3;    // breathing base glow: one LED in 3,
const uint8_t  RIDE_BASE_MIN         = 84;   //   brightness range; keeps the ride at
const uint8_t  RIDE_BASE_MAX         = 116;  //   least as bright as idle at every level
const uint8_t  RIDE_BASE_HUE         = 200;  // violet, never a comet hue (160 -> 0)
const uint16_t RIDE_MIN_LEN          = 24;   // LEDs lit at level 0
const uint8_t  RIDE_MIN_SPEED        = 12;   // comet speed at level 0, LEDs per second
const uint8_t  RIDE_MAX_SPEED        = 90;   // comet speed at level 255, LEDs per second
const uint8_t  RIDE_GLITTER_LEVEL    = 128;  // above this level, white glitter appears
const uint16_t FW_LAUNCH_MIN_MS      = 400;  // gap between two rocket launches (random)
const uint16_t FW_LAUNCH_MAX_MS      = 1100;
const uint32_t FW_LAUNCH_WINDOW_MS   = FINALE_MS - 4000UL; // last launch, so sparks end in time
const uint8_t  FW_ROCKET_MIN_SPEED   = 140;  // LEDs per second
const uint8_t  FW_ROCKET_MAX_SPEED   = 230;
const uint16_t FW_BURST_MS           = 1500; // life of the sparks of one burst
const uint8_t  FW_BURST_MIN_RADIUS   = 12;   // LEDs, reach of the fastest spark
const uint8_t  FW_BURST_MAX_RADIUS   = 36;
const uint8_t  FW_FADE_PER_20MS      = 56;   // trail fade: keep (256-56)/256 every 20 ms

// ---------------------------------------------------------------- generator / speed
// Placeholder calibration: to be measured on the bench (board item q-generator-range).
const uint16_t ADC_REST       = 20;    // raw ADC at rest (noise floor)
const uint16_t ADC_MAX        = 920;   // raw ADC at a hard sprint (~4.5 V)
const uint8_t  ADC_SAMPLES    = 8;     // averaged per reading
const uint16_t SPEED_TAU_MS   = 300;   // smoothing time constant of the speed level

// ---------------------------------------------------------------- calories
// Speed level (0..255) -> mechanical power (W), linear interpolation between points.
const uint8_t POWER_POINTS = 5;
const uint8_t  POWER_LEVEL[POWER_POINTS] = {0, 64, 128, 192, 255};
const uint16_t POWER_WATTS[POWER_POINTS] = {0, 60, 130, 220, 320};
// Human metabolic efficiency, in percent: kcal = J / (4184 * 0.24)
const uint8_t EFFICIENCY_PERCENT = 24;
const uint16_t DISPLAY_MAX = 99;       // clamp for a 2-digit display
const bool IDLE_SHOWS_LAST_SCORE = true; // IDLE: keep the last score (blank before the first game)

// ---------------------------------------------------------------- sound
// SD card: /mp3/000N_<name>.mp3 (see docs/sd-card.md)
const uint16_t TRACK_COUNTDOWN = 1;    // 0001_AttentionAuDepart543210.mp3
const uint16_t TRACK_RIDE      = 2;    // 0002_WakaWakaShakira.mp3 (looped)
const uint16_t TRACK_APPLAUSE  = 3;    // 0003_BravoApplaudissements.mp3
const uint16_t TRACK_WINNER    = 4;    // 0004_<winner>.mp3, file to come
const uint8_t  VOLUME          = 20;   // 0..30
const uint32_t APPLAUSE_TIMEOUT_MS = 12000UL; // start the winner music even if BUSY is silent
const uint16_t MP3_BOOT_MS     = 2000;  // the module ignores commands for 1.5-3 s after power-up
const uint16_t MP3_CMD_GAP_MS  = 50;    // min time between two commands (module needs ~10 ms)
const uint16_t RIDE_REPLAY_IDLE_MS = 2000; // ride music silent this long (BUSY) -> play it again

}  // namespace cfg
}  // namespace qb
