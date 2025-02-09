#pragma once

#include "string.h"

#include <FastLED.h>
#include "LittleFS.h"

#include "effects/effect.hpp"
#include "config.hpp"

class LedController {
  public:

    struct Config {
        int num_leds;
        String compressed_sequence_file_location;
    };

    static LedController* Create(const Config &config);

    void SeekToStep(uint32_t to_step);
    void StepSequence(bool update_leds=true);
    bool SetLedsFromBuffer(uint8_t* buffer, int buffer_size);

    uint32_t Step() const { return step_; }
    CRGB leds_[config::kTotalNumLeds]; 
  private:

    static const auto kNumChannels = config::kTotalNumLeds * config::kNumChannelsPerLed;

    LedController() = default;

    effects::Effect* effects_[256];
    uint8_t num_actual_effects_ = 0;

    
    uint32_t step_to_change_at_[kNumChannels] = {0};
    File sequence_file_;
    uint32_t step_ = 0;
    uint32_t next_update_step_ = 0;
};