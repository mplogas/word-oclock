#ifndef HOMEASSISTANT_H
#define HOMEASSISTANT_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include <functional>
#include <memory>

// HA entities enums
enum class SensorType {
    LightIntensity=0,
    LightColor,
    TimeUpdateInterval,
    SensorTypeCount // Keep this as the last element
};

enum class SwitchType {
    LED=0,
    AutoTimeUpdate,
    SwitchTypeCount // Keep this as the last element
};

// Define callback types
using MqttConnectCallback = std::function<void()>;
using MqttDisconnectCallback = std::function<void()>;
using SwitchCommandCallback = std::function<void(SwitchType switchType, bool state)>;

class HomeAssistant
{
private:
    const uint8_t maclen = 6;
    static constexpr const char* DEFAULT_ICON = "mdi:clock-digital";

    // Using smart pointers for dynamic initialization
    std::unique_ptr<HAMqtt> mqtt;
    std::unique_ptr<HASensor> sensor;
    std::unique_ptr<HASwitch> led;
    HADevice device;

    // Callbacks for MQTT
    MqttConnectCallback mqttConnectCallback;
    MqttDisconnectCallback mqttDisconnectCallback;

    // Callback for switch commands
    SwitchCommandCallback switchCommandCallback;

    // Static pointer to the current instance
    static HomeAssistant* instance;

    // Static callback forwarders
    static void onMqttConnectedStatic();
    static void onMqttDisconnectedStatic();
    static void onSwitchCommandStatic(bool state, HASwitch* sender);

    // Arrays to store sensors and switches
    std::unique_ptr<HASensor> sensors[static_cast<uint8_t>(SensorType::SensorTypeCount)];
    std::unique_ptr<HASwitch> switches[static_cast<uint8_t>(SwitchType::SwitchTypeCount)];

    // Helper method to generate, devices, names and unique IDs
    HADevice buildDevice(const char* name, const char* firmware);
    const char* generateUniqueId(const char* name);
    const char* getSensorName(SensorType sensor);
    const char* getSwitchName(SwitchType sw);

public:
    // Constructor and Destructor
    HomeAssistant(WiFiClient& client, const char* devicename, const char* firmware);
    ~HomeAssistant();

    // Connect methods with callbacks
    bool connect(
        IPAddress broker, 
        const MqttConnectCallback& onConnected, 
        const MqttDisconnectCallback& onDisconnected,
        const char* username = nullptr,
        const char* password = nullptr
    );

        // Methods to add sensors and switches
    void addSensor(SensorType sensorType, const char * initialValue = nullptr, const char* icon = HomeAssistant::DEFAULT_ICON);
    void addSwitch(SwitchType switchType, bool initialState = false, const char* icon = HomeAssistant::DEFAULT_ICON);

    // Methods to modify sensor and switch values
    void setSensorValue(SensorType sensorType, const char *value);
    void setSwitchState(SwitchType switchType, bool state);

    // Method to set switch command callback
    void setSwitchCommandCallback(const SwitchCommandCallback& callback);

    // Loop method to maintain MQTT connection
    void loop() { if(mqtt) mqtt->loop(); }
};


#endif // HOMEASSISTANT_H