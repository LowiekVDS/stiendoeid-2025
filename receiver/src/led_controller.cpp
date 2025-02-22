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
 * - 1 byte: number of effect objects to start
 * - For each effect object:
 *     - 1 byte: effect ID
 *     - 1 byte: effect type
 *     - 2 bytes: start led number
 *     - 2 bytes: end led number
 *     - 2 byte: length of configuration (=N)
 *     - N bytes: configuration
 * - 1 byte: num effects to stop (=Z)
 * - Z bytes: effect IDs to stop
 */

LedController* LedController::Create(const Config &config) {
    LedController* led_controller = new LedController();
    FastLED.addLeds<SK6812, config::kDataPin_1, GRB>(led_controller->leds_, 0, config::kNumLeds[0]);
    FastLED.addLeds<SK6812, config::kDataPin_2, GRB>(led_controller->leds_, config::kNumLeds[0], config::kNumLeds[1]);
    FastLED.addLeds<SK6812, config::kDataPin_3, GRB>(led_controller->leds_, config::kNumLeds[0] + config::kNumLeds[1], config::kNumLeds[2]);
    FastLED.addLeds<SK6812, config::kDataPin_4, GRB>(led_controller->leds_, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2], config::kNumLeds[3]);
    FastLED.addLeds<SK6812, config::kDataPin_5, GRB>(led_controller->leds_, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2] + config::kNumLeds[3], config::kNumLeds[4]);
    FastLED.addLeds<SK6812, config::kDataPin_6, GRB>(led_controller->leds_, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2] + config::kNumLeds[3] + config::kNumLeds[4], config::kNumLeds[5]);
    FastLED.setBrightness(config::kBrightness);

    // ESP32S3
    if (!LittleFS.begin(true, "/littlefs", 10, "ffat")) {
        return nullptr;
    }
    // ESP32
    // if (!LittleFS.begin(true, "/littlefs")) {
    //     return nullptr;
    // }

    if (!SERIAL_MODE) {
        led_controller->sequence_file_ = LittleFS.open(config.compressed_sequence_file_location, "r");
        if (!led_controller->sequence_file_) {
            return nullptr;
        }
    }
    for (int i = 0; i < 256; ++i) {
        led_controller->effects_[i] = nullptr;
    }

    // Read first 4 bytes to get the max number of steps
    uint32_t max_steps;
    led_controller->sequence_file_.read((uint8_t*)&max_steps, 4);
    led_controller->max_steps_ = max_steps;

    return led_controller;
}

void LedController::SeekToStep(uint32_t to_step) {
    sequence_file_.seek(4);
    step_ = 0;
    next_update_step_ = 0;
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

    if (step_ == next_update_step_) {
        if (sequence_file_.position() == sequence_file_.size()) {
            sequence_file_.seek(4);
            step_ = 0;
            next_update_step_ = 0;
            Serial.println("Restarting sequence");
        }


        uint8_t num_effects_to_stop;
        sequence_file_.read(&num_effects_to_stop, 1);
        for (int i = 0; i < num_effects_to_stop; ++i) {
            uint8_t effect_id;
            sequence_file_.read(&effect_id, 1);
            if (effects_[effect_id] != nullptr) {
                effects_[effect_id]->ClearLeds();
                delete effects_[effect_id];
                effects_[effect_id] = nullptr;
            }
        }
        
        uint16_t delta_length;
        sequence_file_.read((uint8_t*)&delta_length, 2);

        uint8_t num_effects_to_update;
        sequence_file_.read(&num_effects_to_update, 1);

        // Serial.println("Step: " + String(step_));
        // Serial.println("Delta length: " + String(delta_length));
        // Serial.println("Num effects to update: " + String(num_effects_to_update));

        // make a map of effect_ids that have been updated
        std::vector<int> updated_effect_ids;

        for (int i = 0; i < num_effects_to_update; ++i) {
            uint8_t effect_id;
            sequence_file_.read(&effect_id, 1);

            updated_effect_ids.push_back(effect_id);

            if (effects_[effect_id] != nullptr) {
                effects_[effect_id]->ClearLeds();
                delete effects_[effect_id];
                effects_[effect_id] = nullptr;
            }

            uint8_t effect_type;
            sequence_file_.read(&effect_type, 1);
            uint16_t start_led;
            sequence_file_.read((uint8_t*)&start_led, 2);
            uint16_t end_led;
            sequence_file_.read((uint8_t*)&end_led, 2);
            uint16_t config_length;
            sequence_file_.read((uint8_t*)&config_length, 2);
            uint8_t config_as_bytes[config_length];
            sequence_file_.read(config_as_bytes, config_length);

            // Serial.println("Effect ID: " + String(effect_id));
            // Serial.println("Effect type: " + String(effect_type));
            // Serial.println("Start led: " + String(start_led));
            // Serial.println("End led: " + String(end_led));
            // Serial.println("Config length: " + String(config_length));
            // Serial.print("Config: ");
            // for (int i = 0; i < config_length; ++i) {
            //     Serial.print(String(config_as_bytes[i], HEX) + " ");
            // }
            // Serial.println();
            
            switch(effect_type) {
            case EffectType::Alternating:
                effects_[effect_id] = new effects::Alternating(effects::Alternating::ParseConfigFromBytes(config_as_bytes, config_length), leds_ + start_led, end_led - start_led);
                break;
            case EffectType::Chase:
                effects_[effect_id] = new effects::Chase(effects::Chase::ParseConfigFromBytes(config_as_bytes, config_length), leds_ + start_led, end_led - start_led);
                break;
            case EffectType::Dissolve:
                effects_[effect_id] = new effects::Dissolve(effects::Dissolve::ParseConfigFromBytes(config_as_bytes, config_length), leds_ + start_led, end_led - start_led);
                break;
            case EffectType::Pulse:
                effects_[effect_id] = new effects::Pulse(effects::Pulse::ParseConfigFromBytes(config_as_bytes, config_length), leds_ + start_led, end_led - start_led);
                break;
            case EffectType::SetLevel:
                effects_[effect_id] = new effects::SetLevel(effects::SetLevel::ParseConfigFromBytes(config_as_bytes, config_length), leds_ + start_led, end_led - start_led);
                break;
            case EffectType::Strobe:
                effects_[effect_id] = new effects::Strobe(effects::Strobe::ParseConfigFromBytes(config_as_bytes, config_length), leds_ + start_led, end_led - start_led);
                break;
            case EffectType::Twinkle:
                effects_[effect_id] = new effects::Twinkle(effects::Twinkle::ParseConfigFromBytes(config_as_bytes, config_length), leds_ + start_led, end_led - start_led );
                break;
            case None:
            default:
                effects_[effect_id] = new effects::SetLevel({0, 0, 0}, leds_ + start_led, end_led - start_led);
                break;
            }   
        }


        // // All effects that were not updated should be cleared
        // for (int i = 0; i < 256; ++i) {
        //     if (std::find(updated_effect_ids.begin(), updated_effect_ids.end(), i) == updated_effect_ids.end()) {
        //         if (effects_[i] != nullptr) {
        //             effects_[i]->ClearLeds();
        //             delete effects_[i];
        //             effects_[i] = nullptr;
        //         }
        //     }
        // }

        next_update_step_ += delta_length;
    }

    if (!update_leds) {
        step_++;
        return;
    }

    // Serial.println("Step: " + String(step_));

    for (int i = 0; i < config::kTotalNumLeds; ++i) {
        leds_[i] = CRGB::Black;
    }
    
    for (int i = 0; i < 256; ++i) {
        if (effects_[i] != nullptr) {
            effects_[i]->update();
        }
    }

    FastLED.show();

    step_++;    
}


/**
 * Server: 0 - 2000; 0 - 2000; 0 - 2000
 * 
 */