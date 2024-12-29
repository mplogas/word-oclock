#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <Arduino.h>

class Defaults
{
public:
    static const uint32_t WIFI_SCAN_TIMEOUT = 10000;
    static const uint16_t LED_INTERVAL = 5000;
    static constexpr const char* PRODUCT = "word-o-clock";
    static constexpr const char* FW_VERSION = "0.5.4-test";
    static constexpr const char* DEFAULT_WIFI_SETUP_PASS = "w0Rd0Cl0cK";
    static constexpr bool DEFAULT_MQTT_ENABLED = false;
    static constexpr uint16_t DEFAULT_MQTT_PORT = 1883;
    static constexpr const char* DEFAULT_MQTT_TOPIC = "woc";
    static constexpr bool DEFAULT_NTP_ENABLED = true;
    static constexpr const char* DEFAULT_NTP_TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3"; // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for UI
    static constexpr const char* DEFAULT_NTP_SERVER = "0.pool.ntp.org";
    static constexpr bool DEFAULT_LIGHT_SCHEDULE_ENABLED = false;
    static constexpr bool DEFAULT_NTP_UPDATE_ENABLED = true;
    static constexpr uint32_t DEFAULT_NTP_UPDATE_INTERVAL = 86400; // 24 hours
    static constexpr bool DEFAULT_AUTO_BRIGHTNESS_ENABLED = true;
    static constexpr uint16_t DEFAULT_ILLUMINANCE_THRESHOLD_HIGH = 4095;
    static constexpr uint16_t DEFAULT_ILLUMINANCE_THRESHOLD_LOW = 300;
    static constexpr uint8_t DEFAULT_LIGHT_BRIGHTNESS = 50;
    static constexpr const char* DEFAULT_LIGHT_COLOR = "FFFFFF";

    Defaults(/* args */);
    ~Defaults();
};

#endif