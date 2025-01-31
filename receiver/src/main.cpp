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
        config::kNumLeds, 
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
    CRGB leds[config::kNumLeds];
    FastLED.addLeds<SK6812, config::kDataPin, GRB>(leds, config::kNumLeds);  

    auto kNumChannels = config::kNumChannelsPerLed * config::kNumLeds;
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
    }
}

void setup() {
    Serial.begin(115200);

    radio_time_queue_handle = xQueueCreate(1, sizeof(unsigned long));
    radio_time_source = RadioTimeSource::Create({
        config::kRadioRxPin,
        config::kRadioSpeed,
        config::kRadioTxRate
    });
    if (radio_time_source == nullptr) {
        Serial.println("Failed to create radio time source");
        while (true) {}
    }

    xTaskCreatePinnedToCore(SequenceHandlingTask, "SequenceHandlingTask", 4096, NULL, 1, &sequence_handling_task_handle, 1);    
}

void loop() {
    if (Serial.available() && !serial_mode) {
        // Serial.println("Serial input detected, switching to manual mode");
        
        // vTaskDelete(sequence_handling_task_handle);
        // vTaskDelete(time_sync_handling_task_handle);
        // xTaskCreatePinnedToCore(SequenceHandlingTaskFromSerial, "SequenceHandlingTaskFromSerial", 
                                // 4096, NULL, 1, &sequence_handling_task_handle, 1);

        // serial_mode = true;

        // Read millis from chars
        auto input = Serial.readStringUntil('\n');
        auto millis = input.toInt();
        radio_time_source->RadioTimeMock(millis);     
    }

    radio_time_source->Sync();

    Serial.printf("TimeSyncHandlingTask: sequence_millis == %lu\n", radio_time_source->GetReferenceMillis() + millis());

    const unsigned long reference_millis = radio_time_source->GetReferenceMillis();
    xQueueOverwrite(radio_time_queue_handle, &reference_millis);

    delay(1000);

    BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
    if (!xWasDelayed) {
        Serial.println("[WARNING] TimeSyncHandlingTask was NOT delayed!");
    }
}