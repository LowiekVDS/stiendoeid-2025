#include "alternating.hpp"
#include "colors.hpp"

namespace effects {

Alternating::Alternating(const Config& config, CRGB* leds, int num_leds) : Effect(leds, num_leds), config_(config) {}

static Alternating::Config ParseConfigFromBytes(const uint8_t* bytes, int size) {
    Alternating::Config config;
    config.interval = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
    config.is_static = bytes[4] == 1;
    int num_colors = bytes[5];
    int offset = 6;
    for (int i = 0; i < num_colors; ++i) {
        GradientLevelPair gradient_level_pair;
        int extra_offset = ParseGradientLevelPairFromBytes(bytes + offset, size - offset, gradient_level_pair);
        if (extra_offset == 0) {
            return Alternating::Config{};
        }
        offset += extra_offset;
        config.colors.push_back(gradient_level_pair);
    }
    return config;
}  

void Alternating::update() {
    if (config_.is_static) {
        return;
    }

    // When the current step is greater than the interval, change the offset
    if (current_step_ > config_.interval) {
        offset_ = (offset_ + 1) % config_.colors.size();
        current_step_ = 0;
    }

    float position = static_cast<float>(current_step_) / static_cast<float>(config_.interval);
    for (int i = 0; i < config_.colors.size(); i++) {
        GradientLevelPair gradient_level_pair = config_.colors[i];
        auto rgb_color = GetCRGBColorFromGradientLevelPair(gradient_level_pair, position);
        CRGB color = CRGB(rgb_color.r, rgb_color.g, rgb_color.b);
        
        for (int j = (i + offset_) % config_.colors.size(); j < num_leds_; j += config_.colors.size()) {
            leds_[j] = color;
        }
    }

    ++current_step_;
}

} // namespace effects