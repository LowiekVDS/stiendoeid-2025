#pragma once

#include "FastLED.h"

#include "src/effects/effect.hpp"

namespace effects {

class Alternating : public Effect {  
  public:
    struct Config {
      bool is_static = false;
      int interval = 20; // In steps
      CRGB colors[] = {};
      size_t colors_size = 0;
    };

    Alternating(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:
    Config config_;
    int offset_ = 0;
};

} // namespace effects