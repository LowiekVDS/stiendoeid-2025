#include "led_controller.hpp"
#include "config.hpp"
#include "radio_time_source.hpp"

LedController* led_controller;
RadioTimeSource* radio_time_source;

void SequenceHandlingTask(void *params) {
    // TODO: handle timing changes
    TickType_t xLastWakeTime;
    while (true) {
        led_controller->StepSequence();
        led_controller->UpdateLeds();
        
        Serial.printf("SequenceHandlingTask: micros == %lu\n", micros());

        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        if (!xWasDelayed) {
            Serial.println("[WARNING] SequenceHandlingTask was NOT delayed!");
        }
    }
}

void TimeSyncHandlingTask(void *params) {
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

void setup() {
    Serial.begin(115200);

    led_controller = LedController::Create({
        config::kNumLeds, 
        config::kCompressedSequenceFileLocation
    });
    if (led_controller == nullptr) {
        Serial.println("Failed to create led controller");
        while (true) {}
    }
    
    radio_time_source = RadioTimeSource::Create({
        config::kRadioRxPin,
        config::kRadioSpeed,
        config::kRadioTxRate
    });
    if (radio_time_source == nullptr) {
        Serial.println("Failed to create radio time source");
        while (true) {}
    }

    xTaskCreatePinnedToCore(SequenceHandlingTask, "SequenceHandlingTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TimeSyncHandlingTask, "TimeSyncHandlingTask", 4096, NULL, 1, NULL, 0);

    Serial.println("Tasks created and system running!");
}

void loop() {}