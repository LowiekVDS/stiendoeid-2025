#pragma once

#include "string.h"

namespace config {

constexpr auto kRadioRxPin = 4;
constexpr auto kRadioSpeed = 6000;
constexpr auto kRadioPayloadSize = 4;
constexpr auto kRadioTxRate = 5.0;

const String kCompressedSequenceFileLocation = "/sequence.bin";

constexpr auto kNumLeds = 30;
constexpr auto kNumChannelsPerLed = 3;
constexpr auto kDataPin = 15;

constexpr auto kUpdateFrequency = 40.0;

constexpr auto kAllowedFrameDifference = 3;

} // namespace config