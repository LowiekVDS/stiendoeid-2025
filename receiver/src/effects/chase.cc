#include <algorithm>

#include "chase.hpp"

namespace effects {

namespace {

int clamp(int value, int min, int max) {
    return std::max(min, std::min(max, value));
}

} // namespace

Chase::Config Chase::ParseConfigFromBytes(const uint8_t* bytes, int size) {
    Chase::Config config;
    int offset = 0;
    config.interval = bytes[offset++] << 24 | bytes[offset++] << 16 | bytes[offset++] << 8 | bytes[offset++];
    config.color_handling = static_cast<Chase::ColorHandling>(bytes[offset++]);
    config.minimum_brightnes = bytes[offset++];
    offset += ParseCurvePointVectorOfStructsFromBytes(bytes + offset, size - offset, config.direction);
    config.pulse_overlap = bytes[offset++];
    offset += ParseGradientLevelPairFromBytes(bytes + offset, size - offset, config.color);
    return config;
}


void Chase::SetBackgroundColors() {
    if (config_.color_handling == ColorHandling::kGradientThruEffect) {
        for (int i = 0; i < num_leds_; ++i) {
            const float position = static_cast<float>(i) / static_cast<float>(num_leds_);
            const auto color = GetCRGBColorFromGradientLevelPair(config_.color, position);
            const float brightness_factor = config_.minimum_brightnes / 255.0;
            const auto min_brightness_color = RGBColor{
                static_cast<uint8_t>(color.r * brightness_factor),
                static_cast<uint8_t>(color.g * brightness_factor),
                static_cast<uint8_t>(color.b * brightness_factor)
            };
            leds_[i] += CRGB(min_brightness_color.r, min_brightness_color.g, min_brightness_color.b);
        }
    } else {
        const auto color = GetCRGBColorFromGradientLevelPair(config_.color, 0.0);
        const float brightness_factor = config_.minimum_brightnes / 255.0;
        const auto min_brightness_color = RGBColor{
            static_cast<uint8_t>(color.r * brightness_factor),
            static_cast<uint8_t>(color.g * brightness_factor),
            static_cast<uint8_t>(color.b * brightness_factor)
        };
        for (int i = 0; i < num_leds_; ++i) {
            leds_[i] += CRGB(min_brightness_color.r, min_brightness_color.g, min_brightness_color.b);
        }
    }
}

Chase::Chase(const Config& config, CRGB* leds, int num_leds)
    : Effect(leds, num_leds), config_(config) {
    SetBackgroundColors();
}

void Chase::update() {
    if (current_step_ > config_.interval) {
        current_step_ = 0;
    }

    auto steps_per_cycle = config_.interval / num_leds_;
    steps_per_cycle = clamp(steps_per_cycle, 1, config_.interval);
    if (current_step_ % steps_per_cycle == 0 || steps_per_cycle == 0) {
        const float position = static_cast<float>(current_step_) / static_cast<float>(config_.interval);
        const float direction = GetCurrentCurveY(config_.direction, position);
        current_led_ = std::round(direction * num_leds_);
    }

    SetBackgroundColors();

    RGBColor color;
    if (config_.color_handling == ColorHandling::kGradientThruEffect) {
        const float position = static_cast<float>(current_led_) / static_cast<float>(num_leds_);
        color = GetCRGBColorFromGradientLevelPair(config_.color, position);
    } else {
        const float local_position = static_cast<float>(current_step_ % steps_per_cycle) / static_cast<float>(steps_per_cycle == 0 ? 1 : steps_per_cycle);
        color = GetCRGBColorFromGradientLevelPair(config_.color, local_position);
    }
    const int led_start = current_led_ - std::floor((config_.pulse_overlap / 2.0) / steps_per_cycle == 0 ? 1 : steps_per_cycle);
    const int led_end = current_led_ + std::ceil((config_.pulse_overlap / 2.0) / steps_per_cycle == 0 ? 1 : steps_per_cycle);
    for (int i = led_start; i <= led_end; ++i) {
        leds_[i] = CRGB(color.r, color.g, color.b);
    }

    ++current_step_;
}

} // namespace effects