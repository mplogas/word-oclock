#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include "constants.h"
#include <functional>
#include <memory>

using UpdateSuccessCallback = std::function<bool()>;
using UploadHandlerCallback = std::function<void(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)>;
using WiFiCredentialsCallback = std::function<void(const String &ssid, const String &password)>;

// Forward declaration for shared_ptr usage
class WebUI : public std::enable_shared_from_this<WebUI>
{
    private:
        AsyncWebServer &server; 
        UpdateSuccessCallback updateCallback;
        UploadHandlerCallback uploadHandlerCallback;
        WiFiCredentialsCallback wifiCredentialsCallback;

        // Processor functions
        String wifiSetupProcessor(const String &var);
        String fwUpdateProcessor(const String &var);
        String configurationProcessor(const String &var);

        // Helper functions
        void handleFirmwareUpdate(AsyncWebServerRequest *request);
        String sanitizeHTML(const String &input);

        // Constant strings
        static constexpr const char* INDEX_HTML = "/index.html";
        static constexpr const char* FIRMWARE_HTML = "/firmware.html";
        static constexpr const char* WIFI_MANAGER_HTML = "/wifimanager.html";
        static constexpr const char* PAGE_TITLE = "PAGE_TITLE";
        static constexpr const char* FIRMWARE = "FW_VERSION";

    public:
        WebUI(AsyncWebServer &server);
        ~WebUI();

        void init(const UpdateSuccessCallback &updateCb, const UploadHandlerCallback &uploadCb);
        void initHostAP(const WiFiCredentialsCallback &wifiCb);
};

WebUI::WebUI(AsyncWebServer &srv) : server(srv)
{
    // Constructor implementation
}

WebUI::~WebUI()
{
    // Destructor implementation
}

void WebUI::init(const UpdateSuccessCallback &updateCb, const UploadHandlerCallback &uploadCb)
{
    updateCallback = updateCb;
    uploadHandlerCallback = uploadCb;

    // Using weak_ptr to prevent dangling references
    std::weak_ptr<WebUI> weakSelf = shared_from_this();

    server.on("/", HTTP_GET, [weakSelf](AsyncWebServerRequest *request) {
        if(auto self = weakSelf.lock()) {
            request->send(LittleFS, INDEX_HTML, "text/html", false, std::bind(&WebUI::configurationProcessor, self.get(), std::placeholders::_1));
        }
    });

    server.on("/update", HTTP_GET, [weakSelf](AsyncWebServerRequest *request) {
        if(auto self = weakSelf.lock()) {
            request->send(LittleFS, FIRMWARE_HTML, "text/html", false, std::bind(&WebUI::fwUpdateProcessor, self.get(), std::placeholders::_1));
        }
    });

    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleFirmwareUpdate(request);
    },
    [this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
        uploadHandlerCallback(request, filename, index, data, len, final);
    });

    // Other routes with sanitized handlers
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", F("Not found"));
    });

    server.on("/on", HTTP_GET, [weakSelf](AsyncWebServerRequest *request) {
        if(auto self = weakSelf.lock()) {
            request->send(LittleFS, INDEX_HTML, "text/html", false, std::bind(&WebUI::configurationProcessor, self.get(), std::placeholders::_1));
        }
    });

    server.on("/off", HTTP_GET, [weakSelf](AsyncWebServerRequest *request) {
        if(auto self = weakSelf.lock()) {
            request->send(LittleFS, INDEX_HTML, "text/html", false, std::bind(&WebUI::configurationProcessor, self.get(), std::placeholders::_1));
        }
    });

    server.serveStatic("/", LittleFS, "/").setCacheControl("max-age=3600");
    server.begin();
}

void WebUI::initHostAP(const WiFiCredentialsCallback &wifiCb) {
    wifiCredentialsCallback = wifiCb;

    std::weak_ptr<WebUI> weakSelf = shared_from_this();

    server.on("/", HTTP_GET, [weakSelf](AsyncWebServerRequest *request) {
        if(auto self = weakSelf.lock()) {
            request->send(LittleFS, WIFI_MANAGER_HTML, "text/html", false, std::bind(&WebUI::wifiSetupProcessor, self.get(), std::placeholders::_1));
        }
    });

    server.on("/", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String ssid, password;
        int params = request->params();
        for (int i = 0; i < params; i++) {
            const AsyncWebParameter* p = request->getParam(i);
            if (p->isPost()) {
                if (p->name() == SSID_INPUT) {
                    ssid = p->value();
                    // Validate SSID length
                    if (ssid.length() < 1 || ssid.length() > 32) {
                        request->send(400, "text/plain", "Invalid SSID length");
                        return;
                    }
                } else if (p->name() == WIFI_PASS_INPUT) {
                    password = p->value();
                    // Validate password length
                    if (password.length() < 8) {
                        request->send(400, "text/plain", "Password too short");
                        return;
                    }
                }
            }
        }
        // Use the callback with the received SSID and password
        wifiCredentialsCallback(ssid, password);
        request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to the new address");
        delay(3000);
        ESP.restart();
    });

    server.serveStatic("/", LittleFS, "/").setCacheControl("max-age=3600");
    server.begin();
}

String WebUI::wifiSetupProcessor(const String &var)
{
    if (var == PAGE_TITLE) {
        return PRODUCT;
    }

    return String();
}

String WebUI::fwUpdateProcessor(const String &var) 
{
    if (var == PAGE_TITLE) {
        return PRODUCT;
    } else if (var == FIRMWARE) {
        return FW_VERSION;
    }

    return String();
}

String WebUI::configurationProcessor(const String &var)
{
    if (var == FIRMWARE) {
        return FW_VERSION;
    } else if (var == PAGE_TITLE) {
        return PRODUCT;
    }
    return String();
}

void WebUI::handleFirmwareUpdate(AsyncWebServerRequest *request)
{
    Serial.println("Update requested");

    if (!updateCallback()) {
        Serial.println("Update successful");
        auto response = request->beginResponse(200, "text/plain", "OK");
        response->addHeader("Connection", "close");
        request->send(response);
        ESP.restart();
    } else {
        Serial.println("Update failed");
        auto response = request->beginResponse(500, "text/plain", "ERROR");
        response->addHeader("Connection", "close");
        request->send(response);
    }
}

String WebUI::sanitizeHTML(const String &input) {
    String output = input;
    output.replace("&", "&amp;");
    output.replace("<", "&lt;");
    output.replace(">", "&gt;");
    output.replace("\"", "&quot;");
    output.replace("'", "&#039;");
    return output;
}

#endif