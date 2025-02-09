#include "led_controller.hpp"

#include <FastLED.h>
#include "LittleFS.h"

#include "effects/all.hpp"
#include "config.hpp"

/**
 * Format of a sequence file.
 * 
 * Per line:
 * - 2 bytes: length of this iteration in steps.
 * - 1 byte: number of effect objects
 * - For each effect object:
 *     - 1 byte: effect type
 *     - 2 bytes: start led number
 *     - 2 bytes: end led number 
 *     - 1 byte: length of configuration (=N)
 *     - N bytes: configuration
 */

namespace {

enum EffectType {
    Alternating,
    CandleFlicker,
    Chase,
    Dissolve,
    LipSync,
    Pulse,
    SetLevel,
    Spin,
    Twinkle,
    Wipe
};

} // namespace

LedController* LedController::Create(const Config &config) {
    LedController* led_controller = new LedController();
    FastLED.addLeds<SK6812, config::kDataPin_1, GRB>(led_controller->leds_, 0, config::kNumLeds[0]);
    FastLED.addLeds<SK6812, config::kDataPin_2, GRB>(led_controller->leds_, config::kNumLeds[0], config::kNumLeds[1]);
    FastLED.addLeds<SK6812, config::kDataPin_3, GRB>(led_controller->leds_, config::kNumLeds[0] + config::kNumLeds[1], config::kNumLeds[2]);
    FastLED.addLeds<SK6812, config::kDataPin_4, GRB>(led_controller->leds_, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2], config::kNumLeds[3]);
    FastLED.addLeds<SK6812, config::kDataPin_5, GRB>(led_controller->leds_, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2] + config::kNumLeds[3], config::kNumLeds[4]);
    FastLED.addLeds<SK6812, config::kDataPin_6, GRB>(led_controller->leds_, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2] + config::kNumLeds[3] + config::kNumLeds[4], config::kNumLeds[5]);
    FastLED.setBrightness(config::kBrightness);

    if (!LittleFS.begin(true, "/littlefs")) {//, 10, "ffat")) {
        return nullptr;
    }
    led_controller->sequence_file_ = LittleFS.open(config.compressed_sequence_file_location, "r");
    if (!led_controller->sequence_file_) {
        return nullptr;
    }
    for (int i = 0; i < 256; ++i) {
        led_controller->effects_[i] = nullptr;
    }

    return led_controller;
}

void LedController::SeekToStep(uint32_t to_step) {
    sequence_file_.seek(0);
    step_ = 0;
    for (int i = 0; i < to_step; ++i) {
        StepSequence(false);
    }
}

bool LedController::SetLedsFromBuffer(uint8_t* buffer, int buffer_size) {
    if (buffer_size != kNumChannels) {
        return false;
    }
    for (int i = 0; i < kNumChannels; i++) {
        switch (i % 3) {
        case 0:
            leds_[i / 3].r = buffer[i];
            break;
        case 1:
            leds_[i / 3].g = buffer[i];
            break;
        case 2:
            leds_[i / 3].b = buffer[i];
            break;
        }
    }
    FastLED.show();
    return true;
}

void LedController::StepSequence(bool update_leds) {

    if (sequence_file_.position() == sequence_file_.size()) {
        sequence_file_.seek(0);
        step_ = 0;
        next_update_step_ = 0;
    }

    if (step_ == next_update_step_) {

        for (int i = 0; i < num_actual_effects_; ++i) {
            delete effects_[i];
        }

        uint16_t delta_length; 
        sequence_file_.read((uint8_t*)&delta_length, 2);
        uint8_t num_effects;
        sequence_file_.read(&num_effects, 1);

        for (int i = 0; i < num_effects; ++i) {
            uint8_t effect_type;
            sequence_file_.read(&effect_type, 1);
            uint16_t start_led;
            sequence_file_.read((uint8_t*)&start_led, 2);
            uint16_t end_led;
            sequence_file_.read((uint8_t*)&end_led, 2);
            uint8_t config_length;
            sequence_file_.read(&config_length, 1);
            uint8_t config_as_bytes[config_length];
            sequence_file_.read(config_as_bytes, config_length);
            
            if (effect_type == EffectType::Alternating) {
                effects::Alternating::Config config;
                assert(config_length == sizeof(effects::Alternating::Config));
                sequence_file_.read((uint8_t*)&config, config_length);
                effects_[i] = new effects::Alternating(config, leds_ + start_led, end_led - start_led + 1);
            } else if (effect_type == EffectType::SetLevel) {
                effects::SetLevel::Config config;
                assert(config_length == sizeof(effects::SetLevel::Config));
                sequence_file_.read((uint8_t*)&config, config_length);
                effects_[i] = new effects::SetLevel(config, leds_ + start_led, end_led - start_led + 1);
            } else {
                // By default, set black because not supported :(.
                const effects::SetLevel::Config config = {CRGB::Black};
                effects_[i] = new effects::SetLevel(config, leds_ + start_led, end_led - start_led + 1);
                break;
            }
        }

        next_update_step_ += delta_length;
        num_actual_effects_ = num_effects;
    }

    for (int i = 0; i < num_actual_effects_; ++i) {
        effects_[i]->update();    
    }

    if (update_leds) {
        FastLED.show();
    }

    step_++;
}