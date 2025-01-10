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
            if (instance->switches[i] && instance->switches[i] == sender)
            {
                SwitchType switchType = static_cast<SwitchType>(i);
                instance->switchCommandCallback(switchType, state);
                break;
            }
        }
    }
}

// Constructor Implementation
HomeAssistant::HomeAssistant(WiFiClient& client, const char* name, const char* firmware) : device(), mqtt(client, device)
{
    // Set the static instance pointer
    if (instance == nullptr) {
        instance = this;
    } else {
        // Handle multiple instances is not supported
        Serial.println("Warning: Multiple instances of HomeAssistant are not supported.");
    }

    // // Build the HADevice first
    byte mac[maclen];
    WiFi.macAddress(mac);    
    // Extract the last two bytes of the MAC address
    uint8_t lastTwoBytes[2] = { mac[4], mac[5] };
    // Format the last two bytes as a hexadecimal string (4 characters)
    char macStr[5]; // 4 hex digits + null terminator
    snprintf(macStr, sizeof(macStr), "%02X%02X", lastTwoBytes[0], lastTwoBytes[1]);
    
    char uniqueid[strlen(Defaults::PRODUCT)+5]; // PRODUCT + "XXXX" (4) + null terminator (1)
    snprintf(uniqueid, sizeof(uniqueid), "%s_%s", Defaults::PRODUCT, macStr);
    Serial.print("Unique ID: ");
    Serial.println(uniqueid);

    //HADevice device("TestDevice-01");  
    device.setUniqueId(uniqueid); 
    device.setName(name);
    device.setSoftwareVersion(firmware);
    device.setModel(Defaults::FW_VERSION);
    device.setManufacturer("testwiese Berlin");

    // Initialize HAMqtt with the built device
    //HAMqtt test(client, device); // last number is the amount of devices/sensors implemented
}

// Destructor Implementation
HomeAssistant::~HomeAssistant()
{
    disconnect();

    // TODO: killing the sensors and switches causes a sigterm.
    // not killing them creates dangling pointers to the switch callbacks
    // meh, life sucks :(

    // for (uint8_t i = 0; i < static_cast<uint8_t>(SensorType::SensorTypeCount); i++)
    // {
    //     if (sensors[i]!= nullptr) delete sensors[i];
    // }
    // for (uint8_t i = 0; i < static_cast<uint8_t>(SwitchType::SwitchTypeCount); i++)
    // {
    //     if (switches[i]!= nullptr) delete switches[i];
    // }

    // Clear the static instance pointer if it points to this object
    if (instance == this) {
        instance = nullptr;
    }
}

void HomeAssistant::loop()
{
        mqtt.loop();
}

// Connect with username and password
bool HomeAssistant::connect(
    IPAddress broker,
    const MqttConnectCallback& onConnected,
    const MqttDisconnectCallback& onDisconnected,
    const char* username,
    const char* password,
    const char* defaultTopic
)
{
    // Store the callbacks
    mqttConnectCallback = onConnected;
    mqttDisconnectCallback = onDisconnected;

    // Use static forwarders
    mqtt.onConnected(&HomeAssistant::onMqttConnectedStatic);
    mqtt.onDisconnected(&HomeAssistant::onMqttDisconnectedStatic);

    if(defaultTopic) {
        mqtt.setDataPrefix(defaultTopic);
    } else {
        mqtt.setDataPrefix(Defaults::DEFAULT_MQTT_TOPIC);
    }

    //Connect with or without credentials
    if (username && password) {
        return mqtt.begin(broker, username, password);
    } else {
        return mqtt.begin(broker);
    }
    //return mqtt.begin(broker);
}

void HomeAssistant::disconnect()
{
    mqtt.disconnect();
    mqtt.onConnected(nullptr);
    mqtt.onDisconnected(nullptr);
    mqttConnectCallback = nullptr;
    mqttDisconnectCallback = nullptr;
}

// // Build HADevice
// void HomeAssistant::setupDevice(const char* name, const char* firmware)
// {
//     byte mac[maclen];
//     WiFi.macAddress(mac);
    
//     //HADevice haDevice(mac, sizeof(mac)); 
//     device = HADevice("TestDevice");
    
//     device.setName(name);
//     device.setSoftwareVersion(firmware);
//     device.setModel("v1.0");
//     device.setManufacturer("digitalnatives Berlin");
// }

const char* HomeAssistant::generateUniqueId(const char* name)
{
    // Static buffer to hold the unique ID
    static char uniqueId[64]; 

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

    sensors[index] = new HASensor(uniqueId);
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

    switches[index] = new HASwitch(uniqueId);
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