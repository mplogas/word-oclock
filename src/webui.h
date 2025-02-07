#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <Network.h>
#include <NetworkInterface.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <functional>
#include <map>
#include "configuration.h"
#include "callbacktypes.h"

using RequestCallback = std::function<void(ControlType type, const std::map<String, String>& params)>;
using ResponseCallback = std::function<const std::map<String, String>(PageType page)>;
using UpdateCallback = std::function<void(UpdateType type, const String &filename, size_t index, uint8_t *data, size_t len, bool final)>;
using UpdateSuccessCallback = std::function<bool()>;

class WebUI
{
    private:
        AsyncWebServer &server; 
        UpdateSuccessCallback updateSuccessCallback;
        RequestCallback requestCallback;
        UpdateCallback updateCallback;
        ResponseCallback responseCallback;

        // Page Processor functions
        String lightPageProcessor(const String &var, const std::map<String, String> &params);
        String timePageProcessor(const String &var, const std::map<String, String> &params);
        String systemPageProcessor(const String &var, const std::map<String, String> &params);
        String firmwarePageProcessor(const String &var, const std::map<String, String> &params);
        String headerProcessor(PageType page);
        //String includeProcessor(const String &var);
        String pageProcessor(const String &var, PageType page, const std::map<String, String> &params);

        // Helper functions
        void handleFirmwareUpdate(AsyncWebServerRequest *request);
        void handleToggleLight(AsyncWebServerRequest *request);
        void handleSetLightColor(AsyncWebServerRequest *request);
        void handleSetAutoBrightness(AsyncWebServerRequest *request);
        void handleSetBrightness(AsyncWebServerRequest *request);
        void handleSetHAIntegration(AsyncWebServerRequest *request);
        void handleSetClockFace(AsyncWebServerRequest *request);
        void printAllParams(AsyncWebServerRequest *request);
        String readFile(const char* path);

        // paths
        static constexpr const char* PATH_NAVIGATION_HTML = "/navigation.html";
        static constexpr const char* PATH_LIGHT_HTML = "/light.html";
        static constexpr const char* PATH_TIME_HTML = "/time.html";
        static constexpr const char* PATH_SYSTEM_HTML = "/system.html";
        static constexpr const char* PATH_FIRMWARE_HTML = "/firmware.html";
        static constexpr const char* PATH_WIFI_HTML = "/wifimanager.html";
        static constexpr const char* PATH_CSS = "/style.css";
        static constexpr const char* PATH_JS = "/index.js";
        static constexpr const char* PATH_ICON = "/favicon.ico";

        // page titles
        static constexpr const char* LIGHT_PAGE_TITLE = "Light Settings";
        static constexpr const char* SYSTEM_PAGE_TITLE = "System Configuration";
        static constexpr const char* TIME_PAGE_TITLE = "Time Configuration";
        static constexpr const char* FIRMWARE_PAGE_TITLE = "Firmware Update";

        // processor variables
        static constexpr const char* PROC_PAGE_TITLE = "PAGE_TITLE";
        static constexpr const char* PROC_FW_VERS = "FW_VERSION";

        // values
        static constexpr const char* VALUE_SUCCESS = "Success";
        static constexpr const char* VALUE_ERROR = "Error!";
        static constexpr const char* VALUE_ACTIVE = "active";
        static constexpr const char* VALUE_FIRMWARE = "firmware";
        static constexpr const char* VALUE_FILESYS = "filesystem";

        static constexpr const char* CONTENT_TEXT = "text/plain";
        static constexpr const char* CONTENT_HTML = "text/html";
        static constexpr const char* CONTENT_CACHE = "max-age=3600";

        static constexpr const char* PARAM_FW_Type = "updateType";

    public:

        static constexpr const char* VALUE_ON = "1";
        static constexpr const char* VALUE_OFF = "0";

        static constexpr const char* PARAM_WIFI_SSID = "ssid";
        static constexpr const char* PARAM_WIFI_PASS = "wifi-pass";
        static constexpr const char* PARAM_ENABLED = "enabled";
        static constexpr const char* PARAM_OPTION = "option";
        static constexpr const char* PARAM_VALUE = "value";
        static constexpr const char* PARAM_COLOR = "color";
        static constexpr const char* PARAM_BRIGHTNESS = "brightness";
        static constexpr const char* PARAM_AUTO_BRIGHTNESS_ENABLED = "abEnabled";
        static constexpr const char* PARAM_TIME = "time";
        static constexpr const char* PARAM_BROKER_ENABLED = "mqttEnabled";
        static constexpr const char* PARAM_BROKER_HOST = "mqttHost";
        static constexpr const char* PARAM_BROKER_PORT = "mqttPort";
        static constexpr const char* PARAM_BROKER_USER = "mqttUsername";
        static constexpr const char* PARAM_BROKER_PASS = "mqttPassword";
        static constexpr const char* PARAM_BROKER_DEFAULT_TOPIC = "mqttTopic";
        static constexpr const char* PARAM_NTP_ENABLED = "ntpEnabled";
        static constexpr const char* PARAM_NTP_HOST = "ntpHost";
        static constexpr const char* PARAM_NTP_UPDATE_INTERVAL = "ntpInterval";
        static constexpr const char* PARAM_NTP_TIMEZONE = "ntpTimezone";
        static constexpr const char* PARAM_SCHEDULE_ENABLED = "scheduleEnabled";
        static constexpr const char* PARAM_SCHEDULE_START = "scheduleStart";
        static constexpr const char* PARAM_SCHEDULE_END = "scheduleEnd";
        static constexpr const char* PARAM_CLOCKFACE = "clockFace";        
        static constexpr const char* PARAM_CLOCKFACE_OPTION = "clockFaceOption";
        static constexpr const char* PARAM_FW_VERSION = "fwVersion";
        
        WebUI(AsyncWebServer &server);
        ~WebUI();
        void init(const RequestCallback &requestCb, 
        const ResponseCallback &responseCb,
        const UpdateCallback &updateCb, 
        const UpdateSuccessCallback &updateSuccessCb);
        void initHostAP(const RequestCallback &wrequestCb);
};

#endif