#pragma once

#include "FastLED.h"

#include "effect.hpp"
#include "colors.hpp"

namespace effects {

class Twinkle : public Effect {  
  public:
    
    enum ColorHandling {
        kSingleColor,
        kGradientThruEffect,
        kGradientPerPulse,
        kGradientAccrossItems
    };

    struct Config {
        int interval; // in steps
        float avg_pulse_interval; // in steps
        float coverage; // between 0 and 1
        float coverage_variation;
        uint8_t min_brightness;
        uint8_t max_brightness;
        float brightness_variation; // 0 and 1
        ColorHandling color_handling;
        GradientLevelPair color;
    };

    static Twinkle::Config ParseConfigFromBytes(const uint8_t* bytes, int size);

    Twinkle(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:

    void SetBackgroundColors();

    Config config_;
    std::vector<int> pulses_start_step_;
    std::vector<bool> lit_;
    std::vector<int> pulses_end_step_;
};

} // namespace effects