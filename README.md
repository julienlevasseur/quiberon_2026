# Vélo du sapin — Saint-Pierre-Quiberon 2026

Arduino Uno firmware for the Christmas bike game: pedalling drives two 300-LED
WS2815 garlands and a calorie display; a start button runs a countdown, a
3-minute ride with music, then a 30-second fireworks + applause finale.

- Request: [bootstrap.md](bootstrap.md) · wiring: `schéma de raccordement.pdf` · examples: [references/](references/)
- Project board: http://127.0.0.1:7777/b/quiberon-2026 (agentBoard)
- Project brief: http://127.0.0.1:7777/c/quiberon-2026-bike-tree-project-brief

## Layout

| Path | What |
|---|---|
| `firmware/quiberon_bike/quiberon_bike.ino` | the sketch: wires the core to the hardware |
| `firmware/quiberon_bike/src/core/` | the game logic, plain C++11, **no Arduino dependency**, testable on a PC |
| `firmware/quiberon_bike/src/core/config.h` | every tunable: pins, durations, calibration, track numbers |
| `firmware/quiberon_bike/src/hal/` | thin Arduino adapters (pins, LEDs, MP3 serial, display) |
| `test/` | host tests (doctest), fakes for the hardware interfaces |
| `tools/` | memory budget gate (`check_size.py`), visual simulator (`sim/`) |
| `docs/` | wiring, SD card layout, bench test procedure |

## Build and test

Requirements: `g++`, `cmake`, `python3`, and [arduino-cli](https://arduino.github.io/arduino-cli/) on `PATH`.

```bash
make setup      # once: installs the Uno core and FastLED (pinned versions)
make test       # host tests (unit + full-game scenarios), no hardware needed
make sim        # HTML replays of simulated games: open build/sim/index.html
make firmware   # compiles for the Uno; fails if static RAM > 1536 B or flash > 95%
make upload PORT=/dev/ttyACM0
make monitor    # serial log / bench console at 115200 baud
```

The Uno has 2048 bytes of RAM and the LED buffer alone takes 900, so
`make firmware` is a real test: keep it green.

## Documentation

- [docs/wiring.md](docs/wiring.md): pins, the two garlands on D6, power, generator input
- [docs/sd-card.md](docs/sd-card.md): how to name the sound files
- [docs/bench-test.md](docs/bench-test.md): bench procedure and serial console commands

A complete guide to the project, in French (hardware, wiring, software, tests,
rebuilding it from scratch), with drawings: [docs/guide/](docs/guide/README.md).

Each page has a French version next to it (`docs/*.fr.md`) for the French-speaking
collaborator: [câblage](docs/wiring.fr.md), [carte SD](docs/sd-card.fr.md),
[test au banc](docs/bench-test.fr.md). Update both versions together.
