#pragma once

#include "FastLED.h"

#include "effect.hpp"
#include "colors.hpp"

namespace effects {

class SetLevel : public Effect {  
  public:
    struct Config {
      RGBColor color;
    };

    static Config ParseConfigFromBytes(const uint8_t* bytes, int size);

    SetLevel(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:
    Config config_;
};

} // namespace effects