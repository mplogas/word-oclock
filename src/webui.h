#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <Network.h>
#include <NetworkInterface.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <functional>

using UpdateSuccessCallback = std::function<bool()>;
using UploadHandlerCallback = std::function<void(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)>;
using WiFiCredentialsCallback = std::function<void(const String &ssid, const String &password)>;

// Forward declaration for shared_ptr usage
class WebUI
{
    private:
        AsyncWebServer &server; 
        UpdateSuccessCallback updateCallback;
        UploadHandlerCallback uploadHandlerCallback;
        WiFiCredentialsCallback wifiCredentialsCallback;
        const char* deviceName;
        const char* firmwareVersion;

        // Processor functions
        String wifiSetupProcessor(const String &var);
        String fwUpdateProcessor(const String &var);
        String configurationProcessor(const String &var);

        // Helper functions
        void handleFirmwareUpdate(AsyncWebServerRequest *request);

        // Constant strings
        static constexpr const char* INDEX_HTML = "/index.html";
        static constexpr const char* FIRMWARE_HTML = "/firmware.html";
        static constexpr const char* WIFI_MANAGER_HTML = "/wifimanager.html";
        static constexpr const char* PAGE_TITLE = "PAGE_TITLE";
        static constexpr const char* FIRMWARE = "FW_VERSION";
        static constexpr const char* SSID_INPUT = "ssid";
        static constexpr const char* WIFI_PASS_INPUT = "wifi-pass";
        static constexpr const char* BROKER_INPUT = "broker";
        static constexpr const char* BROKER_USER_INPUT = "broker-user";
        static constexpr const char* BROKER_PASS_INPUT = "broker-pass";
        static constexpr const char* MQTT_TOPIC_INPUT = "mqtt-topic";

    public:
        WebUI(AsyncWebServer &server, const char* devicename, const char* firmware);
        ~WebUI();

        void init(const UpdateSuccessCallback &updateCb, const UploadHandlerCallback &uploadCb);
        void initHostAP(const WiFiCredentialsCallback &wifiCb);
};

#endif