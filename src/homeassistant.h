#ifndef WOC_MQTT_H
#define WOC_MQTT_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include <functional>
#include "callbacktypes.h"
#include "defaults.h"

using MqttEventCallback = std::function<void(MQTTEvent event, const char* payload)>;

class WoC_MQTT
{  
    private:
        static const char NAME_LEDS[] PROGMEM;
        static const char NAME_LIGHT_INTENSITY[] PROGMEM;
        static const char NAME_AUTO_BRIGHTNESS[] PROGMEM;
        static const char NAME_OPTION1[] PROGMEM;
        static const char NAME_OPTION2[] PROGMEM;
        static const char NAME_OPTION3[] PROGMEM;
        static const char NAME_OPTION4[] PROGMEM;

        static constexpr const char* ID_PATTERN = "%s_%s";
        static constexpr const char* ID_DEVICE = "woc";
        static constexpr const char* ID_LEDS = "leds";
        static constexpr const char* ID_ILLUMINANCE = "illuminance";
        static constexpr const char* ID_AUTO_BRIGHTNESS = "autoBrightness";
        static constexpr const char* ID_OPTION1 = "option1";
        static constexpr const char* ID_OPTION2 = "option2";
        static constexpr const char* ID_OPTION3 = "option3";
        static constexpr const char* ID_OPTION4 = "option4";

        //because of how the ArduinoHA lib is built, we need to run this class as singleton
        static WoC_MQTT* instance;
        // and we need to store the callback in a static variable
        static void onMqttConnectedStatic();
        static void onMqttDisconnectedStatic();
        static void onBrightnessCommandStatic(uint8_t brightness, HALight* sender);
        static void onRGBCommandStatic(HALight::RGBColor color, HALight* sender);
        static void onStateCommandStatic(bool state, HALight* sender);
        static void onSwitchCommandStatic(bool state, HASwitch* sender);
    
        HAMqtt *haMqtt;
        HADevice *device;
        HALight *light;
        HASensorNumber *lightSensor;
        HASwitch *autoBrightness;
        HASwitch *option1;
        HASwitch *option2;
        HASwitch *option3;
        HASwitch *option4;

        MqttEventCallback mqttEventCallback;

        bool useOptions = false;
        const uint8_t maclen = 6;
        static constexpr const char* DEFAULT_ICON = "mdi:clock-digital";
        bool isInitialized = false;
        int connectionResult = -5;
        char uniqueid[9]; // 'woc' + '_' + "XXXX" (4) + null terminator (1)
        char idLeds[14]; // uniqueid + '_leds' + null terminator (1)
        char idIlluminance[21]; // uniqueid + '_illuminance' + null terminator (1)
        char idAutoBrightness[24]; // uniqueid + '_autoBrightness' + null terminator (1)
        char idOption1[17]; // uniqueid + '_option1' + null terminator (1)
        char idOption2[17]; // uniqueid + '_option2' + null terminator (1)
        char idOption3[17]; // uniqueid + '_option3' + null terminator (1)
        char idOption4[17]; // uniqueid + '_option4' + null terminator (1)

        // helper methods
        void setupHomeAssistant();
        void disableHomeAssistant();
    public:
        WoC_MQTT(WiFiClient& client, const char* devicename, const char* firmware);
        ~WoC_MQTT();
        void connect(IPAddress host, MqttEventCallback eventCallback, const char* username = nullptr, const char* password = nullptr, const char* topic = nullptr, bool useOptions = false);
        void disconnect();
        void loop();
        void toggleLightState(bool state);
        void setLightColor(const char* color);
        void setLightBrightness(uint8_t brightness);
        void setLightSensorValue(const uint16_t sensorValue);
        void toggleAutoBrightness(bool state);
        void toggleOption1(bool state);
        void toggleOption2(bool state);
        void toggleOption3(bool state);
        void toggleOption4(bool state);
};

#endif // WOC_MQTT_H