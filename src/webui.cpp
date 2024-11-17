#include "webui.h"

WebUI::WebUI(AsyncWebServer &srv, const char* devicename, const char* firmware) : server(srv)
{
    deviceName = devicename;
    firmwareVersion = firmware;
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
            request->send(LittleFS, WebUI::INDEX_HTML, "text/html", false, std::bind(&WebUI::configurationProcessor, self.get(), std::placeholders::_1));
        }
    });

    server.on("/update", HTTP_GET, [weakSelf](AsyncWebServerRequest *request) {
        if(auto self = weakSelf.lock()) {
            request->send(LittleFS, WebUI::FIRMWARE_HTML, "text/html", false, std::bind(&WebUI::fwUpdateProcessor, self.get(), std::placeholders::_1));
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
            request->send(LittleFS, WebUI::INDEX_HTML, "text/html", false, std::bind(&WebUI::configurationProcessor, self.get(), std::placeholders::_1));
        }
    });

    server.on("/off", HTTP_GET, [weakSelf](AsyncWebServerRequest *request) {
        if(auto self = weakSelf.lock()) {
            request->send(LittleFS, WebUI::INDEX_HTML, "text/html", false, std::bind(&WebUI::configurationProcessor, self.get(), std::placeholders::_1));
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
            request->send(LittleFS, WebUI::WIFI_MANAGER_HTML, "text/html", false, std::bind(&WebUI::wifiSetupProcessor, self.get(), std::placeholders::_1));
        }
    });

    server.on("/", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String ssid, password;
        int params = request->params();
        for (int i = 0; i < params; i++) {
            const AsyncWebParameter* p = request->getParam(i);
            if (p->isPost()) {
                if (p->name() == WebUI::SSID_INPUT) {
                    ssid = p->value();
                    // Validate SSID length
                    if (ssid.length() < 1 || ssid.length() > 32) {
                        request->send(400, "text/plain", "Invalid SSID length");
                        return;
                    }
                } else if (p->name() == WebUI::WIFI_PASS_INPUT) {
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
    if (var == WebUI::PAGE_TITLE) {
        return deviceName;
    }

    return String();
}

String WebUI::fwUpdateProcessor(const String &var) 
{
    if (var == WebUI::PAGE_TITLE) {
        return deviceName;
    } else if (var == WebUI::FIRMWARE) {
        return firmwareVersion;
    }

    return String();
}

String WebUI::configurationProcessor(const String &var)
{
    if (var == WebUI::FIRMWARE) {
        return deviceName;
    } else if (var == WebUI::PAGE_TITLE) {
        return firmwareVersion;
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

