#include "audio.h"

#include "config.h"

namespace qb {

namespace {
// Fallback when BUSY never confirms the ride track started: send the repeat
// command anyway after this long.
const uint16_t REPEAT_FALLBACK_MS = 1000;
}

Audio::Audio(DFPlayer& player)
    : player_(player), lastCmdMs_(0), playMs_(0), lastBusyMs_(0), queueLen_(0), flags_(0), track_(0) {}

void Audio::push(uint8_t op, uint8_t arg) {
    if (queueLen_ >= QUEUE_LEN) return;  // cannot happen with the scripts below
    queue_[queueLen_][0] = op;
    queue_[queueLen_][1] = arg;
    ++queueLen_;
}

void Audio::playNow(uint8_t track) {
    queueLen_ = 0;
    flags_ = static_cast<uint8_t>(flags_ & ~(RIDE_REPEAT_PENDING | WINNER_PENDING));
    if (flags_ & REPEAT_ON) push(OP_REPEAT_OFF, 0);
    push(OP_PLAY, track);
}

void Audio::enterPhase(Phase p) {
    // A phase change replaces whatever was still waiting (the volume is not
    // in the queue, see update()).
    queueLen_ = 0;
    flags_ = static_cast<uint8_t>(flags_ & ~(RIDE_REPEAT_PENDING | WINNER_PENDING));
    if (p != PHASE_RIDE && (flags_ & REPEAT_ON)) push(OP_REPEAT_OFF, 0);
    switch (p) {
        case PHASE_IDLE:      push(OP_STOP, 0); break;
        case PHASE_COUNTDOWN: push(OP_PLAY, cfg::TRACK_COUNTDOWN); break;
        case PHASE_RIDE:      push(OP_PLAY, cfg::TRACK_RIDE); break;
        case PHASE_FINALE:
            push(OP_PLAY, cfg::TRACK_APPLAUSE);
            flags_ |= WINNER_PENDING;
            break;
    }
}

void Audio::send(uint8_t op, uint8_t arg, uint32_t nowMs) {
    switch (op) {
        case OP_PLAY:
            player_.play(arg);
            track_ = arg;
            playMs_ = nowMs;
            lastBusyMs_ = nowMs;
            flags_ = static_cast<uint8_t>(flags_ & ~SAW_BUSY);
            if (arg == cfg::TRACK_RIDE) flags_ |= RIDE_REPEAT_PENDING;
            break;
        case OP_STOP:       player_.stop(); break;
        case OP_VOLUME:     player_.setVolume(arg); break;
        case OP_REPEAT_ON:  player_.setRepeatCurrent(true);  flags_ |= REPEAT_ON; break;
        case OP_REPEAT_OFF: player_.setRepeatCurrent(false); flags_ = static_cast<uint8_t>(flags_ & ~REPEAT_ON); break;
    }
    lastCmdMs_ = nowMs;
    flags_ |= SENT_ANY;
}

void Audio::update(const Game& game, bool mp3Busy, uint32_t nowMs) {
    if (!(flags_ & STARTED)) {
        flags_ |= STARTED;
        lastCmdMs_ = nowMs;  // the boot delay counts from here
        flags_ |= VOLUME_PENDING;
    }

    if (game.changed()) enterPhase(game.phase());

    // What BUSY tells us about the track we asked for. Right after a play
    // command BUSY still reflects the previous track, hence the settle time.
    const bool settled = nowMs - playMs_ >= cfg::BUSY_SETTLE_MS;
    if (mp3Busy) {
        flags_ |= BUSY_WORKS;
        lastBusyMs_ = nowMs;
        if (settled) flags_ |= SAW_BUSY;
    }

    if (queueLen_ == 0) {
        const Phase phase = game.phase();
        if (phase == PHASE_RIDE) {
            if ((flags_ & RIDE_REPEAT_PENDING) &&
                ((flags_ & SAW_BUSY) || nowMs - playMs_ >= REPEAT_FALLBACK_MS)) {
                flags_ = static_cast<uint8_t>(flags_ & ~RIDE_REPEAT_PENDING);
                push(OP_REPEAT_ON, 0);
            } else if ((flags_ & BUSY_WORKS) && !mp3Busy && settled &&
                       nowMs - lastBusyMs_ >= cfg::RIDE_REPLAY_IDLE_MS) {
                push(OP_PLAY, cfg::TRACK_RIDE);  // the loop did not hold
            }
        } else if (phase == PHASE_FINALE && (flags_ & WINNER_PENDING)) {
            const bool applauseOver = (flags_ & SAW_BUSY) && !mp3Busy;
            if (applauseOver || nowMs - playMs_ >= cfg::APPLAUSE_TIMEOUT_MS) {
                flags_ = static_cast<uint8_t>(flags_ & ~WINNER_PENDING);
                push(OP_PLAY, cfg::TRACK_WINNER);
            }
        }
    }

    if (queueLen_ == 0 && !(flags_ & VOLUME_PENDING)) return;
    const uint16_t gap = (flags_ & SENT_ANY) ? cfg::MP3_CMD_GAP_MS : cfg::MP3_BOOT_MS;
    if (nowMs - lastCmdMs_ < gap) return;
    // The volume goes first and survives phase changes during the boot delay.
    if (flags_ & VOLUME_PENDING) {
        flags_ = static_cast<uint8_t>(flags_ & ~VOLUME_PENDING);
        send(OP_VOLUME, cfg::VOLUME, nowMs);
        return;
    }
    const uint8_t op = queue_[0][0];
    const uint8_t arg = queue_[0][1];
    for (uint8_t i = 1; i < queueLen_; ++i) {
        queue_[i - 1][0] = queue_[i][0];
        queue_[i - 1][1] = queue_[i][1];
    }
    --queueLen_;
    send(op, arg, nowMs);
}

}  // namespace qb
