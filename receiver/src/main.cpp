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

    Chase::Config config{
        .interval = 40 * 3,
        .color_handling = Chase::ColorHandling::kGradientThruEffect,
        .minimum_brightnes = 50,
        .direction = {
            {0.0, 1.0},
            // {0.5, 0.5},
            {1.0, 0.0},
        },
        .pulse_overlap = 10,
        .color = {
            .colorGradient = {
                .alpha_points = {},
                .color_points = {
                    {0.5, 0.0, {255, 0, 0}},
                    {0.5, 1.0, {0, 255, 0}},
                },
            },
            .brightness = {
                {0.0, 0.1},
                {0.5, 1.0},
                {1.0, 1.0},
            },
        }, 
    };
    auto effect = Chase(config, led_controller->leds_, config::kNumLeds[0]);

    TickType_t xLastWakeTime;
    unsigned long reference_millis = 0;
    constexpr double period_millis = 1000.0 / config::kUpdateFrequency;
    while (true) {

        // xQueueReceive(radio_time_queue_handle, &reference_millis, 0);
        // const unsigned long radio_sequence_time_millis = reference_millis + millis();

        // unsigned long local_sequence_time_millis = led_controller->Step() * period_millis;
        // if (radio_sequence_time_millis > local_sequence_time_millis + config::kAllowedFrameDifference * period_millis) {
        //     while (radio_sequence_time_millis > local_sequence_time_millis + config::kAllowedFrameDifference * period_millis) {
        //         led_controller->StepSequence(false);
        //         local_sequence_time_millis = led_controller->Step() * period_millis;
        //     }
        // } else if (local_sequence_time_millis > radio_sequence_time_millis + config::kAllowedFrameDifference * period_millis) {
        //     led_controller->SeekToStep(radio_sequence_time_millis / period_millis);
        // }

        // led_controller->StepSequence(true);
    
        unsigned long time = millis();

        effect.update();
        FastLED.show();

        unsigned time_to_sleep = period_millis - (millis() - time);
        if (time_to_sleep > 0) {
            delay(time_to_sleep);
        } else {
            Serial.println("SequenceHandlingTask was NOT delayed!");
        }

        // BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        // if (!xWasDelayed) {
        //     Serial.println("[WARNING] SequenceHandlingTask was NOT delayed!");
        //     Serial.println("xLastWakeTime: " + String(xLastWakeTime));
        //     Serial.println(config::kUpdateFrequency);
        // }
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
    // Serial.setRxBufferSize(4096);
    // Serial.setTimeout(100000);
    Serial.begin(115200);

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
        }

        radio_time_source->Sync();

        Serial.printf("TimeSyncHandlingTask: sequence_millis == %lu\n", radio_time_source->GetReferenceMillis() + millis());

        const unsigned long reference_millis = radio_time_source->GetReferenceMillis();
        xQueueOverwrite(radio_time_queue_handle, &reference_millis);

        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));
        if (!xWasDelayed) {
            //Serial.println("[WARNING] TimeSyncHandlingTask was NOT delayed!");
        }
    }
}