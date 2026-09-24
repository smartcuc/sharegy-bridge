#pragma once

#include <Arduino.h>

struct GridControlSignals {
    bool signal100Pct;   // DI1 (Normal operation)
    bool signal60Pct;    // DI2 (Pre-warning)
    bool signal30Pct;    // DI3 (Dimming / Notbremse)
    bool signal0Pct;     // DI4 (Grid shutdown)
    uint32_t s0PulseCount; // DI5 S0 counter
};

class InputManager {
public:
    void init();
    void update();
    bool getInput(uint8_t inputIndex);
    uint8_t getInputBitmask();
    GridControlSignals getGridStatus();
    bool hasInputChanged();

private:
    const uint8_t inputPins[8] = {4, 5, 6, 7, 15, 16, 17, 18};
    bool inputStates[8] = {false};
    bool lastInputStates[8] = {false};
    uint32_t s0Counter = 0;
    unsigned long lastDebounceTime[8] = {0};
    const unsigned long debounceDelay = 30; // 30ms hardware debounce
    bool stateChanged = false;
};

extern InputManager inputs;
