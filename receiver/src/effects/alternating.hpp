#ifndef ALTERNATING_HPP
#define ALTERNATING_HPP

#include <vector>

#include "FastLED.h"

#include "effect.hpp"
#include "colors.hpp"

namespace effects {

class Alternating : public Effect {  
  public:
    struct Config {
      bool is_static;
      int interval;
      std::vector<GradientLevelPair> colors;
    };

    Alternating(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:
    Config config_;
    int offset_ = 0;
};

} // namespace effects

#endif // ALTERNATING_HPP