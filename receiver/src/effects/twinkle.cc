#include "twinkle.hpp"

namespace effects {

Twinkle::Config Twinkle::ParseConfigFromBytes(const uint8_t* bytes, int size) {
    Twinkle::Config config;
    int offset = 0;
    config.interval = bytes[offset++] << 24 | bytes[offset++] << 16 | bytes[offset++] << 8 | bytes[offset++];

    memcpy(&config.avg_pulse_interval, bytes + offset, sizeof(float));
    offset += sizeof(float);
    memcpy(&config.coverage, bytes + offset, sizeof(float));
    offset += sizeof(float);
    memcpy(&config.coverage_variation, bytes + offset, sizeof(float));
    offset += sizeof(float);

    config.min_brightness = bytes[offset++];
    config.max_brightness = bytes[offset++];
    
    memcpy(&config.brightness_variation, bytes + offset, sizeof(float));
    offset += sizeof(float);

    config.color_handling = static_cast<Twinkle::ColorHandling>(bytes[offset++]);
    offset += ParseGradientLevelPairFromBytes(bytes + offset, size - offset, config.color);
    return config;
}


Twinkle::Twinkle(const Config& config, CRGB* leds, int num_leds)
    : Effect(leds, num_leds), config_(config) {
    for (int i = 0; i < num_leds_; ++i) {
        pulses_end_step_.push_back(0);
        pulses_start_step_.push_back(0);
        lit_.push_back(false);
    }
    if (config_.color_handling == ColorHandling::kGradientThruEffect) {
        for (int i = 0; i < num_leds_; ++i) {
            const float position = static_cast<float>(i) / static_cast<float>(num_leds_);
            const auto color = GetCRGBColorFromGradientLevelPair(config_.color, position);
            const float brightness_factor = config_.min_brightness / 255.0;
            const auto min_brightness_color = RGBColor{
                static_cast<uint8_t>(color.r * brightness_factor),
                static_cast<uint8_t>(color.g * brightness_factor),
                static_cast<uint8_t>(color.b * brightness_factor)
            };
            background_colors_.push_back(CRGB(min_brightness_color.r, min_brightness_color.g, min_brightness_color.b));
        }
    } else {
        const auto color = GetCRGBColorFromGradientLevelPair(config_.color, 0.0);
        const float brightness_factor = config_.min_brightness / 255.0;
        const auto min_brightness_color = RGBColor{
            static_cast<uint8_t>(color.r * brightness_factor),
            static_cast<uint8_t>(color.g * brightness_factor),
            static_cast<uint8_t>(color.b * brightness_factor)
        };
        for (int i = 0; i < num_leds_; ++i) {
            background_colors_.push_back(CRGB(min_brightness_color.r, min_brightness_color.g, min_brightness_color.b));
        }
    }
    for (int i = 0; i < num_leds_; ++i) {
        twinkle_colors_.push_back(CRGB::Black);
    }
    SetBackgroundColors();
}

void Twinkle::SetBackgroundColors() {
    for (int i = 0; i < num_leds_; ++i) {
        leds_[i] += background_colors_[i];
    }
}

void Twinkle::update() {

    SetBackgroundColors();

    for (int i = 0; i < num_leds_; ++i) {
        if (pulses_end_step_[i] <= current_step_) {
            int actual_time = config_.avg_pulse_interval * (1 + config_.coverage_variation * (random(100) / 50.0 - 1));
            pulses_end_step_[i] = current_step_ + actual_time;
            pulses_start_step_[i] = current_step_;
            if (random(100) < config_.coverage * 100.0) {
                lit_[i] = true;
            } else {
                lit_[i] = false;
            }
        }
    }   

    const int avg_brightness = (config_.min_brightness + config_.max_brightness) / 2;
    float position = static_cast<float>(current_step_) / static_cast<float>(config_.interval);
    bool update_next = true;
    for (int i = 0; i < num_leds_; ++i) {
        if (pulses_end_step_[i] > current_step_ && lit_[i] && update_next) {

            if (config_.color_handling != ColorHandling::kGradientThruEffect) {
                position = static_cast<float>(current_step_ - pulses_start_step_[i]) / static_cast<float>(pulses_end_step_[i] - pulses_start_step_[i]);
            }
            const RGBColor color = GetCRGBColorFromGradientLevelPair(config_.color, position);
            const float brightness_factor = (config_.min_brightness + (config_.max_brightness - config_.min_brightness) * ((1.0 - config_.brightness_variation) / 2.0 + config_.brightness_variation * (random(100) / 100.0)));
            const auto twinkle_color = RGBColor{
                static_cast<uint8_t>(color.r * brightness_factor / 255.0),
                static_cast<uint8_t>(color.g * brightness_factor / 255.0),
                static_cast<uint8_t>(color.b * brightness_factor / 255.0)
            };
            twinkle_colors_[i] = CRGB(twinkle_color.r, twinkle_color.g, twinkle_color.b);
        } else if (update_next) {
            twinkle_colors_[i] = CRGB::Black;
        }
        update_next = !update_next;
        leds_[i] += twinkle_colors_[i];
    }

    ++current_step_;
}

} // namespace effects