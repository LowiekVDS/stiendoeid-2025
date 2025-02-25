#include <FastLED.h>

#include "LittleFS.h"

#include "led_controller.hpp"
#include "config.hpp"
#include "radio_time_source.hpp"

#include "effects/all.hpp"

TaskHandle_t sequence_handling_task_handle;
TaskHandle_t time_sync_handling_task_handle;

QueueHandle_t radio_time_queue_handle;

RadioTimeSource* radio_time_source = nullptr;
TickType_t xLastWakeTime;

LedController* led_controller = nullptr;

using namespace effects;

void SequenceHandlingTask(void *params) {

    TickType_t xLastWakeTime = xTaskGetTickCount();
    // At this local time the server time is zero
    unsigned long reference_millis = millis();
    constexpr double period_millis = 1000.0 / config::kUpdateFrequency;
    while (true) {

        if (xQueueReceive(radio_time_queue_handle, &reference_millis, 0) == pdTRUE) {
            // Serial.println("Received new reference millis: " + String(reference_millis));
        }

        // Radio sequence time is the sequence time on the server. So
        const unsigned long radio_sequence_time_millis = millis() - reference_millis;
        const unsigned long radio_sequence_time_steps = 
            static_cast<uint32_t>(radio_sequence_time_millis / period_millis) % led_controller->MaxSteps();

        unsigned long local_sequence_time_step = led_controller->Step();

        if (radio_sequence_time_steps > local_sequence_time_step + config::kAllowedFrameDifference) {
            while (radio_sequence_time_millis > local_sequence_time_step + config::kAllowedFrameDifference) {
                led_controller->StepSequence(false);
                local_sequence_time_step = led_controller->Step() * period_millis;
            }
        } else if (local_sequence_time_step > radio_sequence_time_steps + config::kAllowedFrameDifference) {
            led_controller->SeekToStep(radio_sequence_time_steps);
        }

        led_controller->StepSequence(true);

        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        if (!xWasDelayed) {
            // Serial.println("[WARNING] SequenceHandlingTask was NOT delayed!");
            // Serial.println("xLastWakeTime: " + String(xLastWakeTime));
            // Serial.println(config::kUpdateFrequency);
        }
    }
}

void SequenceHandlingTaskFromSerial(void *params) {
    const int kNumChannels = config::kNumChannelsPerLed * config::kTotalNumLeds;
    uint8_t buffer[kNumChannels];
    while (true) {
        Serial.readBytes(buffer, kNumChannels);    
        if (!led_controller->SetLedsFromBuffer(buffer, kNumChannels)) {
            Serial.println("Failed to set leds from buffer");
        }
    }
}

void setup() {
    Serial.setRxBufferSize(4096);
    Serial.setTimeout(100000);
    Serial.begin(921600);

    delay(2000);

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

    led_controller = LedController::Create({
        config::kTotalNumLeds, 
        config::kCompressedSequenceFileLocation
    });
    if (led_controller == nullptr) {
        Serial.println("Failed to create led controller");
        return;
    }

    if (SERIAL_MODE) {
        Serial.println("Starting SequenceHandlingTaskFromSerial");
        xTaskCreatePinnedToCore(SequenceHandlingTaskFromSerial, "SequenceHandlingTaskFromSerial", 
                                        4096, NULL, 1, &sequence_handling_task_handle, 1);
    } else {
        xTaskCreatePinnedToCore(SequenceHandlingTask, "SequenceHandlingTask", 4096, NULL, 1, &sequence_handling_task_handle, 1);
    }
}

void loop() {
    if (!SERIAL_MODE) {
        if (MOCK_RADIO && Serial.available()) {
            auto input = Serial.readStringUntil('\n');
            auto millis = input.toInt();
            radio_time_source->RadioTimeMock(millis);     
        } else if (!MOCK_RADIO) {
            radio_time_source->Sync();
        }

        Serial.printf("TimeSyncHandlingTask: sequence_millis == %lu\n", radio_time_source->GetReferenceMillis());

        const unsigned long reference_millis = radio_time_source->GetReferenceMillis();
        xQueueOverwrite(radio_time_queue_handle, &reference_millis);

        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        if (!xWasDelayed) {
            Serial.println("[WARNING] TimeSyncHandlingTask was NOT delayed!");
        }
    }
}