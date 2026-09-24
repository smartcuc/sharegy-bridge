#include "ota_manager.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

OtaManager ota;

void OtaManager::init(WebServer* webServer) {
    server = webServer;
    if (!server) return;

    // Local Web OTA Upload Handler
    server->on("/update", HTTP_POST, [this]() {
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", (Update.hasError()) ? "OTA_FAIL" : "OTA_OK_REBOOTING");
        delay(1000);
        ESP.restart();
    }, [this]() {
        HTTPUpload& upload = server->upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("[OTA] Local Update Start: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
                Serial.printf("[OTA] Local Update Success: %u Bytes. Rebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });
}

bool OtaManager::performRemoteOta(const String& binaryUrl, const String& expectedMd5) {
    Serial.printf("[OTA] Starting Remote Cloud OTA from: %s\n", binaryUrl.c_str());
    
    HTTPClient http;
    http.begin(binaryUrl);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[OTA] HTTP GET failed, error: %s (code %d)\n", http.errorToString(httpCode).c_str(), httpCode);
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        Serial.println("[OTA] Content-Length is 0. Aborting.");
        http.end();
        return false;
    }

    if (expectedMd5.length() > 0) {
        Update.setMD5(expectedMd5.c_str());
    }

    bool canBegin = Update.begin(contentLength);
    if (!canBegin) {
        Serial.println("[OTA] Not enough space to begin OTA");
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    size_t written = Update.writeStream(*stream);

    if (written != (size_t)contentLength) {
        Serial.printf("[OTA] Written bytes (%u) != Content-Length (%d)\n", written, contentLength);
        http.end();
        return false;
    }

    if (!Update.end()) {
        Serial.printf("[OTA] Update.end() error #: %u\n", Update.getError());
        http.end();
        return false;
    }

    if (!Update.isFinished()) {
        Serial.println("[OTA] Update not finished? Something went wrong.");
        http.end();
        return false;
    }

    Serial.println("[OTA] Remote Cloud OTA Update successfully written! Rebooting into new firmware...");
    http.end();
    delay(1000);
    ESP.restart();
    return true;
}
