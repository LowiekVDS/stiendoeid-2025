#include "FastLED.h"

#include "set_level.hpp"

namespace effects {

static SetLevel::Config ParseConfigFromBytes(const uint8_t* bytes, int size) {
    if (size != 3) {
        return SetLevel::Config{};
    }
    SetLevel::Config config;
    config.color = RGBColor{bytes[0], bytes[1], bytes[2]};
    return config;
}

SetLevel::SetLevel(const Config& config, CRGB* leds, int num_leds) : Effect(leds, num_leds), config_(config) {
    for (int i = 0; i < num_leds_; i++) {
        leds_[i] = CRGB(config_.color.r, config_.color.g, config_.color.b);
    }
}

void SetLevel::update() {}

} // namespace effects