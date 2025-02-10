#pragma once

#include "FastLED.h"

#include "effect.hpp"
#include "colors.hpp"

namespace effects {

class Strobe : public Effect {  
  public:
    struct Config {
        int interval; // Total time the effect is active
        int cycle_time; // The default cycle time (at cycle_variation == 1.0)
        std::vector<CurvePoint> cycle_variation;
        std::vector<CurvePoint> on_time;
        GradientLevelPair color;
    };

    static Strobe::Config ParseConfigFromBytes(const uint8_t* bytes, int size);

    Strobe(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:
    Config config_;
    int cycle_step_ = 0;
    int current_actual_cycle_time_ = 0;
    int current_on_time_ = 0;

};

} // namespace effects