#include "homeassistant.h"

// Initialize static member
HomeAssistant* HomeAssistant::instance = nullptr;

// Sensor names in PROGMEM
const char sensorName_LightIntensity[] PROGMEM = "Light Intensity";
const char sensorName_LightColor[] PROGMEM    = "Light Color";
const char sensorName_TimeUpdateInterval[] PROGMEM    = "TimeUpdate Interval";

// Array of sensor names
const char* const sensorNames[] PROGMEM = {
    sensorName_LightIntensity,
    sensorName_LightColor,
    sensorName_TimeUpdateInterval
};

// Switch names in PROGMEM
const char switchName_LED[] PROGMEM  = "LEDs";
const char switchName_AutoUpdate[] PROGMEM  = "TimeUpdate Automation";

// Array of switch names
const char* const switchNames[] PROGMEM = {
    switchName_LED,
    switchName_AutoUpdate,
};


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

void HomeAssistant::onSwitchCommandStatic(bool state, HASwitch* sender)
{
    if (instance && instance->switchCommandCallback)
    {
        // Find the SwitchType corresponding to the sender
        for (uint8_t i = 0; i < static_cast<uint8_t>(SwitchType::SwitchTypeCount); ++i)
        {
            if (instance->switches[i] && instance->switches[i].get() == sender)
            {
                SwitchType switchType = static_cast<SwitchType>(i);
                instance->switchCommandCallback(switchType, state);
                break;
            }
        }
    }
}

// Constructor Implementation
HomeAssistant::HomeAssistant(WiFiClient& client, const char* name, const char* firmware)
{
    // Set the static instance pointer
    if (instance == nullptr) {
        instance = this;
    } else {
        // Handle multiple instances is not supported
        Serial.println("Warning: Multiple instances of HomeAssistant are not supported.");
    }

    // Build the HADevice first
    device = buildDevice(name, firmware);

    // Initialize HAMqtt with the built device
    mqtt = std::make_unique<HAMqtt>(client, device);
    mqtt->setDataPrefix("wc2");

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
    const MqttConnectCallback& onConnected,
    const MqttDisconnectCallback& onDisconnected,
    const char* username,
    const char* password
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

    // Connect with or without credentials
    if (username && password) {
        return mqtt->begin(broker, username, password);
    } else {
        return mqtt->begin(broker);
    }
}

// Build HADevice
HADevice HomeAssistant::buildDevice(const char* name, const char* firmware)
{
    byte mac[maclen];
    WiFi.macAddress(mac);
    
    HADevice haDevice(mac, sizeof(mac));  
    haDevice.setName(name);
    haDevice.setSoftwareVersion(firmware);
    haDevice.setModel("v1.0");
    haDevice.setManufacturer("digitalnatives Berlin");
    return haDevice;
}

// // Build HASensor
// std::unique_ptr<HASensor> HomeAssistant::buildSensor()
// {
//     auto localSensor = std::make_unique<HASensor>("wc-lightintensity");
//     localSensor->setIcon("mdi:home");
//     localSensor->setName("Light Intensity");
//     return localSensor;
// }

// // Build HASwitch
// std::unique_ptr<HASwitch> HomeAssistant::buildSwitch()
// {
//     auto localSwitch = std::make_unique<HASwitch>("wc-led");
//     localSwitch->setIcon("mdi:lightbulb");
//     localSwitch->setName("WordClock Light");
    
//     // Use the stored callback for switch commands
//     localSwitch->onCommand(&HomeAssistant::onSwitchCommandStatic);
//     // localSwitch->onCommand([this](const char* state) {
//     //     if (ledSwitchCallback) {
//     //         ledSwitchCallback(state);
//     //     }
//     // });
    
//     return localSwitch;
// }

const char* HomeAssistant::generateUniqueId(const char* name)
{
    // Static buffer to hold the unique ID
    static char uniqueId[64]; // Adjust the size as needed

    const char* deviceId = device.getUniqueId();

    // Build the unique ID
    snprintf(uniqueId, sizeof(uniqueId), "%s-%s", deviceId, name);
    uniqueId[sizeof(uniqueId) - 1] = '\0'; // Ensure null-termination

    return uniqueId;
}

const char* HomeAssistant::getSensorName(SensorType sensor)
{
    // Static buffer to hold the name
    static char buffer[30]; // Adjust the size as needed

    uint8_t index = static_cast<uint8_t>(sensor);
    if (index >= static_cast<uint8_t>(SensorType::SensorTypeCount)) {
        buffer[0] = '\0'; // Empty buffer for invalid index
        return buffer;
    }

    // Read the address of the sensor name from PROGMEM
    const char* namePtr = (const char*)pgm_read_ptr(&sensorNames[index]);

    // Copy the string from PROGMEM to RAM
    strncpy_P(buffer, namePtr, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination

    return buffer;
}

const char* HomeAssistant::getSwitchName(SwitchType sw)
{
    // Static buffer to hold the name
    static char buffer[30]; // Adjust the size as needed

    uint8_t index = static_cast<uint8_t>(sw);
    if (index >= static_cast<uint8_t>(SwitchType::SwitchTypeCount)) {
        buffer[0] = '\0'; // Empty buffer for invalid index
        return buffer;
    }

    // Read the address of the switch name from PROGMEM
    const char* namePtr = (const char*)pgm_read_ptr(&switchNames[index]);

    // Copy the string from PROGMEM to RAM
    strncpy_P(buffer, namePtr, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination

    return buffer;
}

void HomeAssistant::addSensor(SensorType sensorType, const char *initialValue, const char* icon)
{
    uint8_t index = static_cast<uint8_t>(sensorType);

    const char* name = getSensorName(sensorType);
    const char* uniqueId = generateUniqueId(name);

    sensors[index] = std::make_unique<HASensor>(uniqueId);
    sensors[index]->setName(name);
    if (icon) sensors[index]->setIcon(icon);
    if (initialValue) {
        sensors[index]->setValue(initialValue);
    }
}

void HomeAssistant::addSwitch(SwitchType switchType, bool initialState, const char* icon)
{
    uint8_t index = static_cast<uint8_t>(switchType);

    const char* name = getSwitchName(switchType);
    const char* uniqueId = generateUniqueId(name);

    switches[index] = std::make_unique<HASwitch>(uniqueId);
    switches[index]->setName(name);
    if (icon) switches[index]->setIcon(icon);
    switches[index]->setState(initialState);

    // Set up the command callback
    switches[index]->onCommand(&HomeAssistant::onSwitchCommandStatic);
}

void HomeAssistant::setSensorValue(SensorType sensorType, const char *value)
{
    uint8_t index = static_cast<uint8_t>(sensorType);
    if (sensors[index]) {
        sensors[index]->setValue(value);
    }
}

void HomeAssistant::setSwitchState(SwitchType switchType, bool state)
{
    uint8_t index = static_cast<uint8_t>(switchType);
    if (switches[index]) {
        switches[index]->setState(state);
    }
}

void HomeAssistant::setSwitchCommandCallback(const SwitchCommandCallback& callback)
{
    switchCommandCallback = callback;
}