#ifndef HOMEASSISTANT_H
#define HOMEASSISTANT_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include <functional>
#include <memory>
#include "constants.h"

// Define callback types
using MqttConnectCallback = std::function<void()>;
using MqttDisconnectCallback = std::function<void()>;
using LedSwitchCommandCallback = std::function<void(bool)>;

class HomeAssistant
{
private:
    const uint8_t maclen = 6;

    // Using smart pointers for dynamic initialization
    std::unique_ptr<HAMqtt> mqtt;
    std::unique_ptr<HASensor> sensor;
    std::unique_ptr<HASwitch> led;
    HADevice device;

    // Callbacks for MQTT
    MqttConnectCallback mqttConnectCallback;
    MqttDisconnectCallback mqttDisconnectCallback;

    // Callback for LED switch commands
    LedSwitchCommandCallback ledSwitchCallback;

    // Static pointer to the current instance
    static HomeAssistant* instance;

    // Helper methods to build HA components
    HADevice buildDevice();
    std::unique_ptr<HASensor> buildSensor();
    std::unique_ptr<HASwitch> buildSwitch();

    // Static callback forwarders
    static void onMqttConnectedStatic();
    static void onMqttDisconnectedStatic();
    static void onSwitchCommandStatic(bool state, HASwitch* sender);

public:
    // Constructor and Destructor
    HomeAssistant(WiFiClient& client, const LedSwitchCommandCallback& ledCb);
    ~HomeAssistant();

    // Connect methods with callbacks
    bool connect(
        IPAddress broker, 
        const char* username, 
        const char* password, 
        const MqttConnectCallback& onConnected, 
        const MqttDisconnectCallback& onDisconnected
    );
    
    bool connect(
        IPAddress broker, 
        const MqttConnectCallback& onConnected, 
        const MqttDisconnectCallback& onDisconnected
    );

    // Loop method to maintain MQTT connection
    void loop() { if(mqtt) mqtt->loop(); }
};

// Initialize static member
HomeAssistant* HomeAssistant::instance = nullptr;

// Static callback forwarders
void HomeAssistant::onMqttConnectedStatic()
{
    if (instance && instance->mqttConnectCallback) {
        instance->mqttConnectCallback();
    }
}

void HomeAssistant::onMqttDisconnectedStatic()
{
    if (instance && instance->mqttDisconnectCallback) {
        instance->mqttDisconnectCallback();
    }
}

// Inside HomeAssistant class
void HomeAssistant::onSwitchCommandStatic(bool state, HASwitch* sender)
{
    if (instance && instance->ledSwitchCallback) {
        instance->ledSwitchCallback(state);
    }
}

// Constructor Implementation
HomeAssistant::HomeAssistant(WiFiClient& client, const LedSwitchCommandCallback& ledCb) 
    : ledSwitchCallback(ledCb)
{
    // Set the static instance pointer
    if (instance == nullptr) {
        instance = this;
    } else {
        // Handle multiple instances if necessary
        // For simplicity, we'll assume only one instance exists
        Serial.println("Warning: Multiple instances of HomeAssistant are not supported.");
    }

    // Build the HADevice first
    device = buildDevice();

    // Initialize HAMqtt with the built device
    mqtt = std::make_unique<HAMqtt>(client, device);
    mqtt->setDataPrefix("wc2");

    // Build and initialize sensor and switch
    sensor = buildSensor();
    led = buildSwitch();
}

// Destructor Implementation
HomeAssistant::~HomeAssistant()
{
    if (mqtt) {
        mqtt->disconnect();
        // No need to manually delete as std::unique_ptr handles it
        mqtt.reset();
    }

    // Clear the static instance pointer if it points to this object
    if (instance == this) {
        instance = nullptr;
    }
}

// Connect with username and password
bool HomeAssistant::connect(
    IPAddress broker, 
    const char* username, 
    const char* password, 
    const MqttConnectCallback& onConnected, 
    const MqttDisconnectCallback& onDisconnected
)
{
    if (!mqtt) {
        Serial.println("MQTT instance not initialized.");
        return false;
    }

    // Store the callbacks
    mqttConnectCallback = onConnected;
    mqttDisconnectCallback = onDisconnected;

    // Use static forwarders
    mqtt->onConnected(&HomeAssistant::onMqttConnectedStatic);
    mqtt->onDisconnected(&HomeAssistant::onMqttDisconnectedStatic);

    return mqtt->begin(broker, username, password);
}

// Connect without username and password
bool HomeAssistant::connect(
    IPAddress broker, 
    const MqttConnectCallback& onConnected, 
    const MqttDisconnectCallback& onDisconnected
)
{
    if (!mqtt) return false;

    // Store the callbacks
    mqttConnectCallback = onConnected;
    mqttDisconnectCallback = onDisconnected;

    // Use static forwarders
    mqtt->onConnected(&HomeAssistant::onMqttConnectedStatic);
    mqtt->onDisconnected(&HomeAssistant::onMqttDisconnectedStatic);

    return mqtt->begin(broker);
}

// Build HADevice
HADevice HomeAssistant::buildDevice()
{
    byte mac[maclen];
    WiFi.macAddress(mac);
    
    HADevice haDevice(mac, sizeof(mac));  
    haDevice.setName(PRODUCT);
    haDevice.setSoftwareVersion(FW_VERSION);
    haDevice.setModel("v1.0");
    haDevice.setManufacturer("digitalnatives Berlin");
    return haDevice;
}

// Build HASensor
std::unique_ptr<HASensor> HomeAssistant::buildSensor()
{
    auto localSensor = std::make_unique<HASensor>("wc-lightintensity");
    localSensor->setIcon("mdi:home");
    localSensor->setName("Light Intensity");
    return localSensor;
}

// Build HASwitch
std::unique_ptr<HASwitch> HomeAssistant::buildSwitch()
{
    auto localSwitch = std::make_unique<HASwitch>("wc-led");
    localSwitch->setIcon("mdi:lightbulb");
    localSwitch->setName("WordClock Light");
    
    // Use the stored callback for switch commands
    localSwitch->onCommand(&HomeAssistant::onSwitchCommandStatic);
    // localSwitch->onCommand([this](const char* state) {
    //     if (ledSwitchCallback) {
    //         ledSwitchCallback(state);
    //     }
    // });
    
    return localSwitch;
}

#endif // HOMEASSISTANT_H