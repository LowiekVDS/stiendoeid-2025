#pragma once

#include "string.h"

namespace config {

constexpr auto kRadioRxPin = 5;
constexpr auto kRadioSpeed = 6000;
constexpr auto kRadioPayloadSize = 4;
constexpr auto kRadioTxRate = 5.0;

const String kCompressedSequenceFileLocation = "/sequence.bin";

constexpr auto kNumLeds = 30;
constexpr auto kNumChannelsPerLed = 3;
constexpr auto kDataPin = 14;

constexpr auto kUpdateFrequency = 50.0;

} // namespace config