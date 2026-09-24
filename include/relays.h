#pragma once

#include <Arduino.h>

enum SgReadyState {
    SG_LOCK_0PCT = 1,       // Operating state 1: Compressor locked
    SG_NORMAL = 2,          // Operating state 2: Standard auto
    SG_PV_BOOST = 3,        // Operating state 3: Heat pump increased temp
    SG_MAX_POWER = 4        // Operating state 4: Hard heat pump demand
};

class RelayManager {
public:
    void init();
    void setRelay(uint8_t relayIndex, bool state);
    bool getRelay(uint8_t relayIndex);
    void setSgReadyMode(SgReadyState state);
    void setAll(bool state);
    uint8_t getRelayBitmask();

private:
    bool relayStates[8] = {false, false, false, false, false, false, false, false};
    const uint8_t relayPins[8] = {1, 2, 41, 42, 45, 46, 47, 48};
};

extern RelayManager relays;
