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
    systemPreferences.begin("system", false); 
    lightPreferences.begin("light", false);
    if(!getIsInitialized()) {
        initializeDefaultConfig();
    }
}

void Configuration::setIsInitialized(bool isInitialized) {
    systemPreferences.putBool(IS_INITIALIZED_KEY, isInitialized);
}

bool Configuration::getIsInitialized() {
    return systemPreferences.getBool(IS_INITIALIZED_KEY, false);
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
    int size = systemPreferences.putBytes(MQTT_HOST_KEY, config.host, sizeof(config.host));
    systemPreferences.putUInt(MQTT_PORT_KEY, config.port);
    systemPreferences.putBytes(MQTT_USERNAME_KEY, config.username, sizeof(config.username));
    systemPreferences.putBytes(MQTT_PASSWORD_KEY, config.password, sizeof(config.password));
    systemPreferences.putBytes(MQTT_TOPIC_KEY, config.topic, sizeof(config.topic));
}

Configuration::MqttConfig Configuration::getMqttConfig() {
    MqttConfig config;
    config.enabled = systemPreferences.getBool(MQTT_ENABLED_KEY, Defaults::DEFAULT_MQTT_ENABLED);
    if(systemPreferences.getBytes(MQTT_HOST_KEY, config.host, sizeof(config.host)) == 0) {
        strlcpy(config.host, "", sizeof(config.host));
    }
    config.port = systemPreferences.getUInt(MQTT_PORT_KEY, Defaults::DEFAULT_MQTT_PORT);
    systemPreferences.getBytes(MQTT_USERNAME_KEY, config.username, sizeof(config.username));
    systemPreferences.getBytes(MQTT_PASSWORD_KEY, config.password, sizeof(config.password));
    systemPreferences.getBytes(MQTT_TOPIC_KEY, config.topic, sizeof(config.topic));
    return config;
}

// NTP Configuration
void Configuration::setNtpConfig(const NtpConfig& config) {
    systemPreferences.putBool(NTP_ENABLED_KEY, config.enabled);
    systemPreferences.putBytes(NTP_TIMEZONE_KEY, config.timezone, sizeof(config.timezone));
    systemPreferences.putBytes(NTP_SERVER_KEY, config.server, sizeof(config.server));
    systemPreferences.putULong(NTP_UPDATE_INTERVAL_KEY, config.interval);
}

Configuration::NtpConfig Configuration::getNtpConfig() {
    NtpConfig config;

    // Initialize buffers
    // memset(config.timezone, 0, sizeof(config.timezone));
    // memset(config.server, 0, sizeof(config.server));

    // Enabled
    config.enabled = systemPreferences.getBool(NTP_ENABLED_KEY, Defaults::DEFAULT_NTP_ENABLED);
    config.interval = systemPreferences.getULong(NTP_UPDATE_INTERVAL_KEY, Defaults::DEFAULT_NTP_UPDATE_INTERVAL); // Default interval: 24 hours

    // Timezone
    size_t tzLen = systemPreferences.getBytes(NTP_TIMEZONE_KEY, config.timezone, sizeof(config.timezone));
    if (tzLen == 0) {
        strlcpy(config.timezone, Defaults::DEFAULT_NTP_TIMEZONE, sizeof(config.timezone));
    }

    // Server
    size_t serverLen = systemPreferences.getBytes(NTP_SERVER_KEY, config.server, sizeof(config.server));
    if (serverLen == 0) {
        strlcpy(config.server, Defaults::DEFAULT_NTP_SERVER, sizeof(config.server));
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
    config.state = lightPreferences.getBool(LIGHT_STATE_KEY, true);

    memset(config.color, 0, sizeof(config.color));

    size_t colorLen = lightPreferences.getBytes(LIGHT_COLOR_KEY, config.color, sizeof(config.color));
    if (colorLen == 0) {
        strlcpy(config.color, Defaults::DEFAULT_LIGHT_COLOR, sizeof(config.color));
    } 

    config.autoBrightnessConfig = getAutoBrightness();

    return config;
}

Configuration::SystemConfig Configuration::getSystemConfig() {
    SystemConfig config;
    config.mqttConfig = getMqttConfig();
    config.ntpConfig = getNtpConfig();
    config.mode = getClockMode();
    return config;
}


// Reset all configurations
void Configuration::reset() {
    systemPreferences.clear();
    lightPreferences.clear();
}

void Configuration::initializeDefaultConfig(){
    Configuration::WifiConfig wifiConfig;
    strlcpy(wifiConfig.ssid, "", sizeof(wifiConfig.ssid));
    strlcpy(wifiConfig.ssid, "", sizeof(wifiConfig.ssid));
    strlcpy(wifiConfig.password, "", sizeof(wifiConfig.password));
    setWifiConfig(wifiConfig);

    Configuration::MqttConfig mqttConfig;
    mqttConfig.enabled = Defaults::DEFAULT_MQTT_ENABLED;
    strlcpy(mqttConfig.host, "", sizeof(mqttConfig.host));
    mqttConfig.port = Defaults::DEFAULT_MQTT_PORT;
    strlcpy(mqttConfig.username, "", sizeof(mqttConfig.username));
    strlcpy(mqttConfig.password, "", sizeof(mqttConfig.password));
    strlcpy(mqttConfig.topic, Defaults::DEFAULT_MQTT_TOPIC, sizeof(mqttConfig.topic));
    setMqttConfig(mqttConfig);

    Configuration::NtpConfig ntpConfig;
    ntpConfig.enabled = Defaults::DEFAULT_NTP_ENABLED;
    strlcpy(ntpConfig.timezone, Defaults::DEFAULT_NTP_TIMEZONE, sizeof(ntpConfig.timezone));
    strlcpy(ntpConfig.server, Defaults::DEFAULT_NTP_SERVER, sizeof(ntpConfig.server));
    ntpConfig.interval = Defaults::DEFAULT_NTP_UPDATE_INTERVAL;
    setNtpConfig(ntpConfig);

    Configuration::LightScheduleConfig lightScheduleConfig;
    lightScheduleConfig.enabled = Defaults::DEFAULT_LIGHT_SCHEDULE_ENABLED;
    lightScheduleConfig.startTime = 0;
    lightScheduleConfig.endTime = 0;
    setLightSchedule(lightScheduleConfig);

    Configuration::LightConfig lightConfig;
    lightConfig.brightness = Defaults::DEFAULT_LIGHT_BRIGHTNESS;
    lightConfig.autoBrightnessConfig.enabled = Defaults::DEFAULT_AUTO_BRIGHTNESS_ENABLED;
    lightConfig.autoBrightnessConfig.illuminanceThresholdHigh = Defaults::DEFAULT_ILLUMINANCE_THRESHOLD_HIGH;
    lightConfig.autoBrightnessConfig.illuminanceThresholdLow = Defaults::DEFAULT_ILLUMINANCE_THRESHOLD_LOW;
    strlcpy(lightConfig.color, Defaults::DEFAULT_LIGHT_COLOR, sizeof(lightConfig.color));
    lightConfig.state = Defaults::DEFAULT_LIGHT_STATE;
    setLightState(lightConfig.state);
    setLightBrightness(lightConfig.brightness);
    setLightColor(lightConfig.color);
    setAutoBrightness(lightConfig.autoBrightnessConfig);  

    setClockMode(Regular);

    setIsInitialized();
    Serial.println("Default configuration initialized");
}