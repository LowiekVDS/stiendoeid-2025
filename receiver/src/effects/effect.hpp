#pragma once

#include "FastLED.h"

namespace effects {

class Effect {
  public:

    Effect(CRGB* leds, int num_leds) : leds_(leds), num_leds_(num_leds) {};
    
    virtual void update() = 0;

  protected:
    CRGB* leds_ = nullptr;
    int num_leds_ = 0;
    int current_step_ = 0;
};

} // namespace effects