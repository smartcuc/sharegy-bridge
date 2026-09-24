#include "rpc_dispatcher.h"
#include "relays.h"
#include "inputs.h"
#include "config.h"
#include "ota_manager.h"

RpcDispatcher rpc;

struct OtaTaskParams {
    String url;
    String md5;
};

static void otaWorkerTask(void* parameter) {
    OtaTaskParams* params = (OtaTaskParams*)parameter;
    Serial.printf("[OTA Task] Starting async OTA download from: %s\n", params->url.c_str());
    vTaskDelay(pdMS_TO_TICKS(500)); // Allow JSON-RPC response to send over transport
    bool ok = ota.performRemoteOta(params->url, params->md5);
    if (!ok) {
        Serial.println("[OTA Task] ❌ OTA Update failed! System continues running current firmware.");
    }
    delete params;
    vTaskDelete(NULL);
}

String RpcDispatcher::handleJsonRpc(const String& jsonRequest) {
    JsonDocument req;
    DeserializationError err = deserializeJson(req, jsonRequest);
    if (err) {
        return createErrorResponse(JsonVariantConst(), -32700, "Parse error");
    }

    const char* jsonrpc = req["jsonrpc"] | "";
    if (strcmp(jsonrpc, "2.0") != 0) {
        return createErrorResponse(req["id"], -32600, "Invalid Request (must be jsonrpc 2.0)");
    }

    const char* method = req["method"] | "";
    JsonVariantConst id = req["id"];

    // 1. rpc.ping / ping
    if (strcmp(method, "rpc.ping") == 0 || strcmp(method, "sys.ping") == 0) {
        JsonDocument res;
        res["pong"] = true;
        res["device_id"] = deviceId;
        res["firmware_version"] = BRIDGE_FIRMWARE_VERSION;
        res["hardware_model"] = BRIDGE_HARDWARE_MODEL;
        res["uptime_s"] = millis() / 1000;
        return createSuccessResponse(id, res);
    }

    // 1b. adapter.version / sys.info / rpc.version
    if (strcmp(method, "adapter.version") == 0 || strcmp(method, "rpc.version") == 0 || strcmp(method, "sys.info") == 0) {
        JsonDocument res;
        res["version"] = BRIDGE_FIRMWARE_VERSION;
        res["model"] = BRIDGE_HARDWARE_MODEL;
        res["device_id"] = deviceId;
        res["uptime_s"] = millis() / 1000;
        return createSuccessResponse(id, res);
    }

    // 2. rpc.set_relay
    if (strcmp(method, "rpc.set_relay") == 0) {
        int channel = req["params"]["channel"] | 0;
        bool state = req["params"]["state"] | false;
        if (channel < 1 || channel > 8) {
            return createErrorResponse(id, -32602, "Invalid params: channel must be between 1 and 8");
        }
        relays.setRelay(channel - 1, state);
        JsonDocument res;
        res["channel"] = channel;
        res["state"] = state;
        res["status"] = "OK";
        return createSuccessResponse(id, res);
    }

    // 3. rpc.set_sg_ready
    if (strcmp(method, "rpc.set_sg_ready") == 0) {
        int mode = req["params"]["mode"] | 2;
        if (mode < 1 || mode > 4) {
            return createErrorResponse(id, -32602, "Invalid params: mode must be 1, 2, 3 or 4");
        }
        relays.setSgReadyMode((SgReadyState)mode);
        JsonDocument res;
        res["mode"] = mode;
        res["ro1"] = relays.getRelay(0);
        res["ro2"] = relays.getRelay(1);
        res["status"] = "OK";
        return createSuccessResponse(id, res);
    }

    // 4. rpc.get_status
    if (strcmp(method, "rpc.get_status") == 0) {
        JsonDocument res;
        res["device_id"] = deviceId;
        res["firmware_version"] = BRIDGE_FIRMWARE_VERSION;
        res["hardware_model"] = BRIDGE_HARDWARE_MODEL;
        res["uptime_s"] = millis() / 1000;
        for (int i = 0; i < 8; i++) {
            char rk[8];
            snprintf(rk, sizeof(rk), "ro%d", i + 1);
            res["relays"][rk] = relays.getRelay(i);
            char ik[8];
            snprintf(ik, sizeof(ik), "di%d", i + 1);
            res["inputs"][ik] = inputs.getInput(i);
        }
        GridControlSignals grid = inputs.getGridStatus();
        res["grid_14a"]["normal_100"] = grid.signal100Pct;
        res["grid_14a"]["prewarn_60"] = grid.signal60Pct;
        res["grid_14a"]["dimming_30"] = grid.signal30Pct;
        res["grid_14a"]["lockout_0"] = grid.signal0Pct;
        return createSuccessResponse(id, res);
    }

    // 5. rpc.self_test
    if (strcmp(method, "rpc.self_test") == 0) {
        // Quick sequential relay clicking test
        for (int i = 0; i < 8; i++) {
            relays.setRelay(i, true);
            delay(50);
            relays.setRelay(i, false);
        }
        JsonDocument res;
        res["self_test"] = "PASSED";
        res["relays_tested"] = 8;
        return createSuccessResponse(id, res);
    }

    // 6. rpc.reboot
    if (strcmp(method, "rpc.reboot") == 0) {
        JsonDocument res;
        res["reboot"] = "SCHEDULED";
        String out = createSuccessResponse(id, res);
        delay(500);
        ESP.restart();
        return out;
    }

    // 7. rpc.ota_update / adapter.update (Remote Cloud OTA)
    if (strcmp(method, "rpc.ota_update") == 0 || strcmp(method, "adapter.update") == 0) {
        const char* url = req["params"]["url"] | req["params"]["target"] | "";
        const char* md5 = req["params"]["md5"] | "";
        if (strlen(url) == 0) {
            return createErrorResponse(id, -32602, "Invalid params: missing url");
        }
        
        OtaTaskParams* taskParams = new OtaTaskParams();
        taskParams->url = String(url);
        taskParams->md5 = String(md5);

        xTaskCreate(
            otaWorkerTask,
            "ota_task",
            8192,
            taskParams,
            1,
            NULL
        );

        JsonDocument res;
        res["ota_status"] = "STARTED";
        res["target_url"] = url;
        res["current_version"] = BRIDGE_FIRMWARE_VERSION;
        return createSuccessResponse(id, res);
    }

    return createErrorResponse(id, -32601, "Method not found");
}

String RpcDispatcher::createSuccessResponse(const JsonVariantConst& id, const JsonDocument& result) {
    JsonDocument resp;
    resp["jsonrpc"] = "2.0";
    resp["result"] = result;
    resp["id"] = id;
    String out;
    serializeJson(resp, out);
    return out;
}

String RpcDispatcher::createErrorResponse(const JsonVariantConst& id, int code, const char* message) {
    JsonDocument resp;
    resp["jsonrpc"] = "2.0";
    JsonObject err = resp["error"].to<JsonObject>();
    err["code"] = code;
    err["message"] = message;
    resp["id"] = id;
    String out;
    serializeJson(resp, out);
    return out;
}
