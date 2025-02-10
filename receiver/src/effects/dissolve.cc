#include "dissolve.hpp"
#include "tools.hpp"

namespace effects {

Dissolve::Dissolve(const Config& config, CRGB* leds, int num_leds)
    : Effect(leds, num_leds), config_(config) {

    for (int i = 0; i < num_leds; i++) {
        led_mapping_.push_back(i);
        color_mapping_.push_back(i % config_.colors.size());
    }
    if (config_.is_random) {
        std::random_shuffle(led_mapping_.begin(), led_mapping_.end());
    }
    if (config_.random_color_order) {
        std::random_shuffle(color_mapping_.begin(), color_mapping_.end());
    }

    if (config_.colors.size() == 0) {
        config_.colors.push_back({ColorGradient(), {}});
    }
}

Dissolve::Config Dissolve::ParseConfigFromBytes(const uint8_t* bytes, int size) {
    Dissolve::Config config;
    if (size < 1) {
        return config;
    }

    int offset = 0;
    config.interval = bytes[offset++] << 24 | bytes[offset++] << 16 | bytes[offset++] << 8 | bytes[offset++];
    config.is_random = bytes[offset++];
    config.flip = bytes[offset++];
    config.both_directions = bytes[offset++];
    config.random_color_order = bytes[offset++];
    config.alternate_colors = bytes[offset++];

    offset += ParseCurvePointVectorOfStructsFromBytes(bytes + offset, size - offset, config.density);
    int num_colors = bytes[offset];
    for (int i = 0; i < num_colors; ++i) {
        GradientLevelPair gradient_level_pair;
        offset += ParseGradientLevelPairFromBytes(bytes + offset, size - offset, gradient_level_pair);
        config.colors.push_back(gradient_level_pair);
    }
    return config;
}

void Dissolve::update() {
    if (current_step_ >= config_.interval) {
        current_step_ = 0;
    }

    float position = static_cast<float>(current_step_) / config_.interval;
    int num_leds_to_skip_from_start = num_leds_ - GetCurrentCurveY(config_.density, position) * num_leds_;
    int num_leds_to_skip_from_end = config_.both_directions ? num_leds_to_skip_from_start : 0;

    if (config_.flip) {
        int temp = num_leds_to_skip_from_start;
        num_leds_to_skip_from_start = num_leds_to_skip_from_end;
        num_leds_to_skip_from_end = temp;
    }

    for (int i = 0; i < num_leds_; i++) {
        if (i < num_leds_to_skip_from_start) {
            leds_[led_mapping_[i]] = CRGB::Black;
        } else if (i >= num_leds_ - num_leds_to_skip_from_end) {
            leds_[led_mapping_[i]] = CRGB::Black;
        } else {
            int color_index = 0;
            if (config_.alternate_colors) {
                color_index = led_mapping_[i] % config_.colors.size();
            } else if (config_.random_color_order) {
                color_index = color_mapping_[i];
            }
            const auto rgb_color = GetCRGBColorFromGradientLevelPair(config_.colors[color_index], position);
            leds_[led_mapping_[i]] = CRGB(rgb_color.r, rgb_color.g, rgb_color.b);
        }
    }

    ++current_step_;
}

} // namespace effects