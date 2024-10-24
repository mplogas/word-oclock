#ifndef homeassistant_h
#define homeassistant_h

#include <WiFi.h>
#include <ArduinoHA.h>

typedef void (*mqttConnectCallback)(void);
typedef void (*mqttDisconnectCallback)(void);

class HomeAssistant
{
private:
    const u8_t maclen = 6;
    HAMqtt mqtt;
    HADevice device;
    HASensor sensor;
    HASwitch led;
    HADevice buildDevice();
    HASensor buildSensor();
    HASwitch buildSwitch();
public:
    HomeAssistant(WiFiClient& client);
    ~HomeAssistant();
    bool connect(IPAddress broker, const char* username, const char* password, mqttConnectCallback onConnected, mqttDisconnectCallback onDisconnected);
    bool connect(IPAddress broker, mqttConnectCallback onConnected, mqttDisconnectCallback onDisconnected);
    void loop() { mqtt.loop(); }

};

HomeAssistant::HomeAssistant(WiFiClient& client)
{
    mqtt = new HAMqtt(client, buildDevice());
    mqtt.setDataPrefix("wc2");
    sensor = buildSensor();
    led = buildSwitch();
}

HomeAssistant::~HomeAssistant()
{
    mqtt.disconnect();
}

bool HomeAssistant::connect(IPAddress broker, const char* username, const char* password, mqttConnectCallback onConnected, mqttDisconnectCallback onDisconnected)
{
    mqtt.onConnected(onConnected);
    mqtt.onDisconnected(onDisconnected);

    return mqtt.begin(broker, username, password);
}

bool HomeAssistant::connect(IPAddress broker, mqttConnectCallback onConnected, mqttDisconnectCallback onDisconnected)
{
    mqtt.onConnected(onConnected);
    mqtt.onDisconnected(onDisconnected);

    return mqtt.begin(broker);
}

HADevice HomeAssistant::buildDevice()
{
    byte mac[maclen];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    device.setName("WordClock v2");
    device.setSoftwareVersion("0.1-test");
    device.setModel("ESP32");
    device.setManufacturer("Espressif");
    return device;
}

HASensor HomeAssistant::buildSensor()
{
    sensor = new HASensor();
    sensor.setUniqueId("wc-lightintensity");
    sensor.setIcon("mdi:home");
    sensor.setName("Light Intensity");
    return sensor;
}

HASwitch HomeAssistant::buildSwitch()
{
    led = new HASwitch();
    led.setUniqueId("wc-led");
    led.setIcon("mdi:lightbulb");
    led.setName("WordClock Light");
    led.onCommand(ledSwitchCommand);
    return led;
}

#endif