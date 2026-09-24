#include "relays.h"
#include "config.h"

RelayManager relays;

void RelayManager::init() {
    for (int i = 0; i < 8; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW);
        relayStates[i] = false;
    }
}

void RelayManager::setRelay(uint8_t relayIndex, bool state) {
    if (relayIndex >= 8) return;
    relayStates[relayIndex] = state;
    digitalWrite(relayPins[relayIndex], state ? HIGH : LOW);
}

bool RelayManager::getRelay(uint8_t relayIndex) {
    if (relayIndex >= 8) return false;
    return relayStates[relayIndex];
}

void RelayManager::setSgReadyMode(SgReadyState state) {
    switch (state) {
        case SG_LOCK_0PCT:
            // RO1 = Closed (1), RO2 = Open (0)
            setRelay(0, true);
            setRelay(1, false);
            break;
        case SG_NORMAL:
            // RO1 = Open (0), RO2 = Open (0)
            setRelay(0, false);
            setRelay(1, false);
            break;
        case SG_PV_BOOST:
            // RO1 = Open (0), RO2 = Closed (1)
            setRelay(0, false);
            setRelay(1, true);
            break;
        case SG_MAX_POWER:
            // RO1 = Closed (1), RO2 = Closed (1)
            setRelay(0, true);
            setRelay(1, true);
            break;
    }
}

void RelayManager::setAll(bool state) {
    for (int i = 0; i < 8; i++) {
        setRelay(i, state);
    }
}

uint8_t RelayManager::getRelayBitmask() {
    uint8_t mask = 0;
    for (int i = 0; i < 8; i++) {
        if (relayStates[i]) {
            mask |= (1 << i);
        }
    }
    return mask;
}
