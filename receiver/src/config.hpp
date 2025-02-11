#pragma once

#include "string.h"

#define MOCK_RADIO false

namespace config {

constexpr auto kRadioRxPin = 4;
constexpr auto kRadioSpeed = 6000;
constexpr auto kRadioPayloadSize = 4;
constexpr auto kRadioTxRate = 5.0;
constexpr auto kRadioRxStatusLedPin = 3;

const String kCompressedSequenceFileLocation = "/sequence.bin";

constexpr auto kNumChannelsPerLed = 3;
constexpr int kNumLeds[6] = {50, 50, 50, 50, 50, 50};
constexpr auto kTotalNumLeds = 300;
constexpr int kDataPin_1 = 6;
constexpr int kDataPin_2 = 7;
constexpr int kDataPin_3 = 15;
constexpr int kDataPin_4 = 16;
constexpr int kDataPin_5 = 17;
constexpr int kDataPin_6 = 18;

constexpr auto kBrightness = 255;

constexpr auto kUpdateFrequency = 40.0;

constexpr auto kAllowedFrameDifference = 3;

} // namespace config