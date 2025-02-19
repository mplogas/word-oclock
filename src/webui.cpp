#include "webui.h"

// paths
const char WebUI::PATH_NAVIGATION_HTML[] PROGMEM = "/navigation.html";
const char WebUI::PATH_LIGHT_HTML[] PROGMEM = "/light.html";
const char WebUI::PATH_TIME_HTML[] PROGMEM = "/time.html";
const char WebUI::PATH_SYSTEM_HTML[] PROGMEM = "/system.html";
const char WebUI::PATH_FIRMWARE_HTML[] PROGMEM = "/firmware.html";
const char WebUI::PATH_WIFI_HTML[] PROGMEM = "/wifimanager.html";
const char WebUI::PATH_CSS[] PROGMEM = "/style.css";
const char WebUI::PATH_JS[] PROGMEM = "/index.js";
const char WebUI::PATH_ICON[] PROGMEM = "/favicon.ico";

// page titles
const char WebUI::LIGHT_PAGE_TITLE[] PROGMEM = "Light Settings";
const char WebUI::SYSTEM_PAGE_TITLE[] PROGMEM = "System Configuration";
const char WebUI::TIME_PAGE_TITLE[] PROGMEM = "Time Configuration";
const char WebUI::FIRMWARE_PAGE_TITLE[] PROGMEM = "Firmware Update";

// processor variables
const char WebUI::PROC_PAGE_TITLE[] PROGMEM = "%PAGE_TITLE%";
const char WebUI::PROC_ACTIVE_LIGHT[] PROGMEM = "%ACTIVE_LIGHT%";
const char WebUI::PROC_ACTIVE_TIME[] PROGMEM = "%ACTIVE_TIME%";
const char WebUI::PROC_ACTIVE_SYSTEM[] PROGMEM = "%ACTIVE_SYSTEM%";
const char WebUI::PROC_LIGHT_STATE[] PROGMEM = "LIGHT_STATE";
const char WebUI::PROC_LIGHT_COLOR[] PROGMEM = "LIGHT_COLOR";
const char WebUI::PROC_LIGHT_BRIGHTNESS[] PROGMEM = "LIGHT_BRIGHTNESS";
const char WebUI::PROC_AUTO_BRIGHTNESS_ENABLED[] PROGMEM = "AUTO_BRIGHTNESS_ENABLED";
const char WebUI::PROC_TIME[] PROGMEM = "CURRENT_TIME"; 
const char WebUI::PROC_NTP_ENABLED[] PROGMEM = "NTP_ENABLED";
const char WebUI::PROC_NTP_HOST[] PROGMEM = "NTP_HOST";
const char WebUI::PROC_NTP_UPDATE_INTERVAL[] PROGMEM = "NTP_UPDATE_INTERVAL";
const char WebUI::PROC_NTP_TIMEZONE[] PROGMEM = "NTP_TIMEZONE";
const char WebUI::PROC_BROKER_ENABLED[] PROGMEM = "BROKER_ENABLED";
const char WebUI::PROC_BROKER_HOST[] PROGMEM = "BROKER_HOST";
const char WebUI::PROC_BROKER_PORT[] PROGMEM = "BROKER_PORT";
const char WebUI::PROC_BROKER_USER[] PROGMEM = "BROKER_USER";
const char WebUI::PROC_BROKER_PASS[] PROGMEM = "BROKER_PASS";
const char WebUI::PROC_BROKER_DEFAULT_TOPIC[] PROGMEM = "BROKER_DEFAULT_TOPIC";
const char WebUI::PROC_SCHEDULE_ENABLED[] PROGMEM = "SCHEDULE_ENABLED";
const char WebUI::PROC_SCHEDULE_START[] PROGMEM = "SCHEDULE_START";
const char WebUI::PROC_SCHEDULE_END[] PROGMEM = "SCHEDULE_END";
const char WebUI::PROC_CLOCKFACE[] PROGMEM = "CLOCKFACE";
const char WebUI::PROC_CLOCKFACE_OPTION_STATE[] PROGMEM = "CLOCKFACE_OPTION_STATE";
const char WebUI::PROC_FW_VERS[] PROGMEM = "FW_VERSION";

// values
const char WebUI::VALUE_SUCCESS[] PROGMEM = "Success";
const char WebUI::VALUE_ERROR[] PROGMEM = "Error!";
const char WebUI::VALUE_CHECKED[] PROGMEM = "checked";
const char WebUI::VALUE_ACTIVE[] PROGMEM = "active";
const char WebUI::VALUE_EMPTY[] PROGMEM = "";
const char WebUI::VALUE_FIRMWARE[] PROGMEM = "firmware";
const char WebUI::VALUE_FILESYS[] PROGMEM = "filesystem";

const char WebUI::CONTENT_TEXT[] PROGMEM = "text/plain";
const char WebUI::CONTENT_HTML[] PROGMEM = "text/html";
const char WebUI::CONTENT_CACHE[] PROGMEM = "max-age=86400";

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

    // server.on("/getCurrentTime", HTTP_GET, [this](AsyncWebServerRequest *request)
    //           { request->send(200, FPSTR(CONTENT_TEXT), "12:30"); });
    server.on("/setTime", HTTP_POST, [this](AsyncWebServerRequest *request)
    { this->handleSetTime(request); });

    server.on("/setLightSchedule", HTTP_POST, [this](AsyncWebServerRequest *request)
    { this->handleSetLightSchedule(request); });

    server.on("/setNTPConfig", HTTP_POST, [this](AsyncWebServerRequest *request)
    { this->handleSetNTPConfig(request); });

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

            request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
        }
        else
        {
            Serial.println("Invalid status value");
            request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
        }
    }
    else
    {
        Serial.println("Missing status parameter");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
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
                request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
                return;
            }
            else
            {
                Serial.println("Invalid color format");
                request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
                return;
            }
        }
        else
        {
            Serial.println("Invalid color format");
            request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Missing color parameter");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
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
            request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
            return;
        }
        else
        {
            Serial.println("Invalid enabled value");
            request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Missing enabled parameter");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
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
            request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
            return;
        }
        else
        {
            Serial.println("Invalid brightness value");
            request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Missing value parameter");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
        return;
    }
}

void WebUI::handleSetTime(AsyncWebServerRequest *request) {
    //printAllParams(request);
    if (request->hasParam(FPSTR(PARAM_TIME), true)) {
        String timeParam = request->getParam(FPSTR(PARAM_TIME), true)->value();
        // Validate time format (expecting HH:MM)
        if (timeParam.length() == 5 && timeParam[2] == ':') {
            bool valid = true;
            for (size_t i = 0; i < timeParam.length(); i++) {
                char c = timeParam[i];
                if (i == 2) {
                    continue;
                }
                if (!isdigit(c)) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                std::map<String, String> params;
                params[FPSTR(PARAM_TIME)] = timeParam;
                requestCallback(ControlType::Time, params);
                request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
                return;
            } 
        } 

        Serial.println("Invalid time format");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
        return;

    } else {
        Serial.println("Missing time parameter");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
        return;
    }
}

void WebUI::handleSetLightSchedule(AsyncWebServerRequest *request)
{
    if (request->hasParam(FPSTR(PARAM_ENABLED), true))
    {
        String scheduleEnabledParam = request->getParam(FPSTR(PARAM_ENABLED), true)->value();
        if (scheduleEnabledParam == FPSTR(VALUE_OFF)) {
            std::map<String, String> params;
            params[FPSTR(PARAM_SCHEDULE_ENABLED)] = FPSTR(VALUE_OFF);
            params[FPSTR(PARAM_SCHEDULE_START)] = String();
            params[FPSTR(PARAM_SCHEDULE_END)] = String();
            requestCallback(ControlType::LightSchedule, params);
            request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
            return;
        } else if (scheduleEnabledParam == FPSTR(VALUE_ON)) {
            if (!request->hasParam(FPSTR(PARAM_SCHEDULE_START), true) || !request->hasParam(FPSTR(PARAM_SCHEDULE_END), true)) {
                Serial.println("Missing parameters");
                request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
                return;
            }
            String scheduleStart = request->getParam(FPSTR(PARAM_SCHEDULE_START), true)->value();
            String scheduleEnd = request->getParam(FPSTR(PARAM_SCHEDULE_END), true)->value();
            //Serial.printf("Start: %s, End: %s\n", scheduleStart.c_str(), scheduleEnd.c_str());

            // Validate schedule format (expecting HH:MM)
            if (scheduleStart.length() == 5 && scheduleStart[2] == ':' && scheduleEnd.length() == 5 && scheduleEnd[2] == ':') {
                bool valid = true;
                for (size_t i = 0; i < scheduleStart.length(); i++) {
                    char c = scheduleStart[i];
                    if (i == 2) {
                        continue;
                    }
                    if (!isdigit(c)) {
                        valid = false;
                        break;
                    }
                }

                for (size_t i = 0; i < scheduleEnd.length(); i++) {
                    char c = scheduleEnd[i];
                    if (i == 2) {
                        continue;
                    }
                    if (!isdigit(c)) {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    uint32_t startHour = scheduleStart.substring(0, 2).toInt();
                    uint32_t startMinute = scheduleStart.substring(3, 5).toInt();
                    uint32_t endHour = scheduleEnd.substring(0, 2).toInt();
                    uint32_t endMinute = scheduleEnd.substring(3, 5).toInt();
    
                    if(startHour < 24 || startMinute < 60 || endHour < 24 || endMinute < 60 ) {
                        uint32_t startSeconds = startHour * 3600UL + startMinute * 60UL;
                        uint32_t endSeconds = endHour * 3600UL + endMinute * 60UL;
        
                        std::map<String, String> params;
                        params[FPSTR(PARAM_SCHEDULE_ENABLED)] = FPSTR(VALUE_ON);
                        params[FPSTR(PARAM_SCHEDULE_START)] = String(startSeconds);
                        params[FPSTR(PARAM_SCHEDULE_END)] = String(endSeconds);
                        requestCallback(ControlType::LightSchedule, params);
                        request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
                        return;
                    }
                }
            } 
        } 
        Serial.println("Invalid schedule value");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
        return;
    } else {
        Serial.println("Invalid request");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
        return;
    }
}

void WebUI::handleSetNTPConfig(AsyncWebServerRequest *request)
{
    if (request->hasParam(FPSTR(PARAM_ENABLED), true))
    {
        String ntpEnabledParam = request->getParam(FPSTR(PARAM_ENABLED), true)->value();
        if (ntpEnabledParam == FPSTR(VALUE_OFF))
        {
            std::map<String, String> params;
            params[FPSTR(PARAM_NTP_ENABLED)] = FPSTR(VALUE_OFF);
            params[FPSTR(PARAM_NTP_HOST)] = String();
            params[FPSTR(PARAM_NTP_UPDATE_INTERVAL)] = String();
            params[FPSTR(PARAM_NTP_TIMEZONE)] = String();
            requestCallback(ControlType::NTPSync, params);
            request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
            return;
        }
        else if (ntpEnabledParam == FPSTR(VALUE_ON))
        {
            if(request->hasParam(FPSTR(PARAM_NTP_HOST), true) && request->hasParam(FPSTR(PARAM_NTP_UPDATE_INTERVAL), true) && request->hasParam(FPSTR(PARAM_NTP_TIMEZONE), true))
            {
                String ntpHost = request->getParam(FPSTR(PARAM_NTP_HOST), true)->value();
                String ntpInterval = request->getParam(FPSTR(PARAM_NTP_UPDATE_INTERVAL), true)->value();
                String ntpTimezone = request->getParam(FPSTR(PARAM_NTP_TIMEZONE), true)->value();
                if (ntpHost.length() > 0 && ntpInterval.length() > 0 && ntpTimezone.length() > 0)
                {
                    std::map<String, String> params;
                    params[FPSTR(PARAM_NTP_ENABLED)] = FPSTR(VALUE_ON);
                    params[FPSTR(PARAM_NTP_HOST)] = ntpHost;
                    params[FPSTR(PARAM_NTP_UPDATE_INTERVAL)] = ntpInterval;
                    params[FPSTR(PARAM_NTP_TIMEZONE)] = ntpTimezone;
                    requestCallback(ControlType::NTPSync, params);
                    request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
                    return;
                }
            }
        }
            
        Serial.println("Invalid NTP parameters");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
        return;
    }
    Serial.println("Invalid request");
    request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
    return;
}

void WebUI::handleSetHAIntegration(AsyncWebServerRequest *request)
{
    //printAllParams(request);
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
            request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
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
                request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
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
                request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
                return;
            }
            else
            {
                Serial.println("Invalid MQTT host");
                request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
                return;
            }
        }
        else
        {
            Serial.println("Invalid HA Integration value");
            request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Invalid request");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
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
            request->send(200, FPSTR(CONTENT_TEXT), FPSTR(VALUE_SUCCESS));
            return;
        }
        else
        {
            Serial.println("Invalid clock face option value");
            request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
            return;
        }
    }
    else
    {
        Serial.println("Missing clock face parameter");
        request->send(400, FPSTR(CONTENT_TEXT), FPSTR(VALUE_ERROR));
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

    headerContent.replace(FPSTR(PROC_PAGE_TITLE), pageTitle);
    headerContent.replace(FPSTR(PROC_ACTIVE_LIGHT), (page == PageType::LIGHT) ? FPSTR(VALUE_ACTIVE) : FPSTR(VALUE_EMPTY));
    headerContent.replace(FPSTR(PROC_ACTIVE_TIME), (page == PageType::TIME) ? FPSTR(VALUE_ACTIVE) : FPSTR(VALUE_EMPTY));
    headerContent.replace(FPSTR(PROC_ACTIVE_SYSTEM), (page == PageType::SYSTEM) ? FPSTR(VALUE_ACTIVE) : FPSTR(VALUE_EMPTY));

    return headerContent;
}

String WebUI::lightPageProcessor(const String &var, const std::map<String, String> &params)
{
    if (var == FPSTR(PROC_LIGHT_STATE))
    {
        return params.at(FPSTR(PARAM_ENABLED)) == FPSTR(VALUE_ON) ? FPSTR(VALUE_CHECKED) : FPSTR(VALUE_EMPTY);
    }
    else if (var == FPSTR(PROC_LIGHT_COLOR))
    {
        return params.at(FPSTR(PARAM_COLOR)); 
    }
    else if (var == FPSTR(PROC_AUTO_BRIGHTNESS_ENABLED))
    {
        return params.at(FPSTR(PARAM_AUTO_BRIGHTNESS_ENABLED)) == FPSTR(VALUE_ON) ? FPSTR(VALUE_CHECKED) : FPSTR(VALUE_EMPTY);
    }
    else if (var == FPSTR(PROC_LIGHT_BRIGHTNESS))
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
    if (var == FPSTR(PROC_TIME))
    {
        return params.at(FPSTR(PARAM_TIME));
    }
    else if (var == FPSTR(PROC_NTP_ENABLED))
    {
        return params.at(FPSTR(PARAM_NTP_ENABLED)) == FPSTR(VALUE_ON) ? FPSTR(VALUE_CHECKED) : FPSTR(VALUE_EMPTY);
    }
    else if (var == FPSTR(PROC_NTP_HOST))
    {
        return params.at(FPSTR(PARAM_NTP_HOST));
    }
    else if (var == FPSTR(PROC_NTP_TIMEZONE))
    {
        const String currentTz = params.at(FPSTR(PARAM_NTP_TIMEZONE));
        String options;
        char buffer[64];  // Adjust size based on your longest timezone string
        
        for(size_t i = 0; i < TZ_COUNT; i++) {
            // Read name and posixTz from PROGMEM one at a time
            strcpy_P(buffer, (char*)pgm_read_ptr(&(TIMEZONES[i].name)));
            options += F("<option value=\"");
            options += buffer;
            options += F("\"");            
            if(String(buffer) == currentTz) {
                options += F(" selected");
            }            
            options += F(">");
            options += buffer;
            options += F("</option>");
            
            // Optional: yield to prevent watchdog timer from triggering
            //if(i % 5 == 0) yield();
        }
        return options;
    }
    else if (var == FPSTR(PROC_NTP_UPDATE_INTERVAL))
    {
        return params.at(FPSTR(PARAM_NTP_UPDATE_INTERVAL));
    }
    else if (var == FPSTR(PROC_SCHEDULE_ENABLED))
    {
        return params.at(FPSTR(PARAM_SCHEDULE_ENABLED)) == FPSTR(VALUE_ON) ? FPSTR(VALUE_CHECKED) : FPSTR(VALUE_EMPTY);
    }
    else if (var == FPSTR(PROC_SCHEDULE_START))
    {
        uint32_t seconds = params.at(FPSTR(PARAM_SCHEDULE_START)).toInt();
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        char buffer[6]; // "HH:mm" + null terminator
        sprintf(buffer, "%02d:%02d", hours, minutes);
        return String(buffer);
    }
    else if (var == FPSTR(PROC_SCHEDULE_END))
    {
        uint32_t seconds = params.at(FPSTR(PARAM_SCHEDULE_END)).toInt();
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        char buffer[6]; // "HH:mm" + null terminator
        sprintf(buffer, "%02d:%02d", hours, minutes);
        return String(buffer);
    }
    else
    {
        return String();
    }
}

String WebUI::systemPageProcessor(const String &var, const std::map<String, String> &params)
{
    //std::map<String, String> params = responseCallback(DetailsType::SystemConfig);
    if (var == FPSTR(PROC_BROKER_ENABLED))
    {
        return params.at(FPSTR(PARAM_BROKER_ENABLED)) == FPSTR(VALUE_ON) ? FPSTR(VALUE_CHECKED) : FPSTR(VALUE_EMPTY);
    }
    else if (var == FPSTR(PROC_BROKER_HOST))
    {
        return params.at(FPSTR(PARAM_BROKER_HOST));
    }
    else if (var == FPSTR(PROC_BROKER_PORT))
    {
        return params.at(FPSTR(PARAM_BROKER_PORT));
    }
    else if (var == FPSTR(PROC_BROKER_USER))
    {
        if (params.at(FPSTR(PARAM_BROKER_USER)).length() > 0)
            return params.at(FPSTR(PARAM_BROKER_USER));
    }
    else if (var == FPSTR(PROC_BROKER_DEFAULT_TOPIC))
    {
        return params.at(FPSTR(PARAM_BROKER_DEFAULT_TOPIC));
    }
    // else if (var == FPSTR(PROC_CLOCKFACE))
    // {
    //     return params.at(FPSTR(PARAM_CLOCKFACE));
    // }
    else if (var == FPSTR(PROC_CLOCKFACE_OPTION_STATE))
    {
        return params.at(FPSTR(PARAM_CLOCKFACE_OPTION)) == FPSTR(VALUE_ON) ? FPSTR(VALUE_CHECKED) : FPSTR(VALUE_EMPTY);
    }
    else
    {
        return String();
    }

    return String();
}

String WebUI::firmwarePageProcessor(const String &var, const std::map<String, String> &params)
{
    if (var == FPSTR(PROC_FW_VERS))
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
