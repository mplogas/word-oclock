#include "webui.h"

WebUI::WebUI(AsyncWebServer &srv) : server(srv)
{
}

WebUI::~WebUI()
{
    // Destructor implementation
    // check and cleanup callbacks
    updateSuccessCallback = nullptr;
    requestCallback = nullptr;
    responseCallback = nullptr;
    updateCallback = nullptr;
    
}

void WebUI::init(const RequestCallback &requestCb,
                 const ResponseCallback &responseCb,
                 const UpdateCallback &updateCb,
                 const UpdateSuccessCallback &updateSuccessCb)
{
    // check if any of the callbacks is null
    if (!requestCb || !responseCb || !updateCb || !updateSuccessCb)
    {
        Serial.println("One or more callbacks are null");
        return;
    }

    updateSuccessCallback = updateSuccessCb;
    requestCallback = requestCb;
    responseCallback = responseCb;
    updateCallback = updateCb;

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->redirect("/light"); });
    server.on("/light", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::PATH_LIGHT_HTML,
                    CONTENT_HTML,
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
                    WebUI::PATH_TIME_HTML,
                    CONTENT_HTML,
                    false,
                    [this](const String &var) -> String
                    {
                        return this->pageProcessor(var, Page::TIME);
                    }); });

    server.on("/getCurrentTime", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, CONTENT_TEXT, "12:30"); });

    server.on("/system", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::PATH_SYSTEM_HTML,
                    CONTENT_HTML,
                    false,
                    [this](const String &var) -> String
                    {
                        return this->pageProcessor(var, Page::SYSTEM);
                    }); });

    server.on("/setHaIntegration", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->handleSetHAIntegration(request); });

    server.on("/setClockFace", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
                  this->handleSetClockFace(request);
              });

    server.on("/resetConfig", HTTP_POST, [this](AsyncWebServerRequest *request)
              { requestCallback(ControlType::ResetConfig, std::map<String, String>()); });

    server.on("/update", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::PATH_FIRMWARE_HTML,
                    CONTENT_HTML,
                    false,
                    [this](const String &var) -> String
                    {
                        return this->pageProcessor(var, Page::FIRMWARE);
                    }); });

    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest *request)
              { handleFirmwareUpdate(request); }, [this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
              { 
                UpdateType type = UpdateType::FIRMWARE;
                if (index == 0 && request->hasParam(PARAM_FW_Type, true)) {  
                    //const AsyncWebParameter *p = request->getParam(PARAM_FW_Type, true);  
                    //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                    const String tParam = request->getParam(PARAM_FW_Type, true)->value();
                    //Serial.printf("Update type: %s\n", tParam.c_str());
                    if(tParam == VALUE_FIRMWARE) {
                        type = UpdateType::FIRMWARE;
                    } else if(tParam == VALUE_FILESYS) {
                        type = UpdateType::FILESYSTEM;
                    } else {
                        Serial.println("Invalid update type");
                        request->send(400, CONTENT_TEXT, VALUE_ERROR);
                    }
                }
                //Serial.printf("Type: %u, Update: %s, Index: %u, Len: %u, Final: %u\n", type, filename.c_str(), index, len, final);
                updateCallback(type, filename, index, data, len, final); });

    // Other routes with sanitized handlers
    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, CONTENT_TEXT, VALUE_ERROR); });

    // Serve Static CSS and JS only
    server.serveStatic(PATH_CSS, LittleFS, PATH_CSS).setCacheControl(CONTENT_CACHE);
    server.serveStatic(PATH_JS, LittleFS, PATH_JS).setCacheControl(CONTENT_CACHE);
    server.serveStatic(PATH_ICON, LittleFS, PATH_ICON).setCacheControl(CONTENT_CACHE);
    server.begin();
}

void WebUI::initHostAP(const RequestCallback &requestCb)
{
    requestCallback = requestCb;

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(
                    LittleFS,
                    WebUI::PATH_WIFI_HTML,
                    CONTENT_HTML); });

    server.on("/", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
        String ssid, password;

        if(request->hasParam(PARAM_WIFI_SSID, true) && request->hasParam(PARAM_WIFI_PASS, true)) {
            if (ssid.length() < 1 || ssid.length() > 32 || password.length() < 1) {
                Serial.println("Invalid wifi parameters");
                request->send(400, CONTENT_TEXT, VALUE_ERROR);
                return;
            }
        }

        std::map<String, String> params;
        params[PARAM_ENABLED] = VALUE_OFF;
        params[PARAM_WIFI_SSID] = request->getParam(PARAM_WIFI_SSID, true)->value();
        params[PARAM_WIFI_PASS] = request->getParam(PARAM_WIFI_PASS, true)->value();
        // Use the callback with the received SSID and password
        requestCallback(ControlType::WiFi, params);
        request->send(200, CONTENT_TEXT, VALUE_SUCCESS);
        delay(3000);
        ESP.restart(); });

    server.serveStatic("/", LittleFS, "/").setCacheControl(CONTENT_CACHE);
    server.begin();
}

void WebUI::handleFirmwareUpdate(AsyncWebServerRequest *request)
{
    if (updateSuccessCallback())
    {
        Serial.println("Update successful");
        // String responseHtml = "<html><body><h1>Update Successful</h1><p>The device will restart shortly. Please wait...</p><script>setTimeout(function(){ window.location.reload(); }, 10000);</script></body></html>";
        // request->send(200, CONTENT_HTML, responseHtml);
        //
        // auto response = request->beginResponse(200, CONTENT_TEXT, responseHtml);
        // response->addHeader("Connection", "close");
        // request->send(response);
        request->redirect("/system");
        Serial.println("Restarting...");
        delay(2000);
        ESP.restart();
    }
    else
    {
        Serial.println("Update failed");
        request->send(500, CONTENT_TEXT, VALUE_ERROR);
    }
}

void WebUI::handleToggleLight(AsyncWebServerRequest *request)
{
    if (request->hasParam(PARAM_ENABLED))
    {
        String statusParam = request->getParam(PARAM_ENABLED)->value();

        // Validate status parameter
        if (statusParam == VALUE_OFF || statusParam == VALUE_ON)
        {
            lightControlCallback(LightOperationType::ToggleStatus, statusParam);

            request->send(200, CONTENT_TEXT, VALUE_SUCCESS);
        }
        else
        {
Serial.println("Invalid status value");
            request->send(400, CONTENT_TEXT, VALUE_ERROR);
        }
    }
    else
    {
Serial.println("Missing status parameter");
        request->send(400, CONTENT_TEXT, VALUE_ERROR);
    }
}

void WebUI::handleSetLightColor(AsyncWebServerRequest *request)
{
    if (request->hasParam(PARAM_COLOR))
    {
        String colorParam = request->getParam(PARAM_COLOR)->value();
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
                request->send(200, CONTENT_TEXT, VALUE_SUCCESS);
                return;
            }
            else
            {
                Serial.println("Invalid color format");
                request->send(400, CONTENT_TEXT, VALUE_ERROR);
                return;
            }
        }
        else
        {
Serial.println("Invalid color format");
            request->send(400, CONTENT_TEXT, VALUE_ERROR);
            return;
        }
    }
    else
    {
Serial.println("Missing color parameter");
        request->send(400, CONTENT_TEXT, VALUE_ERROR);
        return;
    }
}

void WebUI::handleSetAutoBrightness(AsyncWebServerRequest *request)
{
    if (request->hasParam(PARAM_ENABLED))
    {
        String enabledParam = request->getParam(PARAM_ENABLED)->value();

        // Validate enabled parameter
        if (enabledParam == VALUE_OFF || enabledParam == VALUE_ON)
        {
            bool enabled = enabledParam == VALUE_ON;
            lightControlCallback(LightOperationType::SetAutoBrightness, enabledParam);
            request->send(200, CONTENT_TEXT, VALUE_SUCCESS);
            return;
        }
        else
        {
    Serial.println("Invalid enabled value");
            request->send(400, CONTENT_TEXT, VALUE_ERROR);
            return;
        }
    }
    else
    {
Serial.println("Missing enabled parameter");
        request->send(400, CONTENT_TEXT, VALUE_ERROR);
        return;
    }
}

void WebUI::handleSetBrightness(AsyncWebServerRequest *request)
{
    if (request->hasParam(PARAM_VALUE))
    {
        String valueParam = request->getParam(PARAM_VALUE)->value();

        // Validate brightness value (should be an integer between 0 and 255)
        int brightness = valueParam.toInt();
        if (brightness >= 0 && brightness <= 255)
        {
            lightControlCallback(LightOperationType::SetBrightness, valueParam);
            request->send(200, CONTENT_TEXT, VALUE_SUCCESS);
            return;
        }
        else
        {
            Serial.println("Invalid brightness value");
            request->send(400, CONTENT_TEXT, VALUE_ERROR);
            return;
        }
    }
    else
    {
        Serial.println("Missing value parameter");
        request->send(400, CONTENT_TEXT, VALUE_ERROR);
        return;
    }
}

void WebUI::handleSetHAIntegration(AsyncWebServerRequest *request)
{
    printAllParams(request);
    if (request->hasParam(PARAM_ENABLED, true))
    {
        String haIntegrationParam = request->getParam(PARAM_ENABLED, true)->value();
        if (haIntegrationParam == VALUE_OFF)
        {
            std::map<String, String> params;
            params[PARAM_ENABLED] = VALUE_OFF;
            params[PARAM_BROKER_HOST] = String();
            params[PARAM_BROKER_PORT] = String();
            params[PARAM_BROKER_USER] = String();
            params[PARAM_BROKER_PASS] = String();
            params[PARAM_BROKER_DEFAULT_TOPIC] = String();
            systemControlCallback(SystemOperationType::SetHaIntegration, params);
            request->send(200, CONTENT_TEXT, VALUE_SUCCESS);
            return;
        }
        else if (haIntegrationParam == VALUE_ON)
        {
            //TODO: check if parameters exist
            if (!request->hasParam(PARAM_BROKER_HOST, true) ||
                !request->hasParam(PARAM_BROKER_PORT, true) ||
                !request->hasParam(PARAM_BROKER_DEFAULT_TOPIC, true))
            {
                Serial.println("Missing parameters");
                request->send(400, CONTENT_TEXT, VALUE_ERROR);
                return;
            }

            String mqttHost = request->getParam(PARAM_BROKER_HOST, true)->value();
            String mqttPort = request->getParam(PARAM_BROKER_PORT, true)->value();
            String mqttTopic = request->getParam(PARAM_BROKER_DEFAULT_TOPIC, true)->value();
            if (mqttHost.length() > 0)
            {
                std::map<String, String> params;
                params[PARAM_ENABLED] = VALUE_ON;
                params[PARAM_BROKER_HOST] = mqttHost;
                if (mqttPort.length() > 0)
                {
                    params[PARAM_BROKER_PORT] = mqttPort;
                }
                else
                {
                    params[PARAM_BROKER_PORT] = Defaults::DEFAULT_MQTT_PORT;
                }

                params[PARAM_BROKER_USER] = String();
                if(request->hasParam(PARAM_BROKER_USER, true))
                {
                    String mqttUsername = request->getParam(PARAM_BROKER_USER, true)->value();
                    if (mqttUsername.length() > 0)
                    {                        
                        params[PARAM_BROKER_USER] = mqttUsername;
                    }
                }
                
                params[PARAM_BROKER_PASS] = String();
                if(request->hasParam(PARAM_BROKER_PASS, true))
                {
                    String mqttPassword = request->getParam(PARAM_BROKER_PASS, true)->value();
                    if (mqttPassword.length() > 0)
                    {
                        params[PARAM_BROKER_PASS] = mqttPassword;
                    }
                }         

                
                if (mqttTopic.length() > 0)
                {
                    params[PARAM_BROKER_DEFAULT_TOPIC] = mqttTopic;
                }
                else
                {
                    params[PARAM_BROKER_DEFAULT_TOPIC] = Defaults::DEFAULT_MQTT_TOPIC;
                }
                systemControlCallback(SystemOperationType::SetHaIntegration, params);
                request->send(200, CONTENT_TEXT, VALUE_SUCCESS);
                return;
            }
            else
            {
                Serial.println("Invalid MQTT host");
                request->send(400, CONTENT_TEXT, VALUE_ERROR);
                return;
            }
        }
        else
        {
            Serial.println("Invalid HA Integration value");
            request->send(400, CONTENT_TEXT, VALUE_ERROR);
            return;
        }
    } else {
        Serial.println("Invalid request");
        request->send(400, CONTENT_TEXT, VALUE_ERROR);
        return;
    }
}

void WebUI::handleSetClockFace(AsyncWebServerRequest *request)
{
    //TODO: accept different clock faces (eg, languages, styles or sizes from the dropdown)
    if (request->hasParam(PARAM_OPTION, true))
    {
        String option = request->getParam(PARAM_OPTION, true)->value();
        if(option == VALUE_OFF || option == VALUE_ON) {
            std::map<String, String> params;
            params[PARAM_OPTION] = option;
            systemControlCallback(SystemOperationType::SetClockFormat, params);
            request->send(200, CONTENT_TEXT, VALUE_SUCCESS);
            return;
        } 
        else
        {
            Serial.println("Invalid clock face option value");
            request->send(400, CONTENT_TEXT, VALUE_ERROR);
            return;
        }
    }
    else
    {
        Serial.println("Missing clock face parameter");
        request->send(400, CONTENT_TEXT, VALUE_ERROR);
        return;
    }
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
    case Page::TIME:
        return timePageProcessor(var);
    default:
        return String();
    }
}

String WebUI::headerProcessor(Page page)
{
    String headerContent = readFile(PATH_NAVIGATION_HTML);
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
    headerContent.replace("%ACTIVE_LIGHT%", (page == Page::LIGHT) ? VALUE_ACTIVE : "");
    headerContent.replace("%ACTIVE_TIME%", (page == Page::TIME) ? VALUE_ACTIVE : "");
    headerContent.replace("%ACTIVE_SYSTEM%", (page == Page::SYSTEM) ? VALUE_ACTIVE : "");

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

String WebUI::timePageProcessor(const String &var)
{
    if(var == "CURRENT_TIME") {
        return "12:30";
    } else if (var == "TIME_ZONES") {
        /*
            <option value="Etc/UTC">UTC</option>
            <option value="Europe/Berlin">Europe/Berlin</option>
            <option value="Europe/London">Europe/London</option>
            <option value="Europe/Paris">Europe/Paris</option>
            <option value="Europe/Rome">Europe/Rome</option>
            <option value="Atlantic/Reykjavik">Atlantic/Reykjavik</option>

            Etc/UTC -> UTC0
            Europe/Berlin -> CET-1CEST,M3.5.0,M10.5.0/3
            Europe/London -> GMT0BST,M3.5.0/1,M10.5.0
            Europe/Paris -> CET-1CEST,M3.5.0,M10.5.0/3
            Europe/Rome -> CET-1CEST,M3.5.0,M10.5.0/3
            Atlantic/Reykjavik -> GMT0
        */
       return "<option value=\"Europe/Berlin\">Europe/Berlin</option>";
    } else if(var == "NTP_UPDATE_INTERVAL") {
        return String(systemConfiguration->ntpConfig.interval);
    } else if (var == "NTP_UPDATE_STATE") {
        return systemConfiguration->ntpConfig.enabled ? "checked" : "";
    } else if (var == "LIGHT_SCHEDULE_STATE") {
        return systemConfiguration->lightScheduleConfig.enabled ? "checked" : "";
    } else if (var == "LIGHT_SCHEDULE_START") {
        return String(systemConfiguration->lightScheduleConfig.startTime);
    } else if (var == "LIGHT_SCHEDULE_END") {
        return String(systemConfiguration->lightScheduleConfig.endTime);
    }
    
    else {
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
        Serial.printf("Broker IP: %d\n", strlen(systemConfiguration->mqttConfig.host));
        if (strlen(systemConfiguration->mqttConfig.host) <= 1)
        {
            return String();
        }

        return systemConfiguration->mqttConfig.host;
    }
    else if (var == "BROKER_PORT")
    {
        return String(systemConfiguration->mqttConfig.port);
    }
    else if (var == "MQTT_USERNAME")
    {
        if (strlen(systemConfiguration->mqttConfig.username) > 1)
            return systemConfiguration->mqttConfig.username;
    }
    else if (var == "DEFAULT_TOPIC")
    {
        return systemConfiguration->mqttConfig.topic;
    }
    else if (var == "CLOCK_FACE_OPTION_STATE") {
        return systemConfiguration->mode == Configuration::ClockMode::Regular ? "" : "checked";
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

void WebUI::printAllParams(AsyncWebServerRequest *request)
{
    Serial.println("Parameters found in request:");
    int params = request->params();
    for (int i = 0; i < params; i++)
    {
        const AsyncWebParameter *p = request->getParam(i);
        Serial.printf("PARAM[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
}
