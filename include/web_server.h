#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>

class DeviceWebServer {
public:
    void init(bool isApMode = false);
    void handleClient();

private:
    WebServer server{80};
    DNSServer dnsServer;
    bool isCaptivePortal = false;

    // Authentication settings
    bool authEnabled = true;
    String authUser = "admin";
    String authPass = "sharegy!26B";

    bool checkAuth();
    void setupRoutes();
    void handleRoot();
    void handleApiStatus();
    void handleApiRelay();
    void handleApiSgReady();
    void handleApiRpc();
    void handleApiConfig();
    void handleApiSystem();
    void handleApiMqtt();
    void handleApiAddon();
    void handleApiWifiScan();
    void handleApiWifiTest();
    void handleNotFound();
};

extern DeviceWebServer webServer;
