#include "src/effects/alternating.hpp"

namespace effects {

void Alternating::update() {
    if (config_.is_static || current_step_ == 0) {
        for (int i = 0; i < num_leds_; i++) {
            leds_[i] = config_.colors[i % config_.colors_size];
        }
    } else {
        if (current_step_ > config_.interval) {
            for (int i = 0; i < num_leds_; i++) {
                leds_[i] = config_.colors[(i + offset_) % config_.colors_size];
            }
            offset_ = (offset_ + 1) % config_.colors_size;
            current_step_ = 0;
        }
    }
    ++current_step_;
}

} // namespace effects