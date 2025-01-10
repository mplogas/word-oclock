#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <Preferences.h>
#include <RTClib.h> // For DateTime
#include "defaults.h"

class Configuration {
public:
    enum ClockMode {
        Regular = 0,
        Option_1
    };

    struct WifiConfig {
        char ssid[32];
        char password[64];
    };

    struct MqttConfig {
        bool enabled;
        char host[64];
        uint16_t port;
        char username[32];
        char password[32];
        char topic[64];
    };

    struct NtpConfig {
        bool enabled;
        char timezone[32];
        char server[64];
        uint32_t interval;
    };

    struct LightScheduleConfig {
        bool enabled;
        time_t startTime; // Time in seconds since midnight
        time_t endTime;
    };

    struct AutoBrightnessConfig {
        bool enabled;
        uint16_t illuminanceThresholdHigh;
        uint16_t illuminanceThresholdLow;
    };

    struct LightConfig {
        uint8_t brightness;
        AutoBrightnessConfig autoBrightnessConfig;       
        char color[8]; // Hex color string (e.g., "FF0000")
        bool state;
    };

    struct SystemConfig {
        //WifiConfig wifiConfig;
        ClockMode mode;
        MqttConfig mqttConfig;
        NtpConfig ntpConfig;
        LightScheduleConfig lightScheduleConfig; 
    };    

    Configuration();
    ~Configuration();
    void init();

    void setLightState(bool state);
    void setLightBrightness(uint8_t brightness);
    void setLightColor(const char* color);
    void setAutoBrightness(const AutoBrightnessConfig& brightnessConfig);
    LightConfig getLightConfig();
    void setClockMode(ClockMode mode);
    void setMqttConfig(const MqttConfig& config);
    void setNtpConfig(const NtpConfig& config);
    void setLightSchedule(const LightScheduleConfig& schedule);
    SystemConfig getSystemConfig();
    void setWifiConfig(const WifiConfig& config);
    WifiConfig getWifiConfig();
    void reset();

private:
    Preferences systemPreferences;
    Preferences lightPreferences;
    //Preferences internalPreferences;

    // System Preferences Keys
    static constexpr const char* IS_INITIALIZED_KEY = "is_init";
    static constexpr const char* CLOCK_MODE_KEY = "clock_mode";
    static constexpr const char* WIFI_SSID_KEY = "wifi_ssid";
    static constexpr const char* WIFI_PASSWORD_KEY = "wifi_pass";
    static constexpr const char* MQTT_ENABLED_KEY = "mqtt_nbld";
    static constexpr const char* MQTT_HOST_KEY = "mqtt_host";
    static constexpr const char* MQTT_PORT_KEY = "mqtt_port";
    static constexpr const char* MQTT_USERNAME_KEY = "mqtt_user";
    static constexpr const char* MQTT_PASSWORD_KEY = "mqtt_pass";
    static constexpr const char* MQTT_TOPIC_KEY = "mqtt_tpc";
    static constexpr const char* NTP_ENABLED_KEY = "ntp_nbld";
    static constexpr const char* NTP_TIMEZONE_KEY = "ntp_tz";
    static constexpr const char* NTP_SERVER_KEY = "ntp_srv";
    static constexpr const char* NTP_UPDATE_ENABLED_KEY = "ntp_upd_nbld";
    static constexpr const char* NTP_UPDATE_INTERVAL_KEY = "ntp_upd_itvl";

    // Light Preferences Keys
    static constexpr const char* LIGHT_SCHEDULE_ENABLED_KEY = "ls_nbld";
    static constexpr const char* LIGHT_SCHEDULE_START_TIME_KEY = "ls_start_t";
    static constexpr const char* LIGHT_SCHEDULE_END_TIME_KEY = "ls_end_t";
    static constexpr const char* AUTO_BRIGHTNESS_ENABLED_KEY = "ab_nbld";
    static constexpr const char* AUTO_BRIGHTNESS_THRESH_HIGH_KEY = "ab_thrsh_hi";
    static constexpr const char* AUTO_BRIGHTNESS_THRESH_LOW_KEY = "ab_thrsh_lo";
    static constexpr const char* LIGHT_BRIGHTNESS_KEY = "lght_brightn";
    static constexpr const char* LIGHT_COLOR_KEY = "lght_color";
    static constexpr const char* LIGHT_STATE_KEY = "lght_state";

    bool getIsInitialized();
    void setIsInitialized(bool isInitialized = true);
    void initializeDefaultConfig();
    ClockMode getClockMode();
    MqttConfig getMqttConfig();
    NtpConfig getNtpConfig();
    LightScheduleConfig getLightSchedule();
    AutoBrightnessConfig getAutoBrightness();
};
#endif // CONFIGURATION_H
