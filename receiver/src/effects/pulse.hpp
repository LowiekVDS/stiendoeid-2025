#pragma once

#include "FastLED.h"

#include "effect.hpp"
#include "colors.hpp"

namespace effects {

class Pulse : public Effect {  
  public:
    struct Config {
        int interval;
        GradientLevelPair color;
    };

    static Pulse::Config ParseConfigFromBytes(const uint8_t* bytes, int size);

    Pulse(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:
    Config config_;
};

} // namespace effects