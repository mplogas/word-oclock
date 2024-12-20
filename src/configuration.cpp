// configuration.cpp

#include "configuration.h"

// Constructor
Configuration::Configuration() {
    // Initialization is done in the init() method
}

// Destructor
Configuration::~Configuration() {
    systemPreferences.end();
    lightPreferences.end();
}

// Initialize Preferences namespaces
void Configuration::init() {
    systemPreferences.begin("system", false); // Read and write access
    lightPreferences.begin("light", false);
}

// ClockMode
void Configuration::setClockMode(ClockMode mode) {
    systemPreferences.putUInt(CLOCK_MODE_KEY, static_cast<uint32_t>(mode));
}

Configuration::ClockMode Configuration::getClockMode() {
    uint32_t mode = systemPreferences.getUInt(CLOCK_MODE_KEY, static_cast<uint32_t>(Regular));
    return static_cast<ClockMode>(mode);
}

// WiFi Configuration
void Configuration::setWifiConfig(const WifiConfig& config) {
    systemPreferences.putBytes(WIFI_SSID_KEY, config.ssid, sizeof(config.ssid));
    systemPreferences.putBytes(WIFI_PASSWORD_KEY, config.password, sizeof(config.password));
}

Configuration::WifiConfig Configuration::getWifiConfig() {
    WifiConfig config;
    systemPreferences.getBytes(WIFI_SSID_KEY, config.ssid, sizeof(config.ssid));
    systemPreferences.getBytes(WIFI_PASSWORD_KEY, config.password, sizeof(config.password));
    return config;
}

// MQTT Configuration
void Configuration::setMqttConfig(const MqttConfig& config) {
    systemPreferences.putBool(MQTT_ENABLED_KEY, config.enabled);
    systemPreferences.putBytes(MQTT_HOST_KEY, config.host, sizeof(config.host));
    systemPreferences.putUInt(MQTT_PORT_KEY, config.port);
    systemPreferences.putBytes(MQTT_USERNAME_KEY, config.username, sizeof(config.username));
    systemPreferences.putBytes(MQTT_PASSWORD_KEY, config.password, sizeof(config.password));
    systemPreferences.putBytes(MQTT_TOPIC_KEY, config.topic, sizeof(config.topic));
}

Configuration::MqttConfig Configuration::getMqttConfig() {
    MqttConfig config;

    // Initialize buffers to null characters
    memset(config.topic, 0, sizeof(config.topic));

    config.enabled = systemPreferences.getBool(MQTT_ENABLED_KEY, Defaults::DEFAULT_MQTT_ENABLED);
    config.port = systemPreferences.getUInt(MQTT_PORT_KEY, Defaults::DEFAULT_MQTT_PORT);

    size_t topicLen = systemPreferences.getBytes(MQTT_TOPIC_KEY, config.topic, sizeof(config.topic) - 1);
    if (topicLen == 0) {
        // Property not set, assign default
        strncpy(config.topic, Defaults::DEFAULT_MQTT_TOPIC, sizeof(config.topic) - 1);
    }

    return config;
}

// NTP Configuration
void Configuration::setNtpConfig(const NtpConfig& config) {
    systemPreferences.putBool(NTP_ENABLED_KEY, config.enabled);
    systemPreferences.putBytes(NTP_TIMEZONE_KEY, config.timezone, sizeof(config.timezone));
    systemPreferences.putBytes(NTP_SERVER_KEY, config.server, sizeof(config.server));
}

Configuration::NtpConfig Configuration::getNtpConfig() {
    NtpConfig config;

    // Initialize buffers
    memset(config.timezone, 0, sizeof(config.timezone));
    memset(config.server, 0, sizeof(config.server));

    // Enabled
    config.enabled = systemPreferences.getBool(NTP_ENABLED_KEY, Defaults::DEFAULT_NTP_ENABLED);

    // Timezone
    size_t tzLen = systemPreferences.getBytes(NTP_TIMEZONE_KEY, config.timezone, sizeof(config.timezone) - 1);
    if (tzLen == 0) {
        strncpy(config.timezone, Defaults::DEFAULT_NTP_TIMEZONE, sizeof(config.timezone) - 1);
    }

    // Server
    size_t serverLen = systemPreferences.getBytes(NTP_SERVER_KEY, config.server, sizeof(config.server) - 1);
    if (serverLen == 0) {
        strncpy(config.server, Defaults::DEFAULT_NTP_SERVER, sizeof(config.server) - 1);
    }

    return config;
}

// Light Schedule
void Configuration::setLightSchedule(const LightScheduleConfig& schedule) {
    lightPreferences.putBool(LIGHT_SCHEDULE_ENABLED_KEY, schedule.enabled);
    lightPreferences.putUInt(LIGHT_SCHEDULE_START_TIME_KEY, schedule.startTime);
    lightPreferences.putUInt(LIGHT_SCHEDULE_END_TIME_KEY, schedule.endTime);
}

Configuration::LightScheduleConfig Configuration::getLightSchedule() {
    LightScheduleConfig schedule;
    schedule.enabled = lightPreferences.getBool(LIGHT_SCHEDULE_ENABLED_KEY, Defaults::DEFAULT_LIGHT_SCHEDULE_ENABLED);
    schedule.startTime = lightPreferences.getUInt(LIGHT_SCHEDULE_START_TIME_KEY, 0);
    schedule.endTime = lightPreferences.getUInt(LIGHT_SCHEDULE_END_TIME_KEY, 0);
    return schedule;
}

// NTP Update
void Configuration::setNtpUpdate(const NtpUpdateConfig& update) {
    systemPreferences.putBool(NTP_UPDATE_ENABLED_KEY, update.enabled);
    systemPreferences.putULong(NTP_UPDATE_INTERVAL_KEY, update.interval);
}

Configuration::NtpUpdateConfig Configuration::getNtpUpdate() {
    NtpUpdateConfig update;
    update.enabled = systemPreferences.getBool(NTP_UPDATE_ENABLED_KEY, Defaults::DEFAULT_NTP_UPDATE_ENABLED);
    update.interval = systemPreferences.getULong(NTP_UPDATE_INTERVAL_KEY, Defaults::DEFAULT_NTP_UPDATE_INTERVAL); // Default interval: 24 hours
    return update;
}

// Auto Brightness
void Configuration::setAutoBrightness(const AutoBrightnessConfig& brightnessConfig) {
    lightPreferences.putBool(AUTO_BRIGHTNESS_ENABLED_KEY, brightnessConfig.enabled);
    lightPreferences.putUShort(AUTO_BRIGHTNESS_THRESH_HIGH_KEY, brightnessConfig.illuminanceThresholdHigh);
    lightPreferences.putUShort(AUTO_BRIGHTNESS_THRESH_LOW_KEY, brightnessConfig.illuminanceThresholdLow);
}

Configuration::AutoBrightnessConfig Configuration::getAutoBrightness() {
    AutoBrightnessConfig config;
    
    config.enabled = lightPreferences.getBool(AUTO_BRIGHTNESS_ENABLED_KEY, Defaults::DEFAULT_AUTO_BRIGHTNESS_ENABLED);
    
    config.illuminanceThresholdHigh = lightPreferences.getUShort(
        AUTO_BRIGHTNESS_THRESH_HIGH_KEY, Defaults::DEFAULT_ILLUMINANCE_THRESHOLD_HIGH);

    config.illuminanceThresholdLow = lightPreferences.getUShort(
        AUTO_BRIGHTNESS_THRESH_LOW_KEY, Defaults::DEFAULT_ILLUMINANCE_THRESHOLD_LOW);

    return config;
}

// Light Configuration
// void Configuration::setLightConfig(const LightConfig& config) {
//     lightPreferences.putUChar(LIGHT_BRIGHTNESS_KEY, config.brightness);
//     lightPreferences.putBytes(LIGHT_COLOR_KEY, config.color, sizeof(config.color));
//     lightPreferences.putBool(LIGHT_STATE_KEY, config.state);
//     setAutoBrightness(config.autoBrightnessConfig);
// }

void Configuration::setLightState(bool state) {
    lightPreferences.putBool(LIGHT_STATE_KEY, state);
}

void Configuration::setLightBrightness(uint8_t brightness) {
    lightPreferences.putUChar(LIGHT_BRIGHTNESS_KEY, brightness);
}

void Configuration::setLightColor(const char* color) {
    lightPreferences.putBytes(LIGHT_COLOR_KEY, color, strlen(color));
}


Configuration::LightConfig Configuration::getLightConfig() {
    LightConfig config;

    config.brightness = lightPreferences.getUChar(LIGHT_BRIGHTNESS_KEY, Defaults::DEFAULT_LIGHT_BRIGHTNESS);
    config.state = lightPreferences.getBool(LIGHT_STATE_KEY, false);

    memset(config.color, 0, sizeof(config.color));

    size_t colorLen = lightPreferences.getBytes(LIGHT_COLOR_KEY, config.color, sizeof(config.color) - 1);
    if (colorLen == 0) {
        strncpy(config.color, Defaults::DEFAULT_LIGHT_COLOR, sizeof(config.color) - 1);
    } 

    config.autoBrightnessConfig = getAutoBrightness();

    return config;
}

Configuration::SystemConfig Configuration::getSystemConfig() {
    SystemConfig config;
    config.mqttConfig = getMqttConfig();
    config.ntpConfig = getNtpConfig();
    config.ntpUpdateConfig = getNtpUpdate();
    config.mode = getClockMode();
    return config;
}

// Reset all configurations
void Configuration::reset() {
    systemPreferences.clear();
    lightPreferences.clear();
}