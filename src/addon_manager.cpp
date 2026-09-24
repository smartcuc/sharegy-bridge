#include "addon_manager.h"
#include "config.h"
#include "inputs.h"
#include <Preferences.h>

AddonManager addonMgr;
static Preferences prefs;

void AddonManager::init() {
    prefs.begin("sharegy_addon", true);
    cfg.profile = (AddonProfileType)prefs.getInt("profile", ADDON_NONE);
    cfg.protocol = (ModbusProtocolType)prefs.getInt("proto", PROTO_RTU_RS485);
    cfg.slaveId = (uint8_t)prefs.getInt("slave_id", 1);
    cfg.baudRate = (uint32_t)prefs.getInt("baud", 9600);
    cfg.tcpPort = (uint16_t)prefs.getInt("tcp_port", 502);
    cfg.s0ImpPerKwh = (uint16_t)prefs.getInt("s0_imp", 1000);
    cfg.enabled = prefs.getBool("enabled", false);
    
    String host = prefs.getString("tcp_host", "");
    strncpy(cfg.tcpHost, host.c_str(), sizeof(cfg.tcpHost) - 1);
    prefs.end();

    if (cfg.enabled && cfg.protocol == PROTO_RTU_RS485) {
        Serial2.begin(cfg.baudRate, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
        pinMode(PIN_RS485_DE, OUTPUT);
        digitalWrite(PIN_RS485_DE, LOW);
        Serial.printf("[ADDON] RS485 initialized at %u Baud (Slave: %u, Profile: %d)\n", cfg.baudRate, cfg.slaveId, (int)cfg.profile);
    }
}

void AddonManager::setProfile(const AddonConfig& newConfig) {
    cfg = newConfig;

    prefs.begin("sharegy_addon", false);
    prefs.putInt("profile", (int)cfg.profile);
    prefs.putInt("proto", (int)cfg.protocol);
    prefs.putInt("slave_id", (int)cfg.slaveId);
    prefs.putInt("baud", (int)cfg.baudRate);
    prefs.putString("tcp_host", cfg.tcpHost);
    prefs.putInt("tcp_port", (int)cfg.tcpPort);
    prefs.putInt("s0_imp", (int)cfg.s0ImpPerKwh);
    prefs.putBool("enabled", cfg.enabled);
    prefs.end();

    if (cfg.enabled && cfg.protocol == PROTO_RTU_RS485) {
        Serial2.end();
        Serial2.begin(cfg.baudRate, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
        pinMode(PIN_RS485_DE, OUTPUT);
        digitalWrite(PIN_RS485_DE, LOW);
    }
    Serial.printf("[ADDON] Profile updated: %d, Protocol: %d, Enabled: %s\n", (int)cfg.profile, (int)cfg.protocol, cfg.enabled ? "YES" : "NO");
}

void AddonManager::loop() {
    if (!cfg.enabled || cfg.profile == ADDON_NONE) return;

    if (millis() - lastPollMs >= 1500) {
        lastPollMs = millis();
        if (cfg.profile == ADDON_S0_METER) {
            pollS0PulseMeter();
        } else if (cfg.protocol == PROTO_RTU_RS485) {
            pollModbusRtu();
        } else if (cfg.protocol == PROTO_TCP) {
            pollModbusTcp();
        }
    }
}

void AddonManager::pollModbusRtu() {
    // Simulated / standard RS485 register read hook
    // Real implementation reads holding/input registers depending on profile
    data.isOnline = true;
    data.lastReadMs = millis();
}

void AddonManager::pollModbusTcp() {
    // Modbus-TCP client connection handling
    data.isOnline = true;
    data.lastReadMs = millis();
}

void AddonManager::pollS0PulseMeter() {
    GridControlSignals grid = inputs.getGridStatus();
    uint32_t pulses = grid.s0PulseCount;
    if (cfg.s0ImpPerKwh > 0) {
        data.totalEnergyKwh = (float)pulses / (float)cfg.s0ImpPerKwh;
    }
    data.isOnline = true;
    data.lastReadMs = millis();
}

void AddonManager::toJson(JsonObject& obj) const {
    obj["enabled"] = cfg.enabled;
    obj["profile_id"] = (int)cfg.profile;
    obj["protocol_id"] = (int)cfg.protocol;
    obj["slave_id"] = cfg.slaveId;
    obj["baud_rate"] = cfg.baudRate;
    obj["tcp_host"] = cfg.tcpHost;
    obj["tcp_port"] = cfg.tcpPort;
    obj["s0_imp_per_kwh"] = cfg.s0ImpPerKwh;

    JsonObject val = obj["data"].to<JsonObject>();
    val["online"] = data.isOnline;
    val["grid_power_w"] = data.gridPowerW;
    val["pv_power_w"] = data.pvPowerW;
    val["battery_power_w"] = data.batteryPowerW;
    val["battery_soc_pct"] = data.batterySocPct;
    val["total_energy_kwh"] = data.totalEnergyKwh;
}
