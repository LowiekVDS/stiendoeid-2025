#include "led_controller.hpp"
#include "config.hpp"

LedController* led_controller;

void SequenceHandlingTask(void *params) {
    // TODO: handle timing changes
    TickType_t xLastWakeTime;
    while (true) {
        led_controller->StepSequence();
        led_controller->UpdateLeds();
        BaseType_t xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000.0 / config::kUpdateFrequency));

        if (!xWasDelayed) {
            Serial.println("[WARNING] Task was NOT delayed!");
        }
    }
}

void setup() {
    Serial.begin(115200);

    delay(1000);

    bool success = false;
    led_controller = LedController::Create({config::kCompressedSequenceFileLocation});
    if (led_controller == nullptr) {
        Serial.println("Failed to create led controller");
        while (true) {}
    }

    Serial.println("Led controller created");
    xTaskCreatePinnedToCore(SequenceHandlingTask, "SequenceHandlingTask", 4096, NULL, 1, NULL, 0);
    Serial.println("Task created");
}

void loop() {}