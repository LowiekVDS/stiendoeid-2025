#pragma once

#include "FastLED.h"

#include "set_level.hpp"

namespace effects {

SetLevel::SetLevel(const Config& config, CRGB* leds, int num_leds) : Effect(leds, num_leds), config_(config) {
    for (int i = 0; i < num_leds_; i++) {
        leds_[i] = config_.color;
    }
}

void SetLevel::update() {}

} // namespace effects