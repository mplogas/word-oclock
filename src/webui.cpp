#include "webui.h"

WebUI::WebUI(AsyncWebServer &srv) : server(srv)
{
}

WebUI::~WebUI()
{
    // Destructor implementation
    //check and cleanup callbacks
    updateCallback = nullptr;
    uploadHandlerCallback = nullptr;
    wifiCredentialsCallback = nullptr;
    lightControlCallback = nullptr;
    systemControlCallback = nullptr;
}

void WebUI::init(const LightControlCallback &lightCtrlCb, const SystemControlCallback &systemCtrlCb, const UploadHandlerCallback &uploadCb, const UpdateSuccessCallback &updateCb)
{
    //check if any of the callbacks is null
    if (lightCtrlCb == nullptr || systemCtrlCb == nullptr || uploadCb == nullptr || updateCb == nullptr)
    {
        Serial.println("One or more callbacks are null");
        return;
    }

    updateCallback = updateCb;
    uploadHandlerCallback = uploadCb;
    lightControlCallback = lightCtrlCb;
    systemControlCallback = systemCtrlCb;
    
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->redirect("/light");
    });
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

    // Handle light status toggle
    server.on("/toggleLight", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleToggleLight(request);
    });

    // Handle light color change
    server.on("/setLightColor", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleSetLightColor(request);
    });

    // Handle auto-brightness toggle
    server.on("/setAutoBrightness", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleSetAutoBrightness(request);
    });

    // Handle brightness adjustment
    server.on("/setBrightness", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleSetBrightness(request);
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
        uploadHandlerCallback(filename, index, data, len, final);
    });

    // Other routes with sanitized handlers
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", F("Not found"));
    });

    // Serve Static CSS and JS only
    server.serveStatic("/style.css", LittleFS, "/style.css").setCacheControl("max-age=3600");
    server.serveStatic("/index.js", LittleFS, "/index.js").setCacheControl("max-age=3600");
    server.serveStatic("favicon.ico", LittleFS, "/favicon.ico").setCacheControl("max-age=3600");
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

void WebUI::handleToggleLight(AsyncWebServerRequest *request)
{
    if (request->hasParam("status")) {
        String statusParam = request->getParam("status")->value();

        // Validate status parameter
        if (statusParam == "0" || statusParam == "1") {
            lightControlCallback(LightOperationType::ToggleStatus, statusParam);

            request->send(200, "text/plain", "Light status updated");
        } else {
            request->send(400, "text/plain", "Invalid status value");
        }
    } else {
        request->send(400, "text/plain", "Missing status parameter");
    }
}

void WebUI::handleSetLightColor(AsyncWebServerRequest *request)
{
    if (request->hasParam("color")) {
        String colorParam = request->getParam("color")->value();
        // Validate color format (expecting #RRGGBB)
        if (colorParam.length() == 7 && colorParam[0] == '#') {
            bool valid = true;
            for (size_t i = 1; i < colorParam.length(); i++) {
                char c = colorParam[i];
                if (!isxdigit(c)) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                lightControlCallback(LightOperationType::SetColor, colorParam);
                request->send(200, "text/plain", "Light color updated");
            } else {
                request->send(400, "text/plain", "Invalid color format");
            }
        } else {
            request->send(400, "text/plain", "Invalid color format");
        }
    } else {
        request->send(400, "text/plain", "Missing color parameter");
    }
}

void WebUI::handleSetAutoBrightness(AsyncWebServerRequest *request)
{
    if (request->hasParam("enabled")) {
        String enabledParam = request->getParam("enabled")->value();

        // Validate enabled parameter
        if (enabledParam == "0" || enabledParam == "1") {
            bool enabled = enabledParam == "1";
            lightControlCallback(LightOperationType::SetAutoBrightness, enabledParam);
            request->send(200, "text/plain", "Auto-brightness updated");
        } else {
            request->send(400, "text/plain", "Invalid enabled value");
        }
    } else {
        request->send(400, "text/plain", "Missing enabled parameter");
    }
}

void WebUI::handleSetBrightness(AsyncWebServerRequest *request)
{
    if (request->hasParam("value")) {
        String valueParam = request->getParam("value")->value();

        // Validate brightness value (should be an integer between 0 and 255)
        int brightness = valueParam.toInt();
        if (brightness >= 0 && brightness <= 255) {
            lightControlCallback(LightOperationType::SetBrightness, valueParam);
            request->send(200, "text/plain", "Brightness updated");
        } else {
            request->send(400, "text/plain", "Invalid brightness value");
        }
    } else {
        request->send(400, "text/plain", "Missing value parameter");
    }
}

String WebUI::pageProcessor(const String &var, Page page)
{
    if (var == "PAGE_TITLE") {
        switch (page)
        {
        case Page::LIGHT:
            return "Light Settings";
        case Page::SYSTEM:
            return "System Configuration";
        case Page::FIRMWARE:
            return "Firmware Update";
        default:
            return String();
        }
    } else if (var == "ACTIVE_LIGHT") {
        return (page == Page::LIGHT) ? "active" : "";
    } else if (var == "ACTIVE_SYSTEM") {
        return (page == Page::SYSTEM) ? "active" : "";
    } else if (var == "INCLUDE_HEADER") {
        // Return header content with placeholders processed
        String headerContent = readFile(HEADER_HTML);
        // Create a temporary processor to handle header placeholders
        headerContent.replace("%PAGE_TITLE%", pageProcessor("PAGE_TITLE", page));
        headerContent.replace("%ACTIVE_LIGHT%", pageProcessor("ACTIVE_LIGHT", page));
        headerContent.replace("%ACTIVE_SYSTEM%", pageProcessor("ACTIVE_SYSTEM", page));
        return headerContent;
    }
    return String();
}


String WebUI::readFile(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) {
        return String();
    }

    String fileContent = file.readString();
    file.close();
    return fileContent;
}

