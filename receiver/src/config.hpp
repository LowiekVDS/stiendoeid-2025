#pragma once

#include "string.h"

namespace config {

constexpr auto kRadioRxPin = 4;
constexpr auto kRadioSpeed = 6000;
constexpr auto kRadioPayloadSize = 4;
constexpr auto kRadioTxRate = 5.0;

const String kCompressedSequenceFileLocation = "/sequence.bin";

constexpr auto kNumChannelsPerLed = 3;
constexpr int kNumLeds[6] = {30, 30, 30, 30, 30, 30};
constexpr auto kTotalNumLeds = 180;
constexpr int kDataPin_1 = 15;
constexpr int kDataPin_2 = 15;
constexpr int kDataPin_3 = 15;
constexpr int kDataPin_4 = 15;
constexpr int kDataPin_5 = 15;
constexpr int kDataPin_6 = 15;

constexpr auto kUpdateFrequency = 40.0;

constexpr auto kAllowedFrameDifference = 3;

} // namespace config