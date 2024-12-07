#include "webui.h"

WebUI::WebUI(AsyncWebServer &srv) : server(srv)
{
}

WebUI::~WebUI()
{
    // Destructor implementation
}

void WebUI::init(const UpdateSuccessCallback &updateCb, const UploadHandlerCallback &uploadCb)
{
    updateCallback = updateCb;
    uploadHandlerCallback = uploadCb;

    server.on("/light", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(
            LittleFS,
            WebUI::LIGHT_HTML,
            "text/html",
            false,
            [this](const String& var) -> String {
                return this->pageProcessor(var, Page::LIGHT);
            }
        );
    });
    server.on("/light", HTTP_POST, [this](AsyncWebServerRequest *request) {
        // handle light control
    });

    server.on("/system", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(
            LittleFS,
            WebUI::SYSTEM_HTML,
            "text/html",
            false,
            [this](const String& var) -> String {
                return this->pageProcessor(var, Page::SYSTEM);
            }
        );
    });
    server.on("/system", HTTP_POST, [this](AsyncWebServerRequest *request) {
        // handle system control
    });

    // Route for "/update" GET request
    server.on("/update", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(
            LittleFS,
            WebUI::FIRMWARE_HTML,
            "text/html",
            false,
            [this](const String& var) -> String {
                return this->pageProcessor(var, Page::FIRMWARE);
            }
        );
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

    server.serveStatic("/", LittleFS, "/").setCacheControl("max-age=3600");
    server.begin();
}

void WebUI::initHostAP(const WiFiSetupCallback &wifiCb) {
    wifiCredentialsCallback = wifiCb;

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(
            LittleFS,
            WebUI::WIFI_MANAGER_HTML,
            "text/html"
        );
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

String WebUI::pageProcessor(const String &var, Page page)
{
    if (var == WebUI::FIRMWARE) {
        return firmwareVersion;
    }

    // switch (page) {
    //     case Page::LIGHT:
    //         return lightProcessor(var);
    //     case Page::SYSTEM:
    //         return systemProcessor(var);
    //     case Page::FIRMWARE:
    //         return fwUpdateProcessor(var);
    //     case Page::WIFI_SETUP:
    //         return wifiSetupProcessor(var);
    // }
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

