#pragma once

#include <map>

#include "FastLED.h"

#include "effect.hpp"
#include "colors.hpp"

namespace effects {

class Dissolve : public Effect {  
  public:

    struct Config {
        int interval; // Total time the effect is active
        bool is_random;
        bool flip;
        bool both_directions;
        std::vector<CurvePoint> density;
        std::vector<GradientLevelPair> colors;
        bool random_color_order;
        bool alternate_colors; // group colors or color per step
    };

    Dissolve(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:

    void SetBackgroundColors();

    Config config_;
    std::vector<int> led_mapping_;
    std::vector<int> color_mapping_;
};

} // namespace effects