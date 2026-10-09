// What the speaker plays in each phase (brief section 4):
//   IDLE       silence
//   COUNTDOWN  the countdown track
//   RIDE       the ride music, looped
//   FINALE     the applause, then the winner music
//
// The DFPlayer is driven send-only, so everything we know about it comes from
// its BUSY line. Commands are queued and sent one per update(), at least
// cfg::MP3_CMD_GAP_MS apart, and not before cfg::MP3_BOOT_MS after power-up.
//
// BUSY may be badly wired or a clone may not drive it, so every decision that
// relies on it has a fallback:
// - the winner music starts when the applause ends, or after
//   cfg::APPLAUSE_TIMEOUT_MS;
// - the ride music is looped with the repeat command, and also played again if
//   BUSY stays idle for cfg::RIDE_REPLAY_IDLE_MS (clones that ignore repeat),
//   but only once BUSY has been seen working since boot, otherwise a missing
//   BUSY wire would restart the music every two seconds.
// Plain C++11.
#pragma once

#include <stdint.h>

#include "dfplayer.h"
#include "game.h"

namespace qb {

class Audio {
public:
    explicit Audio(DFPlayer& player);

    // Once per loop, right after Game::tick(), with the same nowMs.
    // Sends at most one command.
    void update(const Game& game, bool mp3Busy, uint32_t nowMs);

    // For the bench console: play one track now (bypasses the phase logic
    // until the next phase change).
    void playNow(uint8_t track);

    // True once BUSY has been seen high since boot (bench diagnostic).
    bool busyWorks() const { return (flags_ & BUSY_WORKS) != 0; }

private:
    enum Op { OP_PLAY = 0, OP_STOP, OP_VOLUME, OP_REPEAT_ON, OP_REPEAT_OFF };
    enum {
        STARTED = 1, SENT_ANY = 2, BUSY_WORKS = 4, SAW_BUSY = 8,
        REPEAT_ON = 16, RIDE_REPEAT_PENDING = 32, WINNER_PENDING = 64, VOLUME_PENDING = 128,
    };
    static const uint8_t QUEUE_LEN = 3;

    void push(uint8_t op, uint8_t arg);
    void enterPhase(Phase p);
    void send(uint8_t op, uint8_t arg, uint32_t nowMs);

    DFPlayer& player_;
    uint32_t  lastCmdMs_;   // last command sent, or boot time
    uint32_t  playMs_;      // last play command sent
    uint32_t  lastBusyMs_;  // last time BUSY was high (or play sent)
    uint8_t   queue_[QUEUE_LEN][2];
    uint8_t   queueLen_;
    uint8_t   flags_;
    uint8_t   track_;       // last track played
};

}  // namespace qb
