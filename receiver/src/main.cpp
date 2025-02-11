#include <FastLED.h>

#include "LittleFS.h"

#include "led_controller.hpp"
#include "config.hpp"
#include "radio_time_source.hpp"

bool serial_mode = false;

TaskHandle_t sequence_handling_task_handle;
TaskHandle_t time_sync_handling_task_handle;

QueueHandle_t radio_time_queue_handle;

RadioTimeSource* radio_time_source = nullptr;
TickType_t xLastWakeTime;

void SequenceHandlingTask(void *params) {
    auto led_controller = LedController::Create({
        config::kTotalNumLeds, 
        config::kCompressedSequenceFileLocation
    });
    if (led_controller == nullptr) {
        Serial.println("Failed to create led controller");
        return;
    }

    TickType_t xLastWakeTime;
    unsigned long reference_millis = 0;
    constexpr double period_millis = 1000.0 / config::kUpdateFrequency;
    while (true) {

        xQueueReceive(radio_time_queue_handle, &reference_millis, 0);
        const unsigned long radio_sequence_time_millis = reference_millis + millis();

        unsigned long local_sequence_time_millis = led_controller->Step() * period_millis;
        if (radio_sequence_time_millis > local_sequence_time_millis + config::kAllowedFrameDifference * period_millis) {
            while (radio_sequence_time_millis > local_sequence_time_millis + config::kAllowedFrameDifference * period_millis) {
                led_controller->StepSequence(false);
                local_sequence_time_millis = led_controller->Step() * period_millis;
            }
        } else if (local_sequence_time_millis > radio_sequence_time_millis + config::kAllowedFrameDifference * period_millis) {
            led_controller->SeekToStep(radio_sequence_time_millis / period_millis);
        }

        led_controller->StepSequence(true);
    
        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        if (!xWasDelayed) {
            Serial.println("[WARNING] SequenceHandlingTask was NOT delayed!");
        }
    }
}

void SequenceHandlingTaskFromSerial(void *params) {
    Serial.println("Serial mode starting.");
    CRGB leds[config::kTotalNumLeds];
    FastLED.addLeds<SK6812, config::kDataPin_1, GRB>(leds, 0, config::kNumLeds[0]);
    FastLED.addLeds<SK6812, config::kDataPin_2, GRB>(leds, config::kNumLeds[0], config::kNumLeds[1]);
    FastLED.addLeds<SK6812, config::kDataPin_3, GRB>(leds, config::kNumLeds[0] + config::kNumLeds[1], config::kNumLeds[2]);
    FastLED.addLeds<SK6812, config::kDataPin_4, GRB>(leds, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2], config::kNumLeds[3]);
    FastLED.addLeds<SK6812, config::kDataPin_5, GRB>(leds, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2] + config::kNumLeds[3], config::kNumLeds[4]);
    FastLED.addLeds<SK6812, config::kDataPin_6, GRB>(leds, config::kNumLeds[0] + config::kNumLeds[1] + config::kNumLeds[2] + config::kNumLeds[3] + config::kNumLeds[4], config::kNumLeds[5]);
    
    Serial.println("Serial mode started.");

    auto kNumChannels = config::kNumChannelsPerLed * config::kTotalNumLeds;
    uint8_t buffer[kNumChannels];
    while (true) {
        Serial.readBytes(buffer, kNumChannels);
        for (int i = 0; i < kNumChannels; i++) {
            switch (i % 3) {
            case 0:
                leds[i / 3].r = buffer[i];
                break;
            case 1:
                leds[i / 3].g = buffer[i];
                break;
            case 2:
                leds[i / 3].b = buffer[i];
                break;
            }
        }
        FastLED.show();

        vTaskDelay(pdMS_TO_TICKS(1));
        Serial.println("Serial mode");
    }
}

void setup() {
    Serial.begin(921600);

    radio_time_queue_handle = xQueueCreate(1, sizeof(unsigned long));
    radio_time_source = RadioTimeSource::Create({
        config::kRadioRxPin,
        config::kRadioSpeed,
        config::kRadioTxRate,
        config::kRadioRxStatusLedPin
    });
    if (radio_time_source == nullptr) {
        Serial.println("Failed to create radio time source");
        while (true) {}
    }

    xTaskCreatePinnedToCore(SequenceHandlingTask, "SequenceHandlingTask", 4096, NULL, 1, &sequence_handling_task_handle, 1);    
}

void loop() {
    if (Serial.available() && !serial_mode) {
        if (MOCK_RADIO) {
            auto input = Serial.readStringUntil('\n');
            auto millis = input.toInt();
            radio_time_source->RadioTimeMock(millis);     
        } else {
            Serial.println("Serial input detected, switching to manual mode");
            
            vTaskDelete(sequence_handling_task_handle);
            xTaskCreatePinnedToCore(SequenceHandlingTaskFromSerial, "SequenceHandlingTaskFromSerial", 
                                    4096, NULL, 1, &sequence_handling_task_handle, 1);

            serial_mode = true;
        }
    }

    if (!serial_mode) {

        radio_time_source->Sync();

        Serial.printf("TimeSyncHandlingTask: sequence_millis == %lu\n", radio_time_source->GetReferenceMillis() + millis());

        const unsigned long reference_millis = radio_time_source->GetReferenceMillis();
        xQueueOverwrite(radio_time_queue_handle, &reference_millis);

        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        if (!xWasDelayed) {
            Serial.println("[WARNING] TimeSyncHandlingTask was NOT delayed!");
        }
    }
}