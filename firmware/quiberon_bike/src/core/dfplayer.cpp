#include "dfplayer.h"

namespace qb {

// Out-of-class definitions: C++11 needs them if a constant is ever odr-used.
const uint8_t DFPlayer::FRAME_LEN;
const uint8_t DFPlayer::MAX_VOLUME;
const uint8_t DFPlayer::CMD_VOLUME;
const uint8_t DFPlayer::CMD_RESET;
const uint8_t DFPlayer::CMD_PLAY_MP3_FOLDER;
const uint8_t DFPlayer::CMD_STOP;
const uint8_t DFPlayer::CMD_REPEAT_CURRENT;

void DFPlayer::encode(uint8_t cmd, uint16_t param, uint8_t out[FRAME_LEN]) {
    out[0] = 0x7E;  // start
    out[1] = 0xFF;  // version
    out[2] = 0x06;  // length: version..param
    out[3] = cmd;
    out[4] = 0x00;  // feedback: no ACK
    out[5] = static_cast<uint8_t>(param >> 8);
    out[6] = static_cast<uint8_t>(param);
    uint16_t sum = 0;
    for (uint8_t i = 1; i < 7; ++i) sum = static_cast<uint16_t>(sum + out[i]);
    const uint16_t chk = static_cast<uint16_t>(0u - sum);
    out[7] = static_cast<uint8_t>(chk >> 8);
    out[8] = static_cast<uint8_t>(chk);
    out[9] = 0xEF;  // end
}

void DFPlayer::send(uint8_t cmd, uint16_t param) {
    // On the stack, not a member: 10 bytes of RAM only while sending.
    uint8_t frame[FRAME_LEN];
    encode(cmd, param, frame);
    sink_.write(frame, FRAME_LEN);
}

}  // namespace qb
