#include "strobe.hpp"
#include "tools.hpp"

namespace effects {

Strobe::Strobe(const Config& config, CRGB* leds, int num_leds) : Effect(leds, num_leds), config_(config) {}

void Strobe::update() {
    if (current_step_ > config_.interval) {
        current_step_ = 0;
        cycle_step_ = 0;
    }

    float position = static_cast<float>(current_step_) / static_cast<float>(config_.interval);

    // Time to setup a new cycle!
    if (cycle_step_ >= current_actual_cycle_time_) {
        cycle_step_ = 0;

        current_actual_cycle_time_ = config_.cycle_time * GetCurrentCurveY(config_.cycle_variation, position);
        current_on_time_ = current_actual_cycle_time_ * GetCurrentCurveY(config_.on_time, position);

        Serial.println("New cycle!");
        Serial.println("Cycle time: " + String(current_actual_cycle_time_));
        Serial.println("On time: " + String(current_on_time_));
    }

    // Apply effect
    if (cycle_step_ < current_on_time_) {
        float on_time_position = static_cast<float>(cycle_step_) / static_cast<float>(current_on_time_);
        RGBColor color = GetCRGBColorFromGradientLevelPair(config_.color, on_time_position);
        for (int i = 0; i < num_leds_; ++i) {
            leds_[i] = CRGB(color.r, color.g, color.b);
        }
    } else {
        for (int i = 0; i < num_leds_; ++i) {
            leds_[i] = CRGB::Black;
        }
    }

    ++current_step_;
    ++cycle_step_;
}

} // namespace effects