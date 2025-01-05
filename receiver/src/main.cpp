#include "led_controller.hpp"
#include "config.hpp"
#include "radio_time_source.hpp"

bool serial_mode = false;

TaskHandle_t sequence_handling_task_handle;
TaskHandle_t time_sync_handling_task_handle;

void SequenceHandlingTask(void *params) {
    // TODO: handle timing changes
    auto led_controller = LedController::Create({
        config::kNumLeds, 
        config::kCompressedSequenceFileLocation
    });
    if (led_controller == nullptr) {
        Serial.println("Failed to create led controller");
        return;
    }

    TickType_t xLastWakeTime;
    while (true) {
        led_controller->StepSequence(true);
        
        Serial.printf("SequenceHandlingTask: micros == %lu\n", micros());

        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        if (!xWasDelayed) {
            Serial.println("[WARNING] SequenceHandlingTask was NOT delayed!");
        }
    }
}

void TimeSyncHandlingTask(void *params) {
    auto radio_time_source = RadioTimeSource::Create({
        config::kRadioRxPin,
        config::kRadioSpeed,
        config::kRadioTxRate
    });
    if (radio_time_source == nullptr) {
        Serial.println("Failed to create radio time source");
        while (true) {}
    }

    TickType_t xLastWakeTime;
    while (true) {
        radio_time_source->Sync();

        Serial.printf("TimeSyncHandlingTask: micros == %lu\n", micros());

        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        if (!xWasDelayed) {
            Serial.println("[WARNING] TimeSyncHandlingTask was NOT delayed!");
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

    xTaskCreatePinnedToCore(SequenceHandlingTask, "SequenceHandlingTask", 4096, NULL, 1, &sequence_handling_task_handle, 1);
    xTaskCreatePinnedToCore(TimeSyncHandlingTask, "TimeSyncHandlingTask", 4096, NULL, 1, &time_sync_handling_task_handle, 0);

    Serial.println("Tasks created and system running!");
}

void loop() {
    if (Serial.available() && !serial_mode) {
        Serial.println("Serial input detected, switching to manual mode");
        
        vTaskDelete(sequence_handling_task_handle);
        vTaskDelete(time_sync_handling_task_handle);
        xTaskCreatePinnedToCore(SequenceHandlingTaskFromSerial, "SequenceHandlingTaskFromSerial", 
                                4096, NULL, 1, &sequence_handling_task_handle, 1);

        serial_mode = true;
    }
}