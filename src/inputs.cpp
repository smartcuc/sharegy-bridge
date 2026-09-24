#include "inputs.h"
#include "config.h"

InputManager inputs;

void InputManager::init() {
    for (int i = 0; i < 8; i++) {
        pinMode(inputPins[i], INPUT_PULLUP);
        // Optocoupler inputs are active-low when pulled to GND or active-high with polarity
        inputStates[i] = (digitalRead(inputPins[i]) == LOW);
        lastInputStates[i] = inputStates[i];
    }
}

void InputManager::update() {
    stateChanged = false;
    for (int i = 0; i < 8; i++) {
        bool reading = (digitalRead(inputPins[i]) == LOW);
        if (reading != lastInputStates[i]) {
            lastDebounceTime[i] = millis();
        }

        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
            if (reading != inputStates[i]) {
                inputStates[i] = reading;
                stateChanged = true;

                // S0 Pulse Detection on DI5 (rising edge of pulse)
                if (i == 4 && inputStates[i]) {
                    s0Counter++;
                }
            }
        }
        lastInputStates[i] = reading;
    }
}

bool InputManager::getInput(uint8_t inputIndex) {
    if (inputIndex >= 8) return false;
    return inputStates[inputIndex];
}

uint8_t InputManager::getInputBitmask() {
    uint8_t mask = 0;
    for (int i = 0; i < 8; i++) {
        if (inputStates[i]) {
            mask |= (1 << i);
        }
    }
    return mask;
}

GridControlSignals InputManager::getGridStatus() {
    GridControlSignals g;
    g.signal100Pct = inputStates[0];
    g.signal60Pct = inputStates[1];
    g.signal30Pct = inputStates[2];
    g.signal0Pct = inputStates[3];
    g.s0PulseCount = s0Counter;
    return g;
}

bool InputManager::hasInputChanged() {
    return stateChanged;
}
