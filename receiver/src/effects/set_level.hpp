#pragma once

#include "FastLED.h"

#include "src/effects/effect.hpp"

namespace effects {

class SetLevel : public Effect {  
  public:
    struct Config {
      CRGB color;
    };

    SetLevel(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:
    Config config_;
};

} // namespace effects