#include "local_mqtt.h"
#include "relays.h"
#include "inputs.h"
#include "config.h"
#include <Preferences.h>
#include <ArduinoJson.h>

LocalMqttClient localMqtt;
static Preferences prefs;

void LocalMqttClient::init() {
    prefs.begin("sharegy_cfg", true);
    mqttBroker = prefs.getString("mqtt_host", "");
    mqttPort = (uint16_t)prefs.getInt("mqtt_port", 1883);
    useTls = prefs.getBool("mqtt_tls", false);
    mqttUser = prefs.getString("mqtt_user", "");
    mqttPass = prefs.getString("mqtt_pass", "");
    topicPrefix = prefs.getString("mqtt_prefix", "sharegy");
    prefs.end();

    if (mqttBroker.length() == 0) {
        Serial.println("[LOCAL MQTT] No local MQTT broker configured. Disabled.");
        return;
    }

    if (useTls) {
        secureClient.setInsecure(); // Allow local or self-signed certs
        mqttClient.setClient(secureClient);
        if (mqttPort == 1883) mqttPort = 8883;
    } else {
        mqttClient.setClient(plainClient);
    }

    mqttClient.setServer(mqttBroker.c_str(), mqttPort);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->onMessage(topic, payload, length);
    });

    Serial.printf("[LOCAL MQTT] Configured for broker: %s:%u (TLS: %s, Prefix: %s)\n", 
                  mqttBroker.c_str(), mqttPort, useTls ? "YES" : "NO", topicPrefix.c_str());
}

bool LocalMqttClient::isConnected() const {
    return const_cast<PubSubClient&>(mqttClient).connected();
}

void LocalMqttClient::loop() {
    if (mqttBroker.length() == 0) return;

    if (!mqttClient.connected()) {
        reconnect();
    } else {
        mqttClient.loop();
    }
}

void LocalMqttClient::reconnect() {
    if (mqttBroker.length() == 0) return;
    if (mqttClient.connected()) return;

    if (millis() - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = millis();
        String clientId = "Sharegy-" + String(deviceId);
        
        bool success = false;
        if (mqttUser.length() > 0) {
            success = mqttClient.connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str());
        } else {
            success = mqttClient.connect(clientId.c_str());
        }

        if (success) {
            Serial.println("[LOCAL MQTT] Connected to local broker!");
            
            // Subscribe to relay control topics: <prefix>/ro1/set .. <prefix>/ro8/set
            for (int i = 1; i <= 8; i++) {
                String topic = topicPrefix + "/ro" + String(i) + "/set";
                mqttClient.subscribe(topic.c_str());
            }
            String sgTopic = topicPrefix + "/sg_ready/set";
            mqttClient.subscribe(sgTopic.c_str());

            publishState();
        }
    }
}

void LocalMqttClient::publishState() {
    if (!mqttClient.connected()) return;

    // Publish individual relay states
    for (int i = 1; i <= 8; i++) {
        String topic = topicPrefix + "/ro" + String(i) + "/state";
        mqttClient.publish(topic.c_str(), relays.getRelay(i - 1) ? "ON" : "OFF");
    }

    // Publish telemetry JSON state
    JsonDocument doc;
    doc["device_id"] = deviceId;
    doc["uptime_s"] = millis() / 1000;
    
    JsonObject r = doc["relays"].to<JsonObject>();
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "ro%d", i + 1);
        r[key] = relays.getRelay(i);
    }

    JsonObject in = doc["inputs"].to<JsonObject>();
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "di%d", i + 1);
        in[key] = inputs.getInput(i);
    }

    GridControlSignals grid = inputs.getGridStatus();
    doc["s0_pulses"] = grid.s0PulseCount;

    String payload;
    serializeJson(doc, payload);
    String statusTopic = topicPrefix + "/status";
    mqttClient.publish(statusTopic.c_str(), payload.c_str());
}

void LocalMqttClient::onMessage(char* topic, byte* payload, unsigned int length) {
    String msg = "";
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }
    msg.trim();

    String t = String(topic);
    for (int i = 1; i <= 8; i++) {
        if (t == topicPrefix + "/ro" + String(i) + "/set") {
            bool state = (msg == "ON" || msg == "1" || msg == "true");
            relays.setRelay(i - 1, state);
            publishState();
            return;
        }
    }

    if (t == topicPrefix + "/sg_ready/set") {
        int mode = msg.toInt();
        if (mode >= 1 && mode <= 4) {
            relays.setSgReadyMode((SgReadyState)mode);
            publishState();
        }
    }
}
