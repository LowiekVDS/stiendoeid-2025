#pragma once

#include "string.h"

#define MOCK_RADIO false
#define SERIAL_MODE false

#define ESP32
// #define ESP32S3


namespace config {

constexpr auto kRadioRxPin = 4;
constexpr auto kRadioSpeed = 6000;
constexpr auto kRadioPayloadSize = 4;
constexpr auto kRadioTxRate = 5.0;

const String kCompressedSequenceFileLocation = "/sequence.bin";

constexpr auto kNumChannelsPerLed = 3;
constexpr int kNumLeds[6] = {654, 0, 0, 0, 0, 0}; // {200, 200, 200, 54, 0, 0};
constexpr auto kTotalNumLeds = 654;

#ifdef ESP32
constexpr int kDataPin_1 = 25;
constexpr int kDataPin_2 = 26;
constexpr int kDataPin_3 = 15;
constexpr int kDataPin_4 = 16;
constexpr int kDataPin_5 = 17;
constexpr int kDataPin_6 = 18;
#endif

constexpr auto kBrightness = 255;

// Note: de frequentie is beperkt door de maximale aantal leds in een hardwarekanaal.
// Meerbepaald moet dit gelden: kUpdateFrequency < 25000 / (max(kNumLeds[i == 1..6])) 
constexpr auto kUpdateFrequency = 40.0;

constexpr auto kAllowedFrameDifference = 3;

} // namespace config