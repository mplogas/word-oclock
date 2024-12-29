#include "webui.h"

WebUI::WebUI(AsyncWebServer &srv) : server(srv)
{
}

WebUI::~WebUI()
{
    // Destructor implementation
    // check and cleanup callbacks
    updateSuccessCallback = nullptr;
    uploadHandlerCallback = nullptr;
    wifiCredentialsCallback = nullptr;
    lightControlCallback = nullptr;
    systemControlCallback = nullptr;
}

void WebUI::init(const LightControlCallback &lightCtrlCb,
                 const SystemControlCallback &systemCtrlCb,
                 const UploadHandlerCallback &uploadCb,
                 const UpdateSuccessCallback &updateCb,
                 Configuration::LightConfig *lightConfig,
                 Configuration::SystemConfig *systemConfig)
{
    // check if any of the callbacks is null
    if (!lightCtrlCb || !systemCtrlCb || !uploadCb || !updateCb)
    {
        Serial.println("One or more callbacks are null");
        return;
    }

    updateSuccessCallback = updateCb;
    uploadHandlerCallback = uploadCb;
    lightControlCallback = lightCtrlCb;
    systemControlCallback = systemCtrlCb;

    if (!lightConfig || !systemConfig)
    {
        Serial.println("Light or System configuration is null");
        return;
    }

    lightConfiguration = lightConfig;
    systemConfiguration = systemConfig;

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->redirect("/light"); });
    server.on("/light", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::LIGHT_HTML,
                    "text/html",
                    false,
                    [this](const String &var) -> String
                    {
                        return this->pageProcessor(var, Page::LIGHT);
                    }); });

    // Handle light status toggle
    server.on("/toggleLight", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->handleToggleLight(request); });

    // Handle light color change
    server.on("/setLightColor", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->handleSetLightColor(request); });

    // Handle auto-brightness toggle
    server.on("/setAutoBrightness", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->handleSetAutoBrightness(request); });

    // Handle brightness adjustment
    server.on("/setBrightness", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->handleSetBrightness(request); });

    server.on("/time", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::TIME_HTML,
                    "text/html",
                    false,
                    [this](const String &var) -> String
                    {
                        return this->pageProcessor(var, Page::TIME);
                    }); });

    server.on("/getCurrentTime", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "application/text", "12:30");
    });


    server.on("/system", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::SYSTEM_HTML,
                    "text/html",
                    false,
                    [this](const String &var) -> String
                    {
                        return this->pageProcessor(var, Page::SYSTEM);
                    }); });

    server.on("/setHaIntegration", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->handleSetHAIntegration(request); });

    server.on("/update", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::FIRMWARE_HTML,
                    "text/html",
                    false,
                    [this](const String &var) -> String
                    {
                        return this->pageProcessor(var, Page::FIRMWARE);
                    }); });

    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest *request)
              { handleFirmwareUpdate(request); }, [this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
              { 
                if (request->hasParam("updateType", true) && request->hasParam("file", true)) {
                    String updateType = request->getParam("updateType")->value();
                    UpdateType type = updateType == "firmware" ? UpdateType::FIRMWARE : UpdateType::FILESYSTEM;
                    if (updateType == "firmware")
                    {
                        uploadHandlerCallback(UpdateType::FIRMWARE, filename, index, data, len, final); 
                    }
                    else if (updateType == "data")
                    {
                        uploadHandlerCallback(UpdateType::FILESYSTEM, filename, index, data, len, final); 
                    }
                    else
                    {
                        request->send(400, "text/plain", "Invalid update type");
                    }
                }                
              });

    // Other routes with sanitized handlers
    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, "text/plain", F("Not found")); });

    // Serve Static CSS and JS only
    server.serveStatic("/style.css", LittleFS, "/style.css").setCacheControl("max-age=3600");
    server.serveStatic("/index.js", LittleFS, "/index.js").setCacheControl("max-age=3600");
    server.serveStatic("favicon.ico", LittleFS, "/favicon.ico").setCacheControl("max-age=3600");
    server.begin();
}

void WebUI::initHostAP(const WiFiSetupCallback &wifiCb)
{
    wifiCredentialsCallback = wifiCb;

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::WIFI_MANAGER_HTML,
                    "text/html"); });

    server.on("/", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
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
        ESP.restart(); });

    server.serveStatic("/", LittleFS, "/").setCacheControl("max-age=3600");
    server.begin();
}

void WebUI::handleFirmwareUpdate(AsyncWebServerRequest *request)
{
    Serial.println("Update requested");

    if (updateSuccessCallback())
    {
        Serial.println("Update successful");
        auto response = request->beginResponse(200, "text/plain", "OK");
        response->addHeader("Connection", "close");
        request->send(response);
        ESP.restart();
    }
    else
    {
        Serial.println("Update failed");
        auto response = request->beginResponse(500, "text/plain", "ERROR");
        response->addHeader("Connection", "close");
        request->send(response);
    }
}

void WebUI::handleToggleLight(AsyncWebServerRequest *request)
{
    if (request->hasParam("status"))
    {
        String statusParam = request->getParam("status")->value();

        // Validate status parameter
        if (statusParam == "0" || statusParam == "1")
        {
            lightControlCallback(LightOperationType::ToggleStatus, statusParam);

            request->send(200, "text/plain", "Light status updated");
        }
        else
        {
            request->send(400, "text/plain", "Invalid status value");
        }
    }
    else
    {
        request->send(400, "text/plain", "Missing status parameter");
    }
}

void WebUI::handleSetLightColor(AsyncWebServerRequest *request)
{
    if (request->hasParam("color"))
    {
        String colorParam = request->getParam("color")->value();
        // Validate color format (expecting #RRGGBB)
        if (colorParam.length() == 7 && colorParam[0] == '#')
        {
            bool valid = true;
            for (size_t i = 1; i < colorParam.length(); i++)
            {
                char c = colorParam[i];
                if (!isxdigit(c))
                {
                    valid = false;
                    break;
                }
            }

            if (valid)
            {
                lightControlCallback(LightOperationType::SetColor, colorParam);
                request->send(200, "text/plain", "Light color updated");
            }
            else
            {
                request->send(400, "text/plain", "Invalid color format");
            }
        }
        else
        {
            request->send(400, "text/plain", "Invalid color format");
        }
    }
    else
    {
        request->send(400, "text/plain", "Missing color parameter");
    }
}

void WebUI::handleSetAutoBrightness(AsyncWebServerRequest *request)
{
    if (request->hasParam("enabled"))
    {
        String enabledParam = request->getParam("enabled")->value();

        // Validate enabled parameter
        if (enabledParam == "0" || enabledParam == "1")
        {
            bool enabled = enabledParam == "1";
            lightControlCallback(LightOperationType::SetAutoBrightness, enabledParam);
            request->send(200, "text/plain", "Auto-brightness updated");
        }
        else
        {
            request->send(400, "text/plain", "Invalid enabled value");
        }
    }
    else
    {
        request->send(400, "text/plain", "Missing enabled parameter");
    }
}

void WebUI::handleSetBrightness(AsyncWebServerRequest *request)
{
    if (request->hasParam("value"))
    {
        String valueParam = request->getParam("value")->value();

        // Validate brightness value (should be an integer between 0 and 255)
        int brightness = valueParam.toInt();
        if (brightness >= 0 && brightness <= 255)
        {
            lightControlCallback(LightOperationType::SetBrightness, valueParam);
            request->send(200, "text/plain", "Brightness updated");
        }
        else
        {
            request->send(400, "text/plain", "Invalid brightness value");
        }
    }
    else
    {
        request->send(400, "text/plain", "Missing value parameter");
    }
}

void WebUI::handleSetHAIntegration(AsyncWebServerRequest *request)
{
    if (request->hasParam("haIntegration"))
    {
        char haIntegration[2] = {0};
        strncpy(haIntegration, request->getParam("haIntegration")->value().c_str(), sizeof(haIntegration) - 1);
        haIntegration[sizeof(haIntegration) - 1] = '\0';
        int enabled = atoi(haIntegration);

        if (enabled == 0)
        {
            std::map<String, String> params;
            params["enabled"] = haIntegration;
            systemControlCallback(SystemOperationType::SetHaIntegration, params);
            request->send(200, "text/plain", "Home Assistant integration disabled");
        }
        else if (enabled == 1)
        {
            if (request->hasParam("mqttHost") && request->hasParam("mqttPort"))
            {
                // these are somewhat optional
                // && request->hasParam("mqttUsername") && request->hasParam("mqttPassword") && request->hasParam("mqttTopic")

                // GitHub copilot doesn't like my clean approach :(
                // after 2nd thought, it's onto something here. lifetime of pointers and stuff.
                // const char *mqttHost = request->getParam("mqttHost")->value().c_str();
                // const char *mqttPort = request->getParam("mqttPort")->value().c_str();
                // const char *mqttUsername = request->getParam("mqttUsername")->value().c_str();
                // const char *mqttPassword = request->getParam("mqttPassword")->value().c_str();
                // const char *mqttTopic = request->getParam("mqttTopic")->value().c_str();

                char mqttHost[65] = {0};
                char mqttPort[6] = {0};
                char mqttUsername[65] = {0};
                char mqttPassword[65] = {0};
                char mqttTopic[65] = {0};

                // Copy parameters to fixed-size buffers
                strncpy(mqttHost, request->getParam("mqttHost")->value().c_str(), sizeof(mqttHost) - 1);
                strncpy(mqttPort, request->getParam("mqttPort")->value().c_str(), sizeof(mqttPort) - 1);
                strncpy(mqttUsername, request->getParam("mqttUsername")->value().c_str(), sizeof(mqttUsername) - 1);
                strncpy(mqttPassword, request->getParam("mqttPassword")->value().c_str(), sizeof(mqttPassword) - 1);
                strncpy(mqttTopic, request->getParam("mqttTopic")->value().c_str(), sizeof(mqttTopic) - 1);

                // Ensure null-termination
                mqttHost[sizeof(mqttHost) - 1] = '\0';
                mqttPort[sizeof(mqttPort) - 1] = '\0';
                mqttUsername[sizeof(mqttUsername) - 1] = '\0';
                mqttPassword[sizeof(mqttPassword) - 1] = '\0';
                mqttTopic[sizeof(mqttTopic) - 1] = '\0';

                // Validate MQTT host
                if (strlen(mqttHost) < 1 || strlen(mqttHost) > 64)
                {
                    request->send(400, "text/plain", "Invalid MQTT host");
                    return;
                }

                // Validate MQTT port
                int port = atoi(mqttPort);
                if (port < 1 || port > 65535)
                {
                    request->send(400, "text/plain", "Invalid MQTT port");
                    return;
                }

                // Optional parameters
                // Validate MQTT username
                if (strlen(mqttUsername) > 64)
                {
                    request->send(400, "text/plain", "Invalid MQTT username");
                    return;
                }

                // Validate MQTT password
                if (strlen(mqttPassword) > 64)
                {
                    request->send(400, "text/plain", "Invalid MQTT password");
                    return;
                }

                // Validate MQTT topic
                if (strlen(mqttTopic) > 64)
                {
                    request->send(400, "text/plain", "Invalid MQTT topic");
                    return;
                }

                // Prepare parameters
                std::map<String, String> params;
                params["mqttHost"] = mqttHost;
                params["mqttPort"] = mqttPort;
                if (strlen(mqttUsername) > 0)
                    params["mqttUsername"] = mqttUsername;
                if (strlen(mqttPassword) > 0)
                    params["mqttPassword"] = mqttPassword;
                if (strlen(mqttTopic) > 0)
                    params["defaultTopic"] = mqttTopic;

                // Call the system control callback
                systemControlCallback(SystemOperationType::SetHaIntegration, params);

                request->send(200, "text/plain", "Home Assistant integration settings updated");
            }
        }
    }
    request->send(400, "text/plain", "Missing parameters");
}

String WebUI::pageProcessor(const String &var, Page page)
{
    if (var == "INCLUDE_HEADER")
    {
        return headerProcessor(page);
    }

    switch (page)
    {
    case Page::LIGHT:
        return lightPageProcessor(var);
    case Page::SYSTEM:
        return systemPageProcessor(var);
    case Page::FIRMWARE:
        return firmwarePageProcessor(var);
    default:
        return String();
    }
}

String WebUI::headerProcessor(Page page)
{
    String headerContent = readFile(HEADER_HTML);
    if (headerContent.length() == 0)
    {
        return String();
    }

    String pageTitle;
    switch (page)
    {
    case Page::LIGHT:
        pageTitle = LIGHT_PAGE_TITLE;
        break;
    case Page::SYSTEM:
        pageTitle = SYSTEM_PAGE_TITLE;
        break;
    case Page::TIME:
        pageTitle = TIME_PAGE_TITLE;
        break;        
    case Page::FIRMWARE:
        pageTitle = FIRMWARE_PAGE_TITLE;
        break;
    default:
        pageTitle = "Unknown";
        break;
    }

    headerContent.replace("%PAGE_TITLE%", pageTitle);
    headerContent.replace("%ACTIVE_LIGHT%", (page == Page::LIGHT) ? "active" : "");
    headerContent.replace("%ACTIVE_TIME%", (page == Page::TIME) ? "active" : "");
    headerContent.replace("%ACTIVE_SYSTEM%", (page == Page::SYSTEM) ? "active" : "");

    return headerContent;
}

String WebUI::lightPageProcessor(const String &var)
{
    if (var == "LIGHT_STATE")
    {
        Serial.printf("Light status: %s\n", lightConfiguration->state ? "ON" : "OFF");
        return lightConfiguration->state ? "checked" : "";
    }
    else if (var == "LIGHT_COLOR")
    {
        return lightConfiguration->color;
    }
    else if (var == "AUTO_BRIGHTNESS_STATE")
    {
        Serial.printf("Auto brightness: %s\n", lightConfiguration->autoBrightnessConfig.enabled ? "ON" : "OFF");
        return lightConfiguration->autoBrightnessConfig.enabled ? "checked" : "";
    }
    else if (var == "LIGHT_BRIGHTNESS")
    {
        return String(lightConfiguration->brightness);
    }
    else
    {
        return String();
    }
}

String WebUI::systemPageProcessor(const String &var)
{
    if (var == "HA_INTEGRATION_STATE")
    {
        return systemConfiguration->mqttConfig.enabled ? "checked" : "";
    }
    else if (var == "BROKER_IP")
    {
        return systemConfiguration->mqttConfig.host;
    }
    else if (var == "BROKER_PORT")
    {
        return String(systemConfiguration->mqttConfig.port);
    }
    else if (var == "MQTT_USERNAME")
    {
        return systemConfiguration->mqttConfig.username;
    }
    else if (var == "DEFAULT_TOPIC")
    {
        return systemConfiguration->mqttConfig.topic;
    }
    else
    {
        return String();
    }
    return String();
}

String WebUI::firmwarePageProcessor(const String &var)
{
    if (var == "FW_VERSION")
    {
        return Defaults::FW_VERSION;
    }
    else
    {
        return String();
    }
}

String WebUI::readFile(const char *path)
{
    File file = LittleFS.open(path, "r");
    if (!file)
    {
        return String();
    }

    String fileContent = file.readString();
    file.close();
    return fileContent;
}
