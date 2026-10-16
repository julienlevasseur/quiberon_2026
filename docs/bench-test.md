# Bench test procedure

*Version française : [bench-test.fr.md](bench-test.fr.md).*

For the colleague testing the installation on the bench. Every feature of the
game has already been tested on a PC (`make test`). This procedure checks what
a PC cannot: the real wiring, the real modules and the real cyclist. Tick each
step, and send back the **report** at the end (section 9).

Wiring reference: [wiring.md](wiring.md). SD card: [sd-card.md](sd-card.md).

## 0. What you need

- The Uno with the firmware, a USB cable and a laptop.
- Either `arduino-cli` (`make setup` then `make upload`), or the Arduino IDE
  with the **FastLED 3.10.6** library: open `firmware/quiberon_bike/quiberon_bike.ino`,
  board "Arduino Uno", upload.
- A serial monitor at **115200 baud**, line ending **"Newline"** (Arduino IDE)
  or `make monitor`.
- A multimeter. For the first steps without the bike: a potentiometer
  (10 kΩ between 5 V and GND, wiper on A1) can stand in for the generator.
- The SD card prepared as in [sd-card.md](sd-card.md).

## 1. Boot

| # | Do | Expect | ✓ |
|---|---|---|---|
| 1.1 | Power the 12 V supply, then plug the USB | Serial shows `quiberon_bike ready, free RAM N`, then `type help …`, then `display blank`. **Note N** (expected ≥ 450) | |
| 1.2 | Type `help` | The list of commands | |
| 1.3 | Wait 3 s, type `status` | `phase idle … busy 0 busyok 0` | |
| 1.4 | Look at the garlands | Slow red/green/gold comets climbing from the bottom, dim, on **both** garlands identically | |

The serial console commands are listed in [the console header](../firmware/quiberon_bike/src/core/console.h).

## 2. Garlands

| # | Do | Expect | ✓ |
|---|---|---|---|
| 2.1 | `leds red` | All 600 LEDs **red** (not green: that would mean the colour order is wrong) | |
| 2.2 | `leds green`, `leds blue` | All green, then all blue | |
| 2.3 | `leds white` | All white, same tint from the first to the last LED (a yellow/red end means the 12 V injection is too weak). Measure the supply current: **note it** | |
| 2.4 | `leds off` | All off, no flickering LED, especially the first one of each garland (if it flickers: BIN to GND, see wiring.md) | |
| 2.5 | `leds auto` | Back to the slow idle comets | |

## 3. Sound

| # | Do | Expect | ✓ |
|---|---|---|---|
| 3.1 | `play 1` | Countdown "Attention au départ 5-4-3-2-1-0". **If nothing is heard, stop here and go to [3.7](#37-if-play-stays-silent)**: the rest of section 3 means nothing without sound | |
| 3.2 | `status` while it plays | `busy 1 busyok 1` | |
| 3.3 | `status` after it ended | `busy 0`. **Only if the track was really heard in 3.1**: if busy is 1 when silent and 0 when playing, the BUSY polarity is reversed, report it (one line to change). With no sound at all, `busy 0` is normal (the module is idle) and `busy 1` would mean BUSY is not wired: do not change the polarity | |
| 3.4 | `play 2`, `play 3`, `play 4` | Waka Waka, then applause, then the winner music (if the file exists) | |
| 3.5 | `phase idle` | Sound stops | |
| 3.6 | Listen to the level at the visitors' distance | Volume comfortable (set in `config.h`, `VOLUME`, 0–30) | |

### 3.7 If `play` stays silent

BUSY tells whether the module plays: LOW (`busy 1`) while playing, HIGH
(`busy 0`) when idle. Silent with `busy 0` means the module never started.
Find out which side is at fault, in this order:

| # | Do | Expect | ✓ |
|---|---|---|---|
| 3.7.1 | Module powered, Arduino left as is: touch the DFPlayer pin `IO2` (pin 11) to GND for a short moment | The module plays a track on its own (no serial link involved), BUSY (pin 16) goes LOW. **If yes: the module and the card are fine, go to 3.7.3. If no: 3.7.2** | |
| 3.7.2 | Check the card and power: FAT32, 32 GB or less, folder `mp3` at the root with `0001_….mp3`… ([SD card](sd-card.md)); 5 V on VCC (pin 1); module GND joined to Arduino GND; speaker on `SPK_1` / `SPK_2`. Then try another card, then another module | 3.7.1 now plays | |
| 3.7.3 | Check the link: D5 → 1 kΩ → DFPlayer **RX (pin 2)**, not TX (pin 3); 2.2 kΩ from RX to GND; multimeter RX–GND at rest | ≈ 3.4 V on RX; then `play 1` plays | |
| 3.7.4 | If an oscilloscope or logic analyser is at hand: probe RX during `play 1` | One 10-byte frame at 9600 baud: `7E FF 06 12 00 00 01 FE E8 EF` | |

Report which step failed (and what was seen at 3.7.4, if done).

## 4. Start button

| # | Do | Expect | ✓ |
|---|---|---|---|
| 4.1 | Press the button once | `phase COUNTDOWN …`, `display 0`, the countdown plays, the garland fills from the bottom in 5 steps (red → green) | |
| 4.2 | Press again during the countdown | Nothing changes | |
| 4.3 | Countdown audio ends | `phase RIDE …` at that moment (± 0.5 s), the Waka Waka music starts | |
| 4.4 | `phase idle` | Back to idle, silence | |

## 5. Generator calibration

Do this with the real bike and generator. **First, with the multimeter, check
that A1 never goes above 5 V at the hardest sprint.** Above 5 V the Arduino
can be damaged: fix the signal adapter first.

| # | Do | Expect / note | ✓ |
|---|---|---|---|
| 5.1 | Type `watch` (prints the status twice a second) | Lines with `adc` and `level` | |
| 5.2 | Bike at rest | **Note the adc value** (expected: close to 0) | |
| 5.3 | Normal, relaxed pedalling | **Note adc**, and the voltage at A1 | |
| 5.4 | Hard sprint (a fit adult) | **Note adc**, and the voltage at A1 | |
| 5.5 | Type `watch` again to stop | | |

Then in `config.h`: `ADC_REST` = rest value + a small margin (about +10),
`ADC_MAX` = the sprint value. Re-upload and check that `level` is near 0 at
rest and near 255 at the sprint. If you do not have time, just send the
numbers.

## 6. Ride effect (no cyclist needed)

| # | Do | Expect | ✓ |
|---|---|---|---|
| 6.1 | `phase ride`, then `speed 0` | Violet dots breathing slowly, a short blue section at the bottom with moving comets. Not darker than idle | |
| 6.2 | `speed 64`, `speed 128`, `speed 200`, `speed 255` | The lit section grows up the tree, comets get faster and brighter, the colour goes from blue to red, white glitter appears above 128; at 255 the whole tree is lit | |
| 6.3 | `speed off`, then pedal | The same thing follows the real pedalling, smoothly (no flicker when the speed is constant) | |
| 6.4 | `phase finale` | Fireworks: rockets climbing, white flash, coloured sparks falling apart and fading. Applause, then the winner music | |
| 6.5 | `phase idle` | Back to idle | |

## 7. Full games (with real cyclists)

Use a stopwatch.

| # | Do | Expect | ✓ |
|---|---|---|---|
| 7.1 | Press start, pedal normally | Countdown, then the ride; the display counts up while pedalling, stops when you stop | |
| 7.2 | During the 3 minutes | The music never stops for more than ~2 s (it loops); the garland follows the speed | |
| 7.3 | At 3:00 after the end of the countdown | `phase FINALE`, fireworks, applause then winner music; **the display freezes** even if you keep pedalling | |
| 7.4 | 30 s later | `phase IDLE`, silence, idle comets; the display **keeps the last score** | |
| 7.5 | Each `phase …` line ends with `stack headroom N` | **Note the smallest N** over all games (must stay above 100) | |
| 7.6 | A second game right after | The display goes back to 0 at the countdown, and everything plays again | |
| 7.7 | Calories | A relaxed 3 minutes ≈ 10–20 kcal, a hard one ≈ 30–50 (the display caps at 99). Note the scores: the calorie model may need tuning | |

## 8. Robustness

| # | Do | Expect | ✓ |
|---|---|---|---|
| 8.1 | Unplug the BUSY wire (D3), play a game | The game still runs: the countdown lasts 10 s, the applause 12 s before the winner music | |
| 8.2 | Power off, hold the button, power on, keep holding 5 s | No game starts until the button is released and pressed again | |
| 8.3 | Leave it running in idle for 1 hour, then play a game | Everything still works | |
| 8.4 | Unplug the USB (power from the 12 V side only, if wired so) | Works the same; the serial console is only for the bench | |

## 9. Report to send back

```
free RAM at boot (1.1):            ____
supply current, all white (2.3):   ____ A
BUSY polarity correct (3.3):       yes / no / not checked (no sound)
silent play, failing step (3.7):   ____
volume OK (3.6):                   yes / no, preferred: ____
adc at rest / normal / sprint:     ____ / ____ / ____
voltage A1 at sprint:              ____ V
ride music loops without gap:      yes / no (clone ignores repeat?)
smallest stack headroom (7.5):     ____
scores of the test riders (7.7):   ____
anything odd:
```

The display hardware is not chosen yet: until then, its value is printed on
the serial line (`display N`).
