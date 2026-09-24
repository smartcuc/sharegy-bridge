#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class RpcDispatcher {
public:
    String handleJsonRpc(const String& jsonRequest);

private:
    String createSuccessResponse(const JsonVariantConst& id, const JsonDocument& result);
    String createErrorResponse(const JsonVariantConst& id, int code, const char* message);
};

extern RpcDispatcher rpc;
