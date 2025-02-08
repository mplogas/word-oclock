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
using ResponseCallback = std::function<std::map<String, String>(PageType page)>;
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
        String pageProcessor(const String &var, PageType page, const std::map<String, String> &params);

        // Helper functions
        void handleFirmwareUpdate(AsyncWebServerRequest *request);
        void handleToggleLight(AsyncWebServerRequest *request);
        void handleSetLightColor(AsyncWebServerRequest *request);
        void handleSetAutoBrightness(AsyncWebServerRequest *request);
        void handleSetBrightness(AsyncWebServerRequest *request);
        void handleSetTime(AsyncWebServerRequest *request);
        void handleSetLightSchedule(AsyncWebServerRequest *request);
        void handleSetNTPConfig(AsyncWebServerRequest *request);
        void handleSetHAIntegration(AsyncWebServerRequest *request);
        void handleSetClockFace(AsyncWebServerRequest *request);
        void printAllParams(AsyncWebServerRequest *request);
        String readFile(const char* path);

        // TODO: move to PROGMEM!

        // paths
        static const char PATH_NAVIGATION_HTML[] PROGMEM;
        static const char PATH_LIGHT_HTML[] PROGMEM;
        static const char PATH_TIME_HTML[] PROGMEM;
        static const char PATH_SYSTEM_HTML[] PROGMEM;
        static const char PATH_FIRMWARE_HTML[] PROGMEM;
        static const char PATH_WIFI_HTML[] PROGMEM;
        static constexpr const char* PATH_CSS = "/style.css";
        static constexpr const char* PATH_JS = "/index.js";
        static constexpr const char* PATH_ICON = "/favicon.ico";

        // page titles
        static const char LIGHT_PAGE_TITLE[] PROGMEM;
        static const char SYSTEM_PAGE_TITLE[] PROGMEM;
        static const char TIME_PAGE_TITLE[] PROGMEM;
        static const char FIRMWARE_PAGE_TITLE[] PROGMEM;

        // processor variables
        static const char PROC_PAGE_TITLE[] PROGMEM;
        static const char PROC_FW_VERS[] PROGMEM;

        static const char PROC_ACTIVE_LIGHT[] PROGMEM;
        static const char PROC_ACTIVE_TIME[] PROGMEM;
        static const char PROC_ACTIVE_SYSTEM[] PROGMEM;

        static const char PROC_LIGHT_STATE[] PROGMEM;
        static const char PROC_LIGHT_COLOR[] PROGMEM;
        static const char PROC_LIGHT_BRIGHTNESS[] PROGMEM;
        static const char PROC_AUTO_BRIGHTNESS_ENABLED[] PROGMEM;
        static const char PROC_TIME[] PROGMEM;
        static const char PROC_NTP_ENABLED[] PROGMEM;
        static const char PROC_NTP_HOST[] PROGMEM;
        static const char PROC_NTP_UPDATE_INTERVAL[] PROGMEM;
        static const char PROC_NTP_TIMEZONE[] PROGMEM;
        static const char PROC_BROKER_ENABLED[] PROGMEM;
        static const char PROC_BROKER_HOST[] PROGMEM;
        static const char PROC_BROKER_PORT[] PROGMEM;
        static const char PROC_BROKER_USER[] PROGMEM;
        static const char PROC_BROKER_PASS[] PROGMEM;
        static const char PROC_BROKER_DEFAULT_TOPIC[] PROGMEM;
        static const char PROC_SCHEDULE_ENABLED[] PROGMEM;
        static const char PROC_SCHEDULE_START[] PROGMEM;
        static const char PROC_SCHEDULE_END[] PROGMEM;
        static const char PROC_CLOCKFACE[] PROGMEM;
        static const char PROC_CLOCKFACE_OPTION_STATE[] PROGMEM;

        // values
        static const char VALUE_SUCCESS[] PROGMEM;
        static const char VALUE_ERROR[] PROGMEM;
        static constexpr const char* VALUE_ACTIVE = "active";
        static const char VALUE_CHECKED[] PROGMEM;
        static const char VALUE_EMPTY[] PROGMEM;
        static const char VALUE_FIRMWARE[] PROGMEM;
        static const char VALUE_FILESYS[] PROGMEM;

        static const char CONTENT_TEXT[] PROGMEM;
        static const char CONTENT_HTML[] PROGMEM;
        static constexpr const char* CONTENT_CACHE = "max-age=3600";

        static const char PARAM_FW_Type[] PROGMEM;

    public:

        static const char VALUE_ON[] PROGMEM;
        static const char VALUE_OFF[] PROGMEM;

        static const char PARAM_WIFI_SSID[] PROGMEM;
        static const char PARAM_WIFI_PASS[] PROGMEM;
        static const char PARAM_ENABLED[] PROGMEM;
        static const char PARAM_OPTION[] PROGMEM;
        static const char PARAM_VALUE[] PROGMEM;
        static const char PARAM_COLOR[] PROGMEM;
        static const char PARAM_BRIGHTNESS[] PROGMEM;
        static const char PARAM_AUTO_BRIGHTNESS_ENABLED[] PROGMEM;
        static const char PARAM_TIME[] PROGMEM;
        static const char PARAM_BROKER_ENABLED[] PROGMEM;
        static const char PARAM_BROKER_HOST[] PROGMEM;
        static const char PARAM_BROKER_PORT[] PROGMEM;
        static const char PARAM_BROKER_USER[] PROGMEM;
        static const char PARAM_BROKER_PASS[] PROGMEM;
        static const char PARAM_BROKER_DEFAULT_TOPIC[] PROGMEM;
        static const char PARAM_NTP_ENABLED[] PROGMEM;
        static const char PARAM_NTP_HOST[] PROGMEM;
        static const char PARAM_NTP_UPDATE_INTERVAL[] PROGMEM;
        static const char PARAM_NTP_TIMEZONE[] PROGMEM;
        static const char PARAM_SCHEDULE_ENABLED[] PROGMEM;
        static const char PARAM_SCHEDULE_START[] PROGMEM;
        static const char PARAM_SCHEDULE_END[] PROGMEM;
        static const char PARAM_CLOCKFACE[] PROGMEM;        
        static const char PARAM_CLOCKFACE_OPTION[] PROGMEM;
        static const char PARAM_FW_VERSION[] PROGMEM;
        
        WebUI(AsyncWebServer &server);
        ~WebUI();
        void init(const RequestCallback &requestCb, 
                  const ResponseCallback &responseCb,
                  const UpdateCallback &updateCb, 
                  const UpdateSuccessCallback &updateSuccessCb);
        void initHostAP(const RequestCallback &wrequestCb);
};

#endif