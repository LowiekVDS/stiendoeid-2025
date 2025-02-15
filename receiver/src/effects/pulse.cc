#include "pulse.hpp"

namespace effects {

Pulse::Pulse(const Config& config, CRGB* leds, int num_leds) : Effect(leds, num_leds), config_(config) {}

Pulse::Config Pulse::ParseConfigFromBytes(const uint8_t* bytes, int size) {
    if (size < 1) {
        return Pulse::Config{};
    }

    Pulse::Config config;
    config.interval = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
    ParseGradientLevelPairFromBytes(bytes + 4, size - 4, config.color);
    return config;
}

void Pulse::update() {
    
    if (current_step_ > config_.interval) {
        current_step_ = 0;
    }
    const float position = static_cast<float>(current_step_) / static_cast<float>(config_.interval);

    const auto rgb_color = GetCRGBColorFromGradientLevelPair(config_.color, position);
    const CRGB color = CRGB(rgb_color.r, rgb_color.g, rgb_color.b);

    for (int i = 0; i < num_leds_; ++i) {
        leds_[i] += color;
    }

    ++current_step_;
}

} // namespace effects