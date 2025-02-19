#include "homeassistant.h"

WoC_MQTT *WoC_MQTT::instance = nullptr;

const char WoC_MQTT::NAME_LEDS[] PROGMEM = "LEDs";
const char WoC_MQTT::NAME_LIGHT_INTENSITY[] PROGMEM = "Light Intensity";
const char WoC_MQTT::NAME_AUTO_BRIGHTNESS[] PROGMEM = "Auto Brightness";
const char WoC_MQTT::NAME_OPTION1[] PROGMEM = "Option 1";
const char WoC_MQTT::NAME_OPTION2[] PROGMEM = "Option 2";
const char WoC_MQTT::NAME_OPTION3[] PROGMEM = "Option 3";
const char WoC_MQTT::NAME_OPTION4[] PROGMEM = "Option 4";

// // Static callback forwarders
void WoC_MQTT::onMqttConnectedStatic()
{
    if (instance && instance->mqttEventCallback)
    {
        instance->mqttEventCallback(MQTTEvent::Connected, nullptr);
    }
}

void WoC_MQTT::onMqttDisconnectedStatic()
{
    if (instance && instance->mqttEventCallback)
    {
        instance->mqttEventCallback(MQTTEvent::Disconnected, nullptr);
    }
}

void WoC_MQTT::onBrightnessCommandStatic(uint8_t brightness, HALight *sender)
{
    if (instance && instance->mqttEventCallback)
    {
        char payload[4];
        snprintf(payload, sizeof(payload), "%d", brightness);
        instance->mqttEventCallback(MQTTEvent::BrightnessCommand, payload);
        sender->setBrightness(brightness);
    }
}

void WoC_MQTT::onRGBCommandStatic(HALight::RGBColor color, HALight *sender)
{
    if (instance && instance->mqttEventCallback)
    {
        // convert RGB to HEX
        char payload[8];
        snprintf(payload, sizeof(payload), "#%02X%02X%02X", color.red, color.green, color.blue);
        instance->mqttEventCallback(MQTTEvent::RGBCommand, payload);
        sender->setRGBColor(color);
    }
}

void WoC_MQTT::onStateCommandStatic(bool state, HALight *sender)
{
    if (instance && instance->mqttEventCallback)
    {
        const char *payload = state ? "1" : "0";
        instance->mqttEventCallback(MQTTEvent::StateCommand, payload);
        sender->setState(state);
    }
}

void WoC_MQTT::onSwitchCommandStatic(bool state, HASwitch *sender)
{
    if (instance && instance->mqttEventCallback)
    {
        const char *payload = state ? "1" : "0";
        if (sender == instance->autoBrightness)
        {
            instance->mqttEventCallback(MQTTEvent::AutoBrightnessSwitchCommand, payload);
        }
        else if (sender == instance->option1)
        {
            instance->mqttEventCallback(MQTTEvent::Option1SwitchCommand, payload);
        }
        else if (sender == instance->option2)
        {
            instance->mqttEventCallback(MQTTEvent::Option2SwitchCommand, payload);
        }
        else if (sender == instance->option3)
        {
            instance->mqttEventCallback(MQTTEvent::Option3SwitchCommand, payload);
        }
        else if (sender == instance->option4)
        {
            instance->mqttEventCallback(MQTTEvent::Option4SwitchCommand, payload);
        }
        else
        {
            Serial.printf("Unknown switch sender %s\n", sender->getName());
        }

        sender->setState(state);
    }
}

WoC_MQTT::WoC_MQTT(WiFiClient &client, const char *devicename, const char *firmware) //: device(), mqtt(client, device)
{
    // Set the static instance pointer
    if (instance == nullptr)
    {
        instance = this;
    }
    else
    {
        Serial.println("Warning: Multiple instances of WoC MQTT are not supported.");
        return;
    }

    // Build the HADevice first
    byte mac[maclen];
    WiFi.macAddress(mac);
    // Extract the last two bytes of the MAC address
    uint8_t lastTwoBytes[2] = {mac[4], mac[5]};
    // Format the last two bytes as a hexadecimal string (4 characters)
    char macStr[5]; // 4 hex digits + null terminator
    snprintf(macStr, sizeof(macStr), "%02X%02X", lastTwoBytes[0], lastTwoBytes[1]);

    snprintf(uniqueid, sizeof(uniqueid), ID_PATTERN, ID_DEVICE, macStr);

    device = new HADevice(uniqueid);
    device->setName(devicename);
    device->setSoftwareVersion(firmware);
    device->setModel(Defaults::FW_VERSION);
    device->setManufacturer(Defaults::MANUFACTURER);

    haMqtt = new HAMqtt(client, *device);
}

WoC_MQTT::~WoC_MQTT()
{
    disconnect();
    delete haMqtt;
    if (instance == this)
    {
        instance = nullptr;
    }
}

void WoC_MQTT::connect(IPAddress host, MqttEventCallback eventCallback, const char *username, const char *password, const char *topic, bool useOptions)
{
    if (isInitialized)
    {
        return;
    }
    mqttEventCallback = eventCallback;
    instance->useOptions = useOptions;

    if (topic)
    {
        haMqtt->setDataPrefix(topic);
    }
    else
    {
        haMqtt->setDataPrefix(Defaults::DEFAULT_MQTT_TOPIC);
    }

    // callbacks and components setup
    haMqtt->onConnected(onMqttConnectedStatic);
    haMqtt->onDisconnected(onMqttConnectedStatic);
    setupHomeAssistant();

    // Connect with or without credentials
    bool result = false;
    if (username && password && *username != '\0' && *password != '\0')
    {
        result = haMqtt->begin(host, username, password);
    }
    else
    {
        result = haMqtt->begin(host);
    }
    Serial.printf("MQTT connection result: %s\n", result ? "Success" : "Failed");

    isInitialized = true;
}

void WoC_MQTT::disconnect()
{
    if (!isInitialized)
    {
        return;
    }

    if (haMqtt->isConnected())
        haMqtt->disconnect();
    disableHomeAssistant();
    haMqtt->onConnected(nullptr);
    haMqtt->onDisconnected(nullptr);
    mqttEventCallback = nullptr;
}

void WoC_MQTT::loop()
{
    if (haMqtt)
    {
        /*
            enum ConnectionState {
                StateConnecting = -5,
                StateConnectionTimeout = -4,
                StateConnectionLost = -3,
                StateConnectionFailed = -2,
                StateDisconnected = -1,
                StateConnected = 0,
                StateBadProtocol = 1,
                StateBadClientId = 2,
                StateUnavailable = 3,
                StateBadCredentials = 4,
                StateUnauthorized = 5
            };
        */
        // int connectionState = haMqtt->getState();
        // if(connectionState != connectionResult) {
        //     Serial.printf("Connection result changed from %d to %d\n", connectionResult, connectionState);
        //     connectionResult = connectionState;
        // }

        haMqtt->loop();
    }
}

void WoC_MQTT::setupHomeAssistant()
{

    snprintf(idLeds, sizeof(idLeds), ID_PATTERN, uniqueid, ID_LEDS);
    snprintf(idIlluminance, sizeof(idIlluminance), ID_PATTERN, uniqueid, ID_ILLUMINANCE);
    snprintf(idAutoBrightness, sizeof(idAutoBrightness), ID_PATTERN, uniqueid, ID_AUTO_BRIGHTNESS);
    snprintf(idOption1, sizeof(idOption1), ID_PATTERN, uniqueid, ID_OPTION1);
    snprintf(idOption2, sizeof(idOption2), ID_PATTERN, uniqueid, ID_OPTION2);
    snprintf(idOption3, sizeof(idOption3), ID_PATTERN, uniqueid, ID_OPTION3);
    snprintf(idOption4, sizeof(idOption4), ID_PATTERN, uniqueid, ID_OPTION4);

    light = new HALight(idLeds, HALight::BrightnessFeature | HALight::RGBFeature);
    light->onBrightnessCommand(onBrightnessCommandStatic);
    light->onRGBColorCommand(onRGBCommandStatic);
    light->onStateCommand(onStateCommandStatic);
    light->setName(NAME_LEDS);

    lightSensor = new HASensorNumber(idIlluminance, HABaseDeviceType::NumberPrecision::PrecisionP0, HASensorNumber::Features::DefaultFeatures);
    lightSensor->setName(NAME_LIGHT_INTENSITY);
    lightSensor->setUnitOfMeasurement("lx");

    autoBrightness = new HASwitch(idAutoBrightness);
    autoBrightness->onCommand(onSwitchCommandStatic);
    autoBrightness->setName(NAME_AUTO_BRIGHTNESS);

    if (useOptions)
    {
        option1 = new HASwitch(idOption1);
        option1->onCommand(onSwitchCommandStatic);
        option1->setName(NAME_OPTION1);

        option2 = new HASwitch(idOption2);
        option2->onCommand(onSwitchCommandStatic);
        option2->setName(NAME_OPTION2);

        option3 = new HASwitch(idOption3);
        option3->onCommand(onSwitchCommandStatic);
        option3->setName(NAME_OPTION3);

        option4 = new HASwitch(idOption4);
        option4->onCommand(onSwitchCommandStatic);
        option4->setName(NAME_OPTION4);
    }
}

void WoC_MQTT::disableHomeAssistant()
{
    delete light;
    delete lightSensor;
    delete autoBrightness;
    if (useOptions)
    {
        delete option1;
        delete option2;
        delete option3;
        delete option4;
    }
}

void WoC_MQTT::toggleLightState(bool state)
{
    if (!isInitialized || !light)
    {
        return;
    }
    light->setState(state);
}

void WoC_MQTT::setLightBrightness(uint8_t brightness)
{
    if (!isInitialized || !light)
    {
        return;
    }
    light->setBrightness(brightness);
}

void WoC_MQTT::setLightColor(const char *hex)
{
    if (!isInitialized || !light)
    {
        return;
    }

    long number;
    // Skip '#' if present
    if (hex[0] == '#')
    {
        number = strtol(hex + 1, nullptr, 16);
    }
    else
    {
        number = strtol(hex, nullptr, 16);
    }

    // Example Input: "#ff0000" (or "ff0000")
    uint8_t red = (number >> 16);         // 0xff0000 >> 16 = 0xff = 255
    uint8_t green = (number >> 8) & 0xFF; // 0xff0000 >> 8 = 0xff00, & 0xFF = 0x00 = 0
    uint8_t blue = number & 0xFF;         // 0xff0000 & 0xFF = 0x00 = 0

    light->setRGBColor(HALight::RGBColor(red, green, blue));
}

void WoC_MQTT::setLightSensorValue(const uint16_t sensorValue)
{
    if (!isInitialized || !lightSensor)
    {
        return;
    }
    lightSensor->setValue(sensorValue);
}

void WoC_MQTT::toggleAutoBrightness(bool state)
{
    if (!isInitialized || !autoBrightness)
    {
        return;
    }
    autoBrightness->setState(state);
}

void WoC_MQTT::toggleOption1(bool state)
{
    if (!isInitialized || !option1)
    {
        return;
    }
    if (useOptions)
    {
        option1->setState(state);
    }
}

void WoC_MQTT::toggleOption2(bool state)
{
    if (!isInitialized || !option2)
    {
        return;
    }

    if (useOptions)
    {
        option2->setState(state);
    }
}

void WoC_MQTT::toggleOption3(bool state)
{
    if (!isInitialized || !option3)
    {
        return;
    }

    if (useOptions)
    {
        option3->setState(state);
    }
}

void WoC_MQTT::toggleOption4(bool state)
{
    if (!isInitialized || !option4)
    {
        return;
    }

    if (useOptions)
    {
        option4->setState(state);
    }
}
