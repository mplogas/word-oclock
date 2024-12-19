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
    };

    struct LightScheduleConfig {
        bool enabled;
        time_t startTime; // Time in seconds since midnight
        time_t endTime;
    };

    struct NtpUpdateConfig {
        bool enabled;
        uint32_t interval;
    };

    struct AutoBrightnessConfig {
        bool enabled;
        uint16_t illuminanceThresholdHigh;
        uint16_t illuminanceThresholdLow;
    };

    struct LightConfig {
        uint8_t brightness;
        AutoBrightnessConfig autoBrightnessConfig;       
        char color[7]; // Hex color string (e.g., "FF0000")
        bool state;
    };

    struct SystemConfig {
        //WifiConfig wifiConfig;
        ClockMode mode;
        MqttConfig mqttConfig;
        NtpConfig ntpConfig;
        NtpUpdateConfig ntpUpdateConfig;
        LightScheduleConfig lightScheduleConfig; 
    };    

    Configuration();
    ~Configuration();
    void init();

    void setLightConfig(const LightConfig& config);
    LightConfig getLightConfig();
    void setSystemConfig(const SystemConfig& config);
    SystemConfig getSystemConfig();
    void setWifiConfig(const WifiConfig& config);
    WifiConfig getWifiConfig();
    void reset();

private:
    Preferences systemPreferences;
    Preferences lightPreferences;

    // System Preferences Keys
    static constexpr const char* CLOCK_MODE_KEY = "clock_mode";
    static constexpr const char* WIFI_SSID_KEY = "wifi_ssid";
    static constexpr const char* WIFI_PASSWORD_KEY = "wifi_password";
    static constexpr const char* MQTT_ENABLED_KEY = "mqtt_enabled";
    static constexpr const char* MQTT_HOST_KEY = "mqtt_host";
    static constexpr const char* MQTT_PORT_KEY = "mqtt_port";
    static constexpr const char* MQTT_USERNAME_KEY = "mqtt_username";
    static constexpr const char* MQTT_PASSWORD_KEY = "mqtt_password";
    static constexpr const char* MQTT_TOPIC_KEY = "mqtt_topic";
    static constexpr const char* NTP_ENABLED_KEY = "ntp_enabled";
    static constexpr const char* NTP_TIMEZONE_KEY = "ntp_timezone";
    static constexpr const char* NTP_SERVER_KEY = "ntp_server";
    static constexpr const char* NTP_UPDATE_ENABLED_KEY = "ntp_upd_enabled";
    static constexpr const char* NTP_UPDATE_INTERVAL_KEY = "ntp_upd_interval";

    // Light Preferences Keys
    static constexpr const char* LIGHT_SCHEDULE_ENABLED_KEY = "ls_enabled";
    static constexpr const char* LIGHT_SCHEDULE_START_TIME_KEY = "ls_start_time";
    static constexpr const char* LIGHT_SCHEDULE_END_TIME_KEY = "ls_end_time";
    static constexpr const char* AUTO_BRIGHTNESS_ENABLED_KEY = "ab_enabled";
    static constexpr const char* AUTO_BRIGHTNESS_THRESH_HIGH_KEY = "ab_thresh_hi";
    static constexpr const char* AUTO_BRIGHTNESS_THRESH_LOW_KEY = "ab_thresh_lo";
    static constexpr const char* LIGHT_BRIGHTNESS_KEY = "light_brightness";
    static constexpr const char* LIGHT_COLOR_KEY = "light_color";
    static constexpr const char* LIGHT_STATE_KEY = "light_state";

    // Default MQTT Configuration


    // Default Light Configuration

    void setClockMode(ClockMode mode);
    ClockMode getClockMode();
    void setMqttConfig(const MqttConfig& config);
    MqttConfig getMqttConfig();
    void setNtpConfig(const NtpConfig& config);
    NtpConfig getNtpConfig();
    void setLightSchedule(const LightScheduleConfig& schedule);
    LightScheduleConfig getLightSchedule();
    void setNtpUpdate(const NtpUpdateConfig& update);
    NtpUpdateConfig getNtpUpdate();
    void setAutoBrightness(const AutoBrightnessConfig& brightnessConfig);
    AutoBrightnessConfig getAutoBrightness();
};

#endif // CONFIGURATION_H
