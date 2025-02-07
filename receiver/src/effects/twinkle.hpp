#pragma once

#include "FastLED.h"

#include "effect.hpp"

namespace effects {

class Twinkle : public Effect {
  public:
    struct Config {
        CRGB* colors;
        size_t colors_size = 0;
        int max_brightness = 255;
        int min_brightness = 50;
        double variation = 0.3; // Percentage variation in brightness
        int avg_pulse_time = 20; // In milliseconds
        int coverage = 50; // Percentage of LEDs twinkling at a time
        double time_variation = 0.2; // Variation in twinkle duration
    };

    Twinkle(const Config& config, CRGB* leds, int num_leds);

    void update() override final;

  private:
    Config config_;
    int* twinkle_times_;
    int* brightness_levels_;
};

} // namespace effects