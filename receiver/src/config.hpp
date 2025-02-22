#pragma once

#include "string.h"

#define MOCK_RADIO false
#define SERIAL_MODE false

#define CHIP ESP32S3 // ESP32

namespace config {

constexpr auto kRadioRxPin = 4;
constexpr auto kRadioSpeed = 2000;
constexpr auto kRadioPayloadSize = 4;
constexpr auto kRadioTxRate = 5.0;

const String kCompressedSequenceFileLocation = "/sequence.bin";

constexpr auto kNumChannelsPerLed = 3;
constexpr int kNumLeds[6] = {276, 0, 228, 0, 78, 72};
constexpr auto kTotalNumLeds = 654;

// ESP32
// constexpr int kDataPin_1 = 25;
// constexpr int kDataPin_2 = 26;
// constexpr int kDataPin_3 = 15;
// constexpr int kDataPin_4 = 16;
// constexpr int kDataPin_5 = 17;
// constexpr int kDataPin_6 = 18;

// // ESP32S3
constexpr int kDataPin_1 = 7;
constexpr int kDataPin_2 = 6;
constexpr int kDataPin_3 = 16;
constexpr int kDataPin_4 = 15;
constexpr int kDataPin_5 = 17;
constexpr int kDataPin_6 = 18;

constexpr auto kBrightness = 255;

// Note: de frequentie is beperkt door de maximale aantal leds in een hardwarekanaal.
// Meerbepaald moet dit gelden: kUpdateFrequency < 25000 / (max(kNumLeds[i == 1..6])) 
constexpr auto kUpdateFrequency = 25.0;

constexpr auto kAllowedFrameDifference = 10;

} // namespace config