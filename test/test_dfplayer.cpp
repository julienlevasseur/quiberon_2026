#include "doctest.h"
#include "fakes/fakes.h"

#include "core/dfplayer.h"

typedef std::vector<uint8_t> Bytes;

// Frames computed with DFRobotDFPlayerMini::calculateCheckSum (0 - sum of
// bytes 1..6), feedback byte forced to 0.
TEST_CASE("DFPlayer frames are byte-exact") {
    fake::RecordingSink sink;
    qb::DFPlayer mp3(sink);

    mp3.setVolume(20);
    mp3.play(1);
    mp3.play(2);
    mp3.stop();
    mp3.setRepeatCurrent(true);
    mp3.setRepeatCurrent(false);
    mp3.reset();

    REQUIRE(sink.frames.size() == 7);
    CHECK(sink.frames[0] == Bytes({0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, 0x14, 0xFE, 0xE1, 0xEF}));
    CHECK(sink.frames[1] == Bytes({0x7E, 0xFF, 0x06, 0x12, 0x00, 0x00, 0x01, 0xFE, 0xE8, 0xEF}));
    CHECK(sink.frames[2] == Bytes({0x7E, 0xFF, 0x06, 0x12, 0x00, 0x00, 0x02, 0xFE, 0xE7, 0xEF}));
    CHECK(sink.frames[3] == Bytes({0x7E, 0xFF, 0x06, 0x16, 0x00, 0x00, 0x00, 0xFE, 0xE5, 0xEF}));
    CHECK(sink.frames[4] == Bytes({0x7E, 0xFF, 0x06, 0x19, 0x00, 0x00, 0x00, 0xFE, 0xE2, 0xEF}));
    CHECK(sink.frames[5] == Bytes({0x7E, 0xFF, 0x06, 0x19, 0x00, 0x00, 0x01, 0xFE, 0xE1, 0xEF}));
    CHECK(sink.frames[6] == Bytes({0x7E, 0xFF, 0x06, 0x0C, 0x00, 0x00, 0x00, 0xFE, 0xEF, 0xEF}));
}

TEST_CASE("DFPlayer volume is clamped to 30") {
    fake::RecordingSink sink;
    qb::DFPlayer mp3(sink);
    mp3.setVolume(30);
    mp3.setVolume(31);
    mp3.setVolume(255);
    mp3.setVolume(0);
    REQUIRE(sink.frames.size() == 4);
    CHECK(sink.frames[0][6] == 30);
    CHECK(sink.frames[1] == sink.frames[0]);
    CHECK(sink.frames[2] == sink.frames[0]);
    CHECK(sink.frames[3] == Bytes({0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, 0x00, 0xFE, 0xF5, 0xEF}));
}

TEST_CASE("DFPlayer track numbers above 255 use the high byte") {
    fake::RecordingSink sink;
    qb::DFPlayer mp3(sink);
    mp3.play(1000);  // 0x03E8, datasheet 3.4.1 example 3
    mp3.play(256);
    REQUIRE(sink.frames.size() == 2);
    CHECK(sink.frames[0] == Bytes({0x7E, 0xFF, 0x06, 0x12, 0x00, 0x03, 0xE8, 0xFD, 0xFE, 0xEF}));
    CHECK(sink.frames[1] == Bytes({0x7E, 0xFF, 0x06, 0x12, 0x00, 0x01, 0x00, 0xFE, 0xE8, 0xEF}));
}

TEST_CASE("DFPlayer checksum: bytes 1..8 sum to zero modulo 2^16") {
    // Independent of encode(): rebuild the sum from the frame itself.
    const uint8_t cmds[] = {0x06, 0x0C, 0x12, 0x16, 0x19};
    const uint16_t params[] = {0, 1, 15, 30, 255, 256, 1000, 2999, 0xFFFF};
    for (uint8_t c = 0; c < sizeof(cmds); ++c) {
        for (uint8_t p = 0; p < sizeof(params) / sizeof(params[0]); ++p) {
            uint8_t f[qb::DFPlayer::FRAME_LEN];
            qb::DFPlayer::encode(cmds[c], params[p], f);
            CAPTURE(int(cmds[c]));
            CAPTURE(params[p]);
            CHECK(f[0] == 0x7E);
            CHECK(f[1] == 0xFF);
            CHECK(f[2] == 0x06);
            CHECK(f[3] == cmds[c]);
            CHECK(f[4] == 0x00);
            CHECK(((f[5] << 8) | f[6]) == params[p]);
            CHECK(f[9] == 0xEF);
            uint32_t sum = 0;
            for (int i = 1; i <= 6; ++i) sum += f[i];
            const uint32_t chk = (uint32_t(f[7]) << 8) | f[8];
            CHECK(((sum + chk) & 0xFFFF) == 0);
        }
    }
}

TEST_CASE("DFPlayer BUSY pin is active low") {
    CHECK(qb::DFPlayer::playingFromBusyPin(false));
    CHECK_FALSE(qb::DFPlayer::playingFromBusyPin(true));
}
