#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

class LocalMqttClient {
public:
    void init();
    void loop();
    void publishState();
    bool isConnected() const;

private:
    WiFiClient plainClient;
    WiFiClientSecure secureClient;
    PubSubClient mqttClient;
    unsigned long lastReconnectAttempt = 0;
    
    String mqttBroker = "";
    uint16_t mqttPort = 1883;
    bool useTls = false;
    String mqttUser = "";
    String mqttPass = "";
    String topicPrefix = "sharegy";

    void reconnect();
    void onMessage(char* topic, byte* payload, unsigned int length);
};

extern LocalMqttClient localMqtt;
