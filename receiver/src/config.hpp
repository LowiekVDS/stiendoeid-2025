#pragma once

#include "string.h"

namespace config {

const String kCompressedSequenceFileLocation = "/sequence.bin";

constexpr auto kNumLeds = 30;
constexpr auto kNumChannelsPerLed = 3;
constexpr auto kDataPin = 14;

constexpr auto kUpdateFrequency = 50.0;

} // namespace config