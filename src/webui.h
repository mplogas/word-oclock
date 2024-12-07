#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <Network.h>
#include <NetworkInterface.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <functional>
#include "defaults.h"

using UpdateSuccessCallback = std::function<bool()>;
using UploadHandlerCallback = std::function<void(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)>;
using WiFiSetupCallback = std::function<void(const String &ssid, const String &password)>;
using LightControlCallback = std::function<void(bool)>;
using SystemControlCallback = std::function<void(bool)>;

// Forward declaration for shared_ptr usage
class WebUI
{
    private:
        enum class Page {
            LIGHT,
            SYSTEM,
            FIRMWARE
        };
        AsyncWebServer &server; 
        UpdateSuccessCallback updateCallback;
        UploadHandlerCallback uploadHandlerCallback;
        WiFiSetupCallback wifiCredentialsCallback;
        const char* deviceName;
        const char* firmwareVersion;

        // Processor functions
        String pageProcessor(const String &var, Page page);

        // Helper functions
        void handleFirmwareUpdate(AsyncWebServerRequest *request);

        // paths
        static constexpr const char* HEADER_HTML = "/header.html";
        static constexpr const char* LIGHT_HTML = "/light.html";
        static constexpr const char* SYSTEM_HTML = "/system.html";
        static constexpr const char* FIRMWARE_HTML = "/firmware.html";
        static constexpr const char* WIFI_MANAGER_HTML = "/wifimanager.html";

        // processor variables
        static constexpr const char* PAGE_TITLE = "PAGE_TITLE";
        static constexpr const char* FIRMWARE = "FW_VERSION";

        // Input fields
        static constexpr const char* SSID_INPUT = "ssid";
        static constexpr const char* WIFI_PASS_INPUT = "wifi-pass";
        static constexpr const char* BROKER_INPUT = "broker";
        static constexpr const char* BROKER_USER_INPUT = "broker-user";
        static constexpr const char* BROKER_PASS_INPUT = "broker-pass";
        static constexpr const char* MQTT_TOPIC_INPUT = "mqtt-topic";

    public:
        WebUI(AsyncWebServer &server);
        ~WebUI();

        void init(const UpdateSuccessCallback &updateCb, const UploadHandlerCallback &uploadCb);
        void initHostAP(const WiFiSetupCallback &wifiCb);
};

#endif