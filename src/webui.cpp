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

    //server.serveStatic("/", LittleFS, "/").setCacheControl("max-age=3600");
    // Serve Static CSS and JS only
    server.serveStatic("/style.css", LittleFS, "/style.css").setCacheControl("max-age=3600");
    server.serveStatic("/index.js", LittleFS, "/index.js").setCacheControl("max-age=3600");
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

String WebUI::pageProcessor(const String &var, Page page)
{
    if (var == "PAGE_TITLE") {
        switch (page)
        {
        case Page::LIGHT:
            return "Light Configuration";
        case Page::SYSTEM:
            return "System Configuration";
        case Page::FIRMWARE:
            return "Firmware Update";
        default:
            return String();
        }
    } else if (var == "FW_UPDATE_LINK") {
        if (page == Page::SYSTEM) {
            return "<a href=\"/update\" class=\"link\"><i class=\"fas fa-upload\"></i> Firmware Update</a>";
        } else {
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
        headerContent.replace("%FW_UPDATE_LINK%", pageProcessor("FW_UPDATE_LINK", page));
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

