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
      int interval;
      bool is_static;
      int group_level;
      std::vector<GradientLevelPair> colors;
    };

    static Config ParseConfigFromBytes(const uint8_t* bytes, int size);

    Alternating(const Config& config, CRGB* leds, int num_leds);

    // Takes ~360us
    void update() override final;

  private:
    Config config_;
    int offset_ = 0;
};

} // namespace effects

#endif // ALTERNATING_HPP