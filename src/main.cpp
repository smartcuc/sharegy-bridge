#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "config.h"
#include "relays.h"
#include "inputs.h"
#include "rpc_dispatcher.h"
#include "web_server.h"
#include "local_mqtt.h"
#include "addon_manager.h"

// Hardware Device Unique ID & Hostname
char deviceId[32] = {0};
char hostName[32] = {0};
bool ethConnected = false;
String ethIp = "";
bool wifiRadioEnabled = true;

unsigned long lastTelemetryMs = 0;
unsigned long lastCloudHeartbeatMs = 0;
bool failSafeActive = false;
static Preferences prefs;

void setupUniqueId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(deviceId, sizeof(deviceId), "WS-ESP32S3-%02X%02X%02X", mac[3], mac[4], mac[5]);

    prefs.begin("sharegy_cfg", true);
    String customHost = prefs.getString("host_name", "");
    prefs.end();

    if (customHost.length() > 0) {
        strncpy(hostName, customHost.c_str(), sizeof(hostName) - 1);
    } else {
        snprintf(hostName, sizeof(hostName), "sharegy-bridge-%02x%02x%02x", mac[3], mac[4], mac[5]);
    }
}

void setupNetwork() {
    prefs.begin("sharegy_cfg", true);
    wifiRadioEnabled = prefs.getBool("wifi_en", true);
    String ssid = prefs.getString("wifi_ssid", "");
    String pass = prefs.getString("wifi_pass", "");
    prefs.end();

    if (!wifiRadioEnabled) {
        Serial.println("[NET] WiFi radio is disabled in configuration.");
        WiFi.mode(WIFI_OFF);
        webServer.init(false);
        return;
    }

    if (ssid.length() > 0) {
        Serial.printf("[NET] Connecting to WiFi '%s' as host '%s'...\n", ssid.c_str(), hostName);
        WiFi.setHostname(hostName);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 25) {
            delay(400);
            Serial.print(".");
            attempts++;
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[NET] WiFi Connected! IP: %s\n", WiFi.localIP().toString().c_str());
            
            // Start mDNS responder: http://<hostName>.local
            if (MDNS.begin(hostName)) {
                MDNS.addService("http", "tcp", 80);
                Serial.printf("[mDNS] Responder started: http://%s.local\n", hostName);
            }

            webServer.init(false);
            localMqtt.init();
            return;
        }
    }

    // Fallback Captive Portal Access Point Mode
    Serial.println("[NET] Starting fallback Access Point & Captive Portal...");
    String apName = "Sharegy-" + String(deviceId);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(apName.c_str(), DEFAULT_WIFI_PASS);
    Serial.printf("[NET] AP Started: '%s' (IP: 192.168.4.1)\n", apName.c_str());
    
    if (MDNS.begin(hostName)) {
        MDNS.addService("http", "tcp", 80);
    }
    webServer.init(true);
}

void processFailsafeWatchdog() {
    // Cloud Watchdog: Fallback after 60s without heartbeat
    if (millis() - lastCloudHeartbeatMs > CLOUD_FAILSAFE_TIMEOUT_MS) {
        if (!failSafeActive) {
            failSafeActive = true;
            Serial.println("[FAIL-SAFE] Cloud timeout! Entering autonomous fallback.");
            relays.setSgReadyMode(SG_NORMAL);
        }
    } else {
        failSafeActive = false;
    }

    // Hard Hardware Priority: § 14a VNB Signals override Cloud
    GridControlSignals grid = inputs.getGridStatus();
    if (grid.signal0Pct) {
        relays.setSgReadyMode(SG_LOCK_0PCT);
        relays.setRelay(2, false); // Wallbox disabled
    } else if (grid.signal30Pct) {
        relays.setRelay(3, false);
        relays.setRelay(4, false);
        relays.setRelay(5, false);
    }
}

void printTelemetryJson() {
    JsonDocument doc;
    doc["device_id"] = deviceId;
    doc["host_name"] = hostName;
    doc["firmware_version"] = BRIDGE_FIRMWARE_VERSION;
    doc["hardware_model"] = BRIDGE_HARDWARE_MODEL;
    doc["uptime_s"] = millis() / 1000;
    doc["failsafe"] = failSafeActive;
    doc["wifi_enabled"] = wifiRadioEnabled;
    doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
    doc["wifi_ip"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "";
    doc["wifi_rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
    doc["eth_connected"] = ethConnected;
    doc["eth_ip"] = ethIp;
    
    // Relay states
    JsonObject r = doc["relays"].to<JsonObject>();
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "ro%d", i + 1);
        r[key] = relays.getRelay(i);
    }

    // Digital Input states
    JsonObject in = doc["inputs"].to<JsonObject>();
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "di%d", i + 1);
        in[key] = inputs.getInput(i);
    }

    GridControlSignals grid = inputs.getGridStatus();
    doc["grid_14a"]["normal_100"] = grid.signal100Pct;
    doc["grid_14a"]["prewarn_60"] = grid.signal60Pct;
    doc["grid_14a"]["dimming_30"] = grid.signal30Pct;
    doc["grid_14a"]["lockout_0"] = grid.signal0Pct;
    doc["s0_pulses"] = grid.s0PulseCount;

    // Addon telemetry
    JsonObject addonObj = doc["addon"].to<JsonObject>();
    addonMgr.toJson(addonObj);

    String output;
    serializeJson(doc, output);
    Serial.println(output);

    // Sync with local MQTT if connected
    localMqtt.publishState();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println("⚡ Sharegy Waveshare ESP32-S3 Hardware Bridge");
    Serial.println("==========================================");

    setupUniqueId();
    Serial.printf("[INIT] Device ID: %s | Hostname: %s\n", deviceId, hostName);

    // Initialize IOs
    relays.init();
    inputs.init();
    addonMgr.init();
    Serial.println("[INIT] Relays, Inputs & Addon Manager initialized.");

    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, HIGH);

    setupNetwork();

    lastCloudHeartbeatMs = millis();
}

void loop() {
    inputs.update();
    addonMgr.loop();
    processFailsafeWatchdog();
    webServer.handleClient();
    localMqtt.loop();

    // Fast event reporting if inputs changed
    if (inputs.hasInputChanged()) {
        Serial.println("[EVENT] Digital Input state changed!");
        printTelemetryJson();
    }

    // Periodic telemetry
    if (millis() - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
        lastTelemetryMs = millis();
        printTelemetryJson();
        digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
    }

    // Handle incoming serial JSON-RPC commands
    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.startsWith("{")) {
            String rpcResp = rpc.handleJsonRpc(line);
            Serial.println(rpcResp);
            lastCloudHeartbeatMs = millis();
        } else if (line == "PING") {
            lastCloudHeartbeatMs = millis();
            Serial.println("PONG");
        }
    }

    delay(5);
}
