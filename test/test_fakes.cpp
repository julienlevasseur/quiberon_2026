#include "doctest.h"
#include "fakes/fakes.h"

TEST_CASE("RecordingSink keeps each write as a separate frame") {
    fake::RecordingSink sink;
    const uint8_t a[] = {1, 2, 3};
    const uint8_t b[] = {9};
    sink.write(a, 3);
    sink.write(b, 1);
    REQUIRE(sink.frames.size() == 2);
    CHECK(sink.frames[0] == std::vector<uint8_t>({1, 2, 3}));
    CHECK(sink.frames[1] == std::vector<uint8_t>({9}));
}

TEST_CASE("RecordingDisplay tracks numbers and blanks") {
    fake::RecordingDisplay display;
    CHECK(display.current() == -2);
    display.showNumber(42);
    display.blank();
    CHECK(display.history == std::vector<int>({42, -1}));
    CHECK(display.current() == -1);
}

TEST_CASE("Rgb has the same layout as FastLED's CRGB") {
    CHECK(sizeof(qb::Rgb) == 3);
}
