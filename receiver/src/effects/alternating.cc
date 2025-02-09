#include "alternating.hpp"

namespace effects {

namespace {

CRGB GetCRGBColorFromGradientLevelPair(const GradientLevelPair& gradient_level_pair, float position) {
    RGBColor color = GetColorAt(gradient_level_pair.colorGradient, position);
    RGBColor adjusted_color = ApplyBrightnessCurve(color, gradient_level_pair.brightness, position);
    return CRGB(adjusted_color.r, adjusted_color.g, adjusted_color.b);
};

} // namespace

Alternating::Alternating(const Config& config, CRGB* leds, int num_leds) : Effect(leds, num_leds), config_(config) {

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
        CRGB color = GetCRGBColorFromGradientLevelPair(gradient_level_pair, position);
        
        for (int j = (i + offset_) % config_.colors.size(); j < num_leds_; j += config_.colors.size()) {
            leds_[j] = color;
        }
    }

    ++current_step_;
}

} // namespace effects