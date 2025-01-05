#include "radio_time_source.hpp"

#include <RH_ASK.h>

RadioTimeSource* RadioTimeSource::Create(const Config &config) {
    RadioTimeSource* radio_time_source = new RadioTimeSource();
    radio_time_source->receiver = new RH_ASK(config.speed, config.rx_pin, config.rx_pin);
    if (!radio_time_source->receiver->init()) {
        return nullptr;
    }
    radio_time_source->receiver->setModeRx();

    return radio_time_source;
}

void RadioTimeSource::Sync() {
    if (!receiver->available()) {
        return;
    }

    uint8_t buf[4];
    uint8_t len = sizeof(buf);
    if (!receiver->recv(buf, &len)) {
        return;
    }
    if (len != 4) {
        return;
    }

    unsigned long server_millis = *((uint32_t *)&buf);
    reference_micros_ = server_millis * 1000 - micros(); 
}