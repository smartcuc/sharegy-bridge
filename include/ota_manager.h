#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>

class OtaManager {
public:
    void init(WebServer* webServer);
    bool performRemoteOta(const String& binaryUrl, const String& expectedMd5 = "");

private:
    WebServer* server = nullptr;
};

extern OtaManager ota;
