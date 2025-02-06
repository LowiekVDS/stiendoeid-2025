#include "src/effects/alternating.hpp"

namespace effects {

Alternating::Alternating(const Config& config, CRGB* leds, int num_leds) : Effect(leds, num_leds), config_(config) {
    for (int i = 0; i < num_leds_; i++) {
      leds_[i] = config_.colors[i % config_.colors_size];
    }
}

void Alternating::update() {
    if (config.is_static) {
      return;
    }

    if (current_step_ > config_.interval) {
        for (int i = 0; i < num_leds_; i++) {
            leds_[i] = config_.colors[(i + offset_) % config_.colors_size];
        }
        offset_ = (offset_ + 1) % config_.colors_size;
        current_step_ = 0;
    }
    ++current_step_;
}

} // namespace effects