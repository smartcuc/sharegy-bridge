#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

enum ModbusProtocolType {
    PROTO_NONE = 0,
    PROTO_RTU_RS485 = 1,
    PROTO_TCP = 2
};

enum AddonProfileType {
    ADDON_NONE = 0,
    ADDON_SDM630 = 1,      // Eastron SDM630 / SDM72 / SDM120 Modbus-RTU
    ADDON_SUNGROW = 2,     // Sungrow Inverter (RTU or TCP)
    ADDON_FRONIUS = 3,     // Fronius Inverter / Smart Meter SunSpec (RTU or TCP)
    ADDON_DEYE = 4,        // Deye / SunSynk Hybrid Inverter Modbus-RTU
    ADDON_SMA = 5,         // SMA Sunny Tripower / Boy Modbus-TCP
    ADDON_VICTRON = 6,     // Victron Energy GX Modbus-TCP
    ADDON_SG_READY = 7,    // SG-Ready Heat Pump Relay Controller
    ADDON_S0_METER = 8     // S0 Pulse Energy & Power Meter
};

struct AddonConfig {
    AddonProfileType profile = ADDON_NONE;
    ModbusProtocolType protocol = PROTO_RTU_RS485;
    uint8_t slaveId = 1;
    uint32_t baudRate = 9600;
    char tcpHost[64] = {0};
    uint16_t tcpPort = 502;
    uint16_t s0ImpPerKwh = 1000;
    bool enabled = false;
};

struct InverterData {
    float gridPowerW = 0.0f;
    float pvPowerW = 0.0f;
    float batteryPowerW = 0.0f;
    float batterySocPct = 0.0f;
    float totalEnergyKwh = 0.0f;
    bool isOnline = false;
    unsigned long lastReadMs = 0;
};

class AddonManager {
public:
    void init();
    void loop();
    void setProfile(const AddonConfig& config);
    AddonConfig getConfig() const { return cfg; }
    InverterData getData() const { return data; }
    void toJson(JsonObject& obj) const;

private:
    AddonConfig cfg;
    InverterData data;
    unsigned long lastPollMs = 0;

    void pollModbusRtu();
    void pollModbusTcp();
    void pollS0PulseMeter();
};

extern AddonManager addonMgr;
