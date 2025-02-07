#include "webui.h"

// paths
const char WebUI::PATH_NAVIGATION_HTML[] PROGMEM = "/navigation.html";
const char WebUI::PATH_LIGHT_HTML[] PROGMEM = "/light.html";
const char WebUI::PATH_TIME_HTML[] PROGMEM = "/time.html";
const char WebUI::PATH_SYSTEM_HTML[] PROGMEM = "/system.html";
const char WebUI::PATH_FIRMWARE_HTML[] PROGMEM = "/firmware.html";
const char WebUI::PATH_WIFI_HTML[] PROGMEM = "/wifimanager.html";

// page titles
const char WebUI::LIGHT_PAGE_TITLE[] PROGMEM = "Light Settings";
const char WebUI::SYSTEM_PAGE_TITLE[] PROGMEM = "System Configuration";
const char WebUI::TIME_PAGE_TITLE[] PROGMEM = "Time Configuration";
const char WebUI::FIRMWARE_PAGE_TITLE[] PROGMEM = "Firmware Update";

// processor variables
const char WebUI::PROC_PAGE_TITLE[] PROGMEM = "PAGE_TITLE";
const char WebUI::PROC_FW_VERS[] PROGMEM = "FW_VERSION";

// values
const char WebUI::VALUE_SUCCESS[] PROGMEM = "Success";
const char WebUI::VALUE_ERROR[] PROGMEM = "Error!";
const char WebUI::VALUE_FIRMWARE[] PROGMEM = "firmware";
const char WebUI::VALUE_FILESYS[] PROGMEM = "filesystem";

const char WebUI::CONTENT_TEXT[] PROGMEM = "text/plain";
const char WebUI::CONTENT_HTML[] PROGMEM = "text/html";

const char WebUI::PARAM_FW_Type[] PROGMEM = "updateType";

const char WebUI::VALUE_ON[] PROGMEM = "1";
const char WebUI::VALUE_OFF[] PROGMEM = "0";

const char WebUI::PARAM_WIFI_SSID[] PROGMEM = "ssid";
const char WebUI::PARAM_WIFI_PASS[] PROGMEM = "wifi-pass";
const char WebUI::PARAM_ENABLED[] PROGMEM = "enabled";
const char WebUI::PARAM_OPTION[] PROGMEM = "option";
const char WebUI::PARAM_VALUE[] PROGMEM = "value";
const char WebUI::PARAM_COLOR[] PROGMEM = "color";
const char WebUI::PARAM_BRIGHTNESS[] PROGMEM = "brightness";
const char WebUI::PARAM_AUTO_BRIGHTNESS_ENABLED[] PROGMEM = "abEnabled";
const char WebUI::PARAM_TIME[] PROGMEM = "time";
const char WebUI::PARAM_BROKER_ENABLED[] PROGMEM = "mqttEnabled";
const char WebUI::PARAM_BROKER_HOST[] PROGMEM = "mqttHost";
const char WebUI::PARAM_BROKER_PORT[] PROGMEM = "mqttPort";
const char WebUI::PARAM_BROKER_USER[] PROGMEM = "mqttUsername";
const char WebUI::PARAM_BROKER_PASS[] PROGMEM = "mqttPassword";
const char WebUI::PARAM_BROKER_DEFAULT_TOPIC[] PROGMEM = "mqttTopic";
const char WebUI::PARAM_NTP_ENABLED[] PROGMEM = "ntpEnabled";
const char WebUI::PARAM_NTP_HOST[] PROGMEM = "ntpHost";
const char WebUI::PARAM_NTP_UPDATE_INTERVAL[] PROGMEM = "ntpInterval";
const char WebUI::PARAM_NTP_TIMEZONE[] PROGMEM = "ntpTimezone";
const char WebUI::PARAM_SCHEDULE_ENABLED[] PROGMEM = "scheduleEnabled";
const char WebUI::PARAM_SCHEDULE_START[] PROGMEM = "scheduleStart";
const char WebUI::PARAM_SCHEDULE_END[] PROGMEM = "scheduleEnd";
const char WebUI::PARAM_CLOCKFACE[] PROGMEM = "clockFace";        
const char WebUI::PARAM_CLOCKFACE_OPTION[] PROGMEM = "clockFaceOption";
const char WebUI::PARAM_FW_VERSION[] PROGMEM = "fwVersion";


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
              { 
                const std::map<String, String> params = responseCallback(PageType::LIGHT);
                request->send(
                    LittleFS,
                    FPSTR(PATH_LIGHT_HTML),
                    FPSTR(CONTENT_HTML),
                    false,
                    [this, params](const String &var) -> String
                    {
                        return this->pageProcessor(var, PageType::LIGHT, params);
                    }); 
                });

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
              { 
                const std::map<String, String> params = responseCallback(PageType::TIME);
                // TODO: call responseCallback to get the details only once and provide parameter list to page processor
                request->send(
                    LittleFS,
                    FPSTR(PATH_TIME_HTML),
                    FPSTR(CONTENT_HTML),
                    false,
                    [this, params](const String &var) -> String
                    {
                        return this->pageProcessor(var, PageType::TIME, params);
                    }); });

    server.on("/getCurrentTime", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, FPSTR(CONTENT_TEXT), "12:30"); });

    server.on("/system", HTTP_GET, [this](AsyncWebServerRequest *request)
              { 
                const std::map<String, String> params = responseCallback(PageType::SYSTEM);
                request->send(
                    LittleFS,
                    FPSTR(PATH_SYSTEM_HTML),
                    FPSTR(CONTENT_HTML),
                    false,
                    [this, params](const String &var) -> String
                    {
                        return this->pageProcessor(var, PageType::SYSTEM, params);
                    }); });

    server.on("/setHaIntegration", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->handleSetHAIntegration(request); });

    server.on("/setClockFace", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->handleSetClockFace(request); });

    server.on("/resetConfig", HTTP_POST, [this](AsyncWebServerRequest *request)
              { requestCallback(ControlType::ResetConfig, std::map<String, String>()); });

    server.on("/update", HTTP_GET, [this](AsyncWebServerRequest *request)
              { 
                const std::map<String, String> params = responseCallback(PageType::FWUPDATE);
                request->send(
                    LittleFS,
                    FPSTR(PATH_FIRMWARE_HTML),
                    FPSTR(CONTENT_HTML),
                    false,
                    [this, params](const String &var) -> String
                    {
                        return this->pageProcessor(var, PageType::FWUPDATE, params);
                    }); });

    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest *request)
              { handleFirmwareUpdate(request); }, [this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
              { 
                UpdateType type = UpdateType::FIRMWARE;
                if (index == 0 && request->hasParam(FPSTR(PARAM_FW_Type), true)) {  
                    //const AsyncWebParameter *p = request->getParam(PARAM_FW_Type, true);  
                    //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                    const String tParam = request->getParam(FPSTR(PARAM_FW_Type), true)->value();
                    //Serial.printf("Update type: %s\n", tParam.c_str());
                    if(tParam == FPSTR(VALUE_FIRMWARE)) {
                        type = UpdateType::FIRMWARE;
                    } else if(tParam == FPSTR(VALUE_FILESYS)) {
                        type = UpdateType::FILESYSTEM;
                    } else {
                        Serial.println("Invalid update type");
                        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
                    }
                }
                //Serial.printf("Type: %u, Update: %s, Index: %u, Len: %u, Final: %u\n", type, filename.c_str(), index, len, final);
                updateCallback(type, filename, index, data, len, final); });

    // Other routes with sanitized handlers
    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR)); });

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
                    FPSTR(PATH_WIFI_HTML),
                    FPSTR(CONTENT_HTML)); });

    server.on("/", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
        String ssid, password;

        if(request->hasParam(FPSTR(PARAM_WIFI_SSID), true)) {
            std::map<String, String> params;
            params[FPSTR(PARAM_ENABLED)] = FPSTR(VALUE_ON);
            params[FPSTR(PARAM_WIFI_SSID)] = request->getParam(FPSTR(PARAM_WIFI_SSID), true)->value();
            if(request->hasParam(FPSTR(PARAM_WIFI_PASS), true)) {
                params[FPSTR(PARAM_WIFI_PASS)] = request->getParam(FPSTR(PARAM_WIFI_PASS), true)->value();
            } else {
                params[FPSTR(PARAM_WIFI_PASS)] = String();
            }

            // Use the callback with the received SSID and password
            requestCallback(ControlType::WiFiSetup, params);
            request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
            delay(3000);
            ESP.restart();
        } else {
                Serial.println("Missing SSID parameter");
                request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
                return;
        } });

    // Serve Static CSS and JS only
    server.serveStatic(PATH_CSS, LittleFS, PATH_CSS).setCacheControl(CONTENT_CACHE);
    server.serveStatic(PATH_JS, LittleFS, PATH_JS).setCacheControl(CONTENT_CACHE);
    server.serveStatic(PATH_ICON, LittleFS, PATH_ICON).setCacheControl(CONTENT_CACHE);
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
        request->send(500, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
    }
}

void WebUI::handleToggleLight(AsyncWebServerRequest *request)
{
    if (request->hasParam(FPSTR(PARAM_ENABLED)))
    {
        String statusParam = request->getParam(FPSTR(PARAM_ENABLED))->value();

        // Validate status parameter
        if (statusParam == FPSTR(VALUE_OFF) || statusParam == FPSTR(VALUE_ON))
        {
            std::map<String, String> params;
            params[FPSTR(PARAM_ENABLED)] = statusParam;
            requestCallback(ControlType::LightStatus, params);

            request->send(200, CONTENT_TEXT, FPSTR(VALUE_SUCCESS));
        }
        else
        {
            Serial.println("Invalid status value");
            request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
        }
    }
    else
    {
        Serial.println("Missing status parameter");
        request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
    }
}

void WebUI::handleSetLightColor(AsyncWebServerRequest *request)
{
    if (request->hasParam(FPSTR(PARAM_COLOR)))
    {
        String colorParam = request->getParam(FPSTR(PARAM_COLOR))->value();
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
                std::map<String, String> params;
                params[FPSTR(PARAM_COLOR)] = colorParam;
                requestCallback(ControlType::Color, params);
                request->send(200, CONTENT_TEXT, FPSTR(VALUE_SUCCESS));
                return;
            }
            else
            {
                Serial.println("Invalid color format");
                request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
                return;
            }
        }
        else
        {
            Serial.println("Invalid color format");
            request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Missing color parameter");
        request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
        return;
    }
}

void WebUI::handleSetAutoBrightness(AsyncWebServerRequest *request)
{
    if (request->hasParam(FPSTR(PARAM_ENABLED)))
    {
        String enabledParam = request->getParam(FPSTR(PARAM_ENABLED))->value();

        // Validate enabled parameter
        if (enabledParam == FPSTR(VALUE_OFF) || enabledParam == FPSTR(VALUE_ON))
        {
            std::map<String, String> params;
            params[FPSTR(PARAM_AUTO_BRIGHTNESS_ENABLED)] = enabledParam;

            requestCallback(ControlType::AutoBrightness, params);
            request->send(200, CONTENT_TEXT, FPSTR(VALUE_SUCCESS));
            return;
        }
        else
        {
            Serial.println("Invalid enabled value");
            request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Missing enabled parameter");
        request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
        return;
    }
}

void WebUI::handleSetBrightness(AsyncWebServerRequest *request)
{
    if (request->hasParam(FPSTR(PARAM_VALUE)))
    {
        String valueParam = request->getParam(FPSTR(PARAM_VALUE))->value();

        // Validate brightness value (should be an integer between 0 and 255)
        int brightness = valueParam.toInt();
        if (brightness >= 0 && brightness <= 255)
        {
            std::map<String, String> params;
            params[FPSTR(PARAM_BRIGHTNESS)] = valueParam;
            requestCallback(ControlType::Brightness, params);
            request->send(200, CONTENT_TEXT, FPSTR(VALUE_SUCCESS));
            return;
        }
        else
        {
            Serial.println("Invalid brightness value");
            request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Missing value parameter");
        request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
        return;
    }
}

void WebUI::handleSetHAIntegration(AsyncWebServerRequest *request)
{
    printAllParams(request);
    if (request->hasParam(FPSTR(PARAM_ENABLED), true))
    {
        String haIntegrationParam = request->getParam(FPSTR(PARAM_ENABLED), true)->value();
        if (haIntegrationParam == FPSTR(VALUE_OFF))
        {
            std::map<String, String> params;
            params[FPSTR(PARAM_ENABLED)] = FPSTR(VALUE_OFF);
            params[FPSTR(PARAM_BROKER_HOST)] = String();
            params[FPSTR(PARAM_BROKER_PORT)] = String();
            params[FPSTR(PARAM_BROKER_USER)] = String();
            params[FPSTR(PARAM_BROKER_PASS)] = String();
            params[FPSTR(PARAM_BROKER_DEFAULT_TOPIC)] = String();
            requestCallback(ControlType::HaIntegration, params);
            request->send(200, CONTENT_TEXT, FPSTR(VALUE_SUCCESS));
            return;
        }
        else if (haIntegrationParam == FPSTR(VALUE_ON))
        {
            // TODO: check if parameters exist
            if (!request->hasParam(FPSTR(PARAM_BROKER_HOST), true) ||
                !request->hasParam(FPSTR(PARAM_BROKER_PORT), true) ||
                !request->hasParam(FPSTR(PARAM_BROKER_DEFAULT_TOPIC), true))
            {
                Serial.println("Missing parameters");
                request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
                return;
            }

            String mqttHost = request->getParam(FPSTR(PARAM_BROKER_HOST), true)->value();
            String mqttPort = request->getParam(FPSTR(PARAM_BROKER_PORT), true)->value();
            String mqttTopic = request->getParam(FPSTR(PARAM_BROKER_DEFAULT_TOPIC), true)->value();
            if (mqttHost.length() > 0)
            {
                std::map<String, String> params;
                params[FPSTR(PARAM_ENABLED)] = FPSTR(VALUE_ON);
                params[FPSTR(PARAM_BROKER_HOST)] = mqttHost;
                if (mqttPort.length() > 0)
                {
                    params[FPSTR(PARAM_BROKER_PORT)] = mqttPort;
                }
                else
                {
                    params[FPSTR(PARAM_BROKER_PORT)] = Defaults::DEFAULT_MQTT_PORT;
                }

                params[FPSTR(PARAM_BROKER_USER)] = String();
                if (request->hasParam(FPSTR(PARAM_BROKER_USER), true))
                {
                    String mqttUsername = request->getParam(FPSTR(PARAM_BROKER_USER), true)->value();
                    if (mqttUsername.length() > 0)
                    {
                        params[FPSTR(PARAM_BROKER_USER)] = mqttUsername;
                    }
                }

                params[FPSTR(PARAM_BROKER_PASS)] = String();
                if (request->hasParam(FPSTR(PARAM_BROKER_PASS), true))
                {
                    String mqttPassword = request->getParam(FPSTR(PARAM_BROKER_PASS), true)->value();
                    if (mqttPassword.length() > 0)
                    {
                        params[FPSTR(PARAM_BROKER_PASS)] = mqttPassword;
                    }
                }

                if (mqttTopic.length() > 0)
                {
                    params[FPSTR(PARAM_BROKER_DEFAULT_TOPIC)] = mqttTopic;
                }
                else
                {
                    params[FPSTR(PARAM_BROKER_DEFAULT_TOPIC)] = Defaults::DEFAULT_MQTT_TOPIC;
                }
                requestCallback(ControlType::HaIntegration, params);
                request->send(200, CONTENT_TEXT, FPSTR(VALUE_SUCCESS));
                return;
            }
            else
            {
                Serial.println("Invalid MQTT host");
                request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
                return;
            }
        }
        else
        {
            Serial.println("Invalid HA Integration value");
            request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Invalid request");
        request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
        return;
    }
}

void WebUI::handleSetClockFace(AsyncWebServerRequest *request)
{
    // TODO: accept different clock faces (eg, languages, styles or sizes from the dropdown)
    if (request->hasParam(FPSTR(PARAM_OPTION), true))
    {
        String option = request->getParam(FPSTR(PARAM_OPTION), true)->value();
        if (option == FPSTR(VALUE_OFF) || option == FPSTR(VALUE_ON))
        {
            std::map<String, String> params;
            params[FPSTR(PARAM_OPTION)] = option;
            requestCallback(ControlType::ClockFace, params);
            request->send(200, CONTENT_TEXT, FPSTR(VALUE_SUCCESS));
            return;
        }
        else
        {
            Serial.println("Invalid clock face option value");
            request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Missing clock face parameter");
        request->send(400, CONTENT_TEXT, FPSTR(VALUE_ERROR));
        return;
    }
}

String WebUI::pageProcessor(const String &var, PageType page, const std::map<String, String> &params)
{
    if (var == "INCLUDE_HEADER")
    {
        return headerProcessor(page);
    }

    switch (page)
    {
    case PageType::LIGHT:
        return lightPageProcessor(var, params);
    case PageType::SYSTEM:
        return systemPageProcessor(var, params);
    case PageType::FWUPDATE:
        return firmwarePageProcessor(var, params);
    case PageType::TIME:
        return timePageProcessor(var, params);
    default:
        return String();
    }
}

String WebUI::headerProcessor(PageType page)
{
    String headerContent = readFile(PATH_NAVIGATION_HTML);
    if (headerContent.length() == 0)
    {
        return String();
    }

    String pageTitle;
    switch (page)
    {
    case PageType::LIGHT:
        pageTitle = FPSTR(LIGHT_PAGE_TITLE);
        break;
    case PageType::SYSTEM:
        pageTitle = FPSTR(SYSTEM_PAGE_TITLE);
        break;
    case PageType::TIME:
        pageTitle = FPSTR(TIME_PAGE_TITLE);
        break;
    case PageType::FWUPDATE:
        pageTitle = FPSTR(FIRMWARE_PAGE_TITLE);
        break;
    default:
        pageTitle = "Unknown";
        break;
    }

    headerContent.replace("%PAGE_TITLE%", pageTitle);
    headerContent.replace("%ACTIVE_LIGHT%", (page == PageType::LIGHT) ? VALUE_ACTIVE : "");
    headerContent.replace("%ACTIVE_TIME%", (page == PageType::TIME) ? VALUE_ACTIVE : "");
    headerContent.replace("%ACTIVE_SYSTEM%", (page == PageType::SYSTEM) ? VALUE_ACTIVE : "");

    return headerContent;
}

String WebUI::lightPageProcessor(const String &var, const std::map<String, String> &params)
{
    if (var == "LIGHT_STATE")
    {
        return params.at(FPSTR(PARAM_ENABLED)) == FPSTR(VALUE_ON) ? "checked" : "";
    }
    else if (var == "LIGHT_COLOR")
    {
        return params.at(FPSTR(PARAM_COLOR)); 
    }
    else if (var == "AUTO_BRIGHTNESS_STATE")
    {

        return params.at(FPSTR(PARAM_AUTO_BRIGHTNESS_ENABLED)) == FPSTR(VALUE_ON) ? "checked" : "";
    }
    else if (var == "LIGHT_BRIGHTNESS")
    {
        return params.at(FPSTR(PARAM_BRIGHTNESS));
    }
    else
    {
        return String();
    }
}

String WebUI::timePageProcessor(const String &var, const std::map<String, String> &params)
{
    //std::map<String, String> params = responseCallback(DetailsType::TimeConfig);
    if (var == "CURRENT_TIME")
    {
        return params.at(FPSTR(PARAM_TIME));
    }
    else if (var == "TIME_ZONES")
    {
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
        // return params[FPSTR(PARAM_NTP_TIMEZONE)];
        return "<option value=\"Europe/Berlin\">Europe/Berlin</option>";
    }
    else if (var == "NTP_UPDATE_INTERVAL")
    {
        return params.at(FPSTR(PARAM_NTP_UPDATE_INTERVAL));
    }
    else if (var == "NTP_UPDATE_STATE")
    {
        return params.at(FPSTR(PARAM_NTP_ENABLED)) == FPSTR(VALUE_ON) ? "checked" : "";
    }
    else if (var == "LIGHT_SCHEDULE_STATE")
    {
        return params.at(FPSTR(PARAM_SCHEDULE_ENABLED)) == FPSTR(VALUE_ON) ? "checked" : "";
    }
    else if (var == "LIGHT_SCHEDULE_START")
    {
        return params.at(FPSTR(PARAM_SCHEDULE_START));
    }
    else if (var == "LIGHT_SCHEDULE_END")
    {
        return params.at(FPSTR(PARAM_SCHEDULE_END));
    }

    else
    {
        return String();
    }
}

String WebUI::systemPageProcessor(const String &var, const std::map<String, String> &params)
{
    //std::map<String, String> params = responseCallback(DetailsType::SystemConfig);
    if (var == "HA_INTEGRATION_STATE")
    {
        return params.at(FPSTR(PARAM_BROKER_ENABLED)) == FPSTR(VALUE_ON) ? "checked" : "";
    }
    else if (var == "BROKER_IP")
    {
        // Serial.printf("Broker IP: %d\n", strlen(systemConfiguration->mqttConfig.host));
        // if (strlen(systemConfiguration->mqttConfig.host) <= 1)
        // {
        //     return String();
        // }

        return params.at(FPSTR(PARAM_BROKER_HOST));
    }
    else if (var == "BROKER_PORT")
    {
        return params.at(FPSTR(PARAM_BROKER_PORT));
    }
    else if (var == "MQTT_USERNAME")
    {
        if (params.at(FPSTR(PARAM_BROKER_USER)).length() > 0)
            return params.at(FPSTR(PARAM_BROKER_USER));
    }
    else if (var == "DEFAULT_TOPIC")
    {
        return params.at(FPSTR(PARAM_BROKER_DEFAULT_TOPIC));
    }
    else if (var == "CLOCK_FACE_OPTION_STATE")
    {
        return params.at(FPSTR(PARAM_CLOCKFACE_OPTION)) == FPSTR(VALUE_OFF) ? "" : "checked";
    }
    else
    {
        return String();
    }

    return String();
}

String WebUI::firmwarePageProcessor(const String &var, const std::map<String, String> &params)
{
    if (var == "FW_VERSION")
    {
        return params.at(FPSTR(PARAM_FW_VERSION));
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
