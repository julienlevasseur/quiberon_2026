// Send-only driver for the DFPlayer Mini (MP3-TF-16P clone).
//
// Each command is one 10-byte frame written in a single ByteSink::write():
//   7E FF 06 CMD 00 PARAM_HI PARAM_LO CHK_HI CHK_LO EF
// CHK = 0 - (FF + 06 + CMD + 00 + PARAM_HI + PARAM_LO), 16 bits, as computed
// by DFRobot's own library (the checksums printed in the DFR0299 datasheet
// are inconsistent with each other, the library's are the ones that work).
// The feedback byte is 0: no ACK is requested since nothing listens.
//
// The module needs ~10 ms between two commands (DFRobot's library waits that
// long after each frame without ACK): send at most one command per loop().
// Plain C++11, no Arduino dependency, no state besides the sink.
#pragma once

#include <stdint.h>

#include "hal.h"

namespace qb {

class DFPlayer {
public:
    static const uint8_t FRAME_LEN  = 10;
    static const uint8_t MAX_VOLUME = 30;

    // Command bytes (DFR0299 datasheet 3.2, DFRobotDFPlayerMini.cpp).
    static const uint8_t CMD_VOLUME          = 0x06;
    static const uint8_t CMD_RESET           = 0x0C;
    static const uint8_t CMD_PLAY_MP3_FOLDER = 0x12;
    static const uint8_t CMD_STOP            = 0x16;
    static const uint8_t CMD_REPEAT_CURRENT  = 0x19;

    explicit DFPlayer(ByteSink& sink) : sink_(sink) {}

    // Plays /mp3/NNNN_*.mp3 (only the 4-digit prefix matters), 1..3000.
    void play(uint16_t track) { send(CMD_PLAY_MP3_FOLDER, track); }

    void stop() { send(CMD_STOP, 0); }

    // 0..30, clamped: the module ignores out-of-range values.
    void setVolume(uint8_t volume) {
        send(CMD_VOLUME, volume > MAX_VOLUME ? MAX_VOLUME : volume);
    }

    // Single-track repeat of the track being played (0x19, param 0 = on,
    // 1 = off; the inverse of what one would guess, see DFRobot enableLoop()).
    // How to loop the ride music: play(TRACK_RIDE), then once BUSY reports
    // playing, setRepeatCurrent(true); turn it off before the next track.
    // 0x08 "loop track N" is not used: its N is the file's global index on the
    // card (FAT copy order), not the /mp3 number, so it may loop the wrong file.
    // Clones differ, so the caller should also replay the track if BUSY goes
    // idle during the ride.
    void setRepeatCurrent(bool on) { send(CMD_REPEAT_CURRENT, on ? 0 : 1); }

    // Soft reset; the module then needs 1.5-3 s before accepting commands.
    void reset() { send(CMD_RESET, 0); }

    // BUSY (pin 16) is LOW while a track plays, HIGH when idle or asleep.
    // (The datasheet text says the opposite; the modules and DFRobot's wiki
    // agree on LOW = playing. To confirm on the bench.)
    static bool playingFromBusyPin(bool pinHigh) { return !pinHigh; }

    // Builds one frame; exposed for the tests.
    static void encode(uint8_t cmd, uint16_t param, uint8_t out[FRAME_LEN]);

private:
    void send(uint8_t cmd, uint16_t param);

    ByteSink& sink_;
};

}  // namespace qb
