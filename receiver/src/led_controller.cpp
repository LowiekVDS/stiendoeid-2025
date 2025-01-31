#include "led_controller.hpp"

#include <FastLED.h>
#include "LittleFS.h"

#include "config.hpp"

LedController* LedController::Create(const Config &config) {
    LedController* led_controller = new LedController();
    
    //TODO: split led config into multiple pins
    FastLED.addLeds<SK6812, config::kDataPin, GRB>(led_controller->leds_, config.num_leds);    
    FastLED.setBrightness(255);

    if (!LittleFS.begin(true, "/littlefs")) {//, 10, "ffat")) {
        return nullptr;
    }
    led_controller->sequence_file_ = LittleFS.open(config.compressed_sequence_file_location, "r");
    if (!led_controller->sequence_file_) {
        return nullptr;
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

void LedController::StepSequence(bool update_leds) {
    if (next_update_step_ != step_) {
        step_++;
        return;
    }
    
    if (sequence_file_.position() == sequence_file_.size()) {
        sequence_file_.seek(0);
        step_ = 0;
        next_update_step_ = 0;
        for (int i = 0; i < kNumChannels; i++) {
            step_to_change_at_[i] = 0;
        }
    }

    bool fastled_change_required = false;

    for (int i = 0; i < kNumChannels; i++) {
        
        if (step_to_change_at_[i] == step_) {
            uint8_t buffer[2];
            sequence_file_.read(buffer, 2);

            step_to_change_at_[i] += buffer[1];

            switch (i % 3) {
            case 0:
                leds_[i / 3].r = buffer[0];
                break;
            case 1:
                leds_[i / 3].g = buffer[0];
                break;
            case 2:
                leds_[i / 3].b = buffer[0];
                break;
            }

            fastled_change_required = true;
        }
    }

    if (fastled_change_required) {
        next_update_step_ = -1;
        for (int i = 0; i < kNumChannels; i++) {
            if (step_to_change_at_[i] < next_update_step_) {
                next_update_step_ = step_to_change_at_[i];
            }
        }
    }

    if (update_leds && fastled_change_required) {
        FastLED.show();
    }

    step_++;
}