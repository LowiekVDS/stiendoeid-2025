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
    uint32_t MaxSteps() const { return max_steps_; }
    
  private:

    enum EffectType {
      None,
      Alternating,
      Chase,
      Dissolve,
      Pulse,
      SetLevel,
      Strobe,
      Twinkle,
    };

    static const auto kNumChannels = config::kTotalNumLeds * config::kNumChannelsPerLed;

    LedController() = default;

    CRGB leds_[config::kTotalNumLeds]; 
    effects::Effect* effects_[256] = {nullptr};
    File sequence_file_;
    uint32_t step_ = 0;
    uint32_t next_update_step_ = 0;
    uint32_t max_steps_ = 0;
};