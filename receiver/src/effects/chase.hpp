#pragma once

#include "FastLED.h"

#include "effect.hpp"
#include "colors.hpp"

namespace effects {

class Chase : public Effect {  
  public:
    
    enum ColorHandling {
        kSingleColor,
        kGradientThruEffect,
        kGradientPerPulse,
        kGradientAccrossItems
    };

    struct Config {
        int interval; // Total time the effect is active
        ColorHandling color_handling;
        uint8_t minimum_brightnes;
        std::vector<CurvePoint> direction;
        int pulse_overlap; // in steps
        GradientLevelPair color;
    };

    static Chase::Config ParseConfigFromBytes(const uint8_t* bytes, int size);

    Chase(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:

    void SetBackgroundColors();

    Config config_;
    int current_led_ = 0;
};

} // namespace effects