#include "radio_time_source.hpp"

#include <RH_ASK.h>

#include "config.h"

RadioTimeSource* RadioTimeSource::Create(const Config &config) {
    RadioTimeSource* radio_time_source = new RadioTimeSource();
    radio_time_source->receiver = new RH_ASK(config.speed, config.rx_pin, config.rx_pin);
    if (!radio_time_source->receiver->init()) {
        return nullptr;
    }
    radio_time_source->receiver->setModeRx();

    radio_time_source->config_ = config;
    radio_time_source->RadioTimeMock(0);

    return radio_time_source;
}

void RadioTimeSource::Sync() {

    receiver->waitAvailableTimeout(500);

    uint8_t buf[4];
    uint8_t len = sizeof(buf);
    if (!receiver->recv(buf, &len)) {
        Serial.println("Failed to receive data");
        return;
    }
    if (len != 4) {
        Serial.println("Received data of unexpected length");
        return;
    }

    unsigned long server_millis = *((uint32_t *)&buf);
    reference_millis_ = millis() - server_millis;
}

void RadioTimeSource::RadioTimeMock(unsigned long sequence_millis) {
    reference_millis_ = millis() - sequence_millis;
}