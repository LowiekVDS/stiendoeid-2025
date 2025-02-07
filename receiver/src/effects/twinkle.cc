#include "twinkle.hpp"

#include <cstdlib>

namespace effects {

Twinkle::Twinkle(const Config& config, CRGB* leds, int num_leds) 
    : Effect(leds, num_leds), config_(config) {
    twinkle_times_ = new int[num_leds_];
    brightness_levels_ = new int[num_leds_];
    
    for (int i = 0; i < num_leds_; i++) {
        int min_time = config_.avg_pulse_time * (1 - config_.time_variation);
        int max_time = config_.avg_pulse_time * (1 + config_.time_variation);
        twinkle_times_[i] = min_time + (rand() % (max_time - min_time));

        int min_
    }
}

void Twinkle::update() {
    for (int i = 0; i < num_leds_; i++) {
        if (rand() % 100 < config_.coverage) {
            int brightness = brightness_levels_[i];
            int color_index = rand() % config_.colors_size;
            leds_[i] = config_.colors[color_index];
            leds_[i].fadeToBlackBy(255 - brightness);
            
            if (--twinkle_times_[i] <= 0) {
                twinkle_times_[i] = rand() % (config_.avg_pulse_time + config_.time_variation);
                brightness_levels_[i] = config_.min_brightness + 
                    (rand() % (config_.max_brightness - config_.min_brightness));
            }
        }
    }
}

} // namespace effects
