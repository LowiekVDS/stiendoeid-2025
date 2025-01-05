#pragma once

#include <RH_ASK.h>

class RadioTimeSource {

  public:

    struct Config {
        int rx_pin;
        int speed;
        float tx_rate;
    };

    static RadioTimeSource* Create(const Config &config);
    void Sync();
    unsigned long GetMicros() const { return micros() + reference_micros_; }

  private:
    RadioTimeSource() = default;

    unsigned long reference_micros_ = 0;
    RH_ASK* receiver = nullptr;
};