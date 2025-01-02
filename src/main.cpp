#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <RTClib.h>
#include <Update.h>
#include <vector>

#include "defaults.h"
#include "configuration.h"
#include "wifisetup.h"
#include "homeassistant.h"
#include "wclock.h"
#include "webui.h"
#include "leds.h"
#include "timeconverterde.h"
#include "callbacktypes.h"

boolean isSetup;

RTC_DS3231 rtc;
WiFiClient client;
Configuration config;
Configuration::SystemConfig systemConfig;
Configuration::LightConfig lightConfig;
Configuration::WifiConfig wifiConfig;
AsyncWebServer server(80);
WebUI webui(server);
WClock* wordClock;
HomeAssistant* homeAssistant;
ITimeConverter* timeConverter;
LED ledController;


bool initialized = false;
unsigned long lastUpdate = 0;
bool isDark = false;


// this if for (reduced) testing purposes
boolean isTick;
std::vector<std::pair<int, int>> tickLEDs = {
    {0,1},
    {1, 1},   // From LED index 0, turn on 3 LEDs (0 to 2)
    {6, 1}   // From LED index 6, turn on 3 LEDs (6 to 9)
};
std::vector<std::pair<int, int>> tockLEDs = {
  {0,1},
    {3, 1}   // From LED index 3, turn on 5 LEDs (3 to 5)
};

void showCurrentTime() {
  if(!isDark) {
    uint8_t hour = wordClock->getHour();  
    uint8_t minute = wordClock->getMinute();
    // Serial.printf("Current time: %d:%d\n", hour, minute);
    std::vector<std::pair<int, int>> leds = timeConverter->convertTime(hour, minute, true, true);
    // Serial.printf("LEDs: %d\n", leds.size());
    ledController.setLEDs(leds);
  }
}

// callbacks
void handleFWUpload(UpdateType updateType, const String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("Update type: %s\n", updateType == UpdateType::FIRMWARE ? "Firmware" : "Filesystem");
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, updateType == UpdateType::FIRMWARE ? U_FLASH : U_SPIFFS ))
    { // start with max available size
      Update.printError(Serial);
    } else Serial.println("OTA update started!");
  }

  if (Update.write(data, len) != len)
  {
    Update.printError(Serial);
  } // else Serial.printf("Written: %u\n", index + len);
  
  if (final)
  {
    if (Update.end(true))
    { 
      Serial.printf("UpdateSuccess: %u\n", index + len);
    }
    else
    {
      Update.printError(Serial);
    }
  }
}

bool isUpdateSuccess() {
    return !Update.hasError();
}

void handleWiFiCredentials(const String &ssid, const String &password) {
    Serial.printf("SSID set to: %s\n", ssid.c_str());
    Serial.printf("Password set to: %s\n", password.c_str());

    strncpy(wifiConfig.ssid, ssid.c_str(), sizeof(wifiConfig.ssid) - 1);
    wifiConfig.ssid[sizeof(wifiConfig.ssid) - 1] = '\0';
    strncpy(wifiConfig.password, password.c_str(), sizeof(wifiConfig.password) - 1);
    wifiConfig.password[sizeof(wifiConfig.password) - 1] = '\0';
    config.setWifiConfig(wifiConfig);
}

// Callback for MQTT connected
void handleMqttConnected() {
    Serial.println("MQTT Connected Successfully!");
    // Additional logic upon successful connection
}

// Callback for MQTT disconnected
void handleMqttDisconnected() {
    Serial.println("MQTT Disconnected!");
    // Additional logic upon disconnection
}

void lightOperationHandler(LightOperationType operation, const String& value) {
  switch (operation) {
    case LightOperationType::ToggleStatus: {
      bool status = (value == "1");
      if(status) {
        isDark = false;
        showCurrentTime();
      } else {
        isDark = true;
        ledController.clearLEDs();
      }
      lightConfig.state = status;
      config.setLightState(lightConfig.state);
      break;
    }
    case LightOperationType::SetColor: {
      ledController.setColor(ledController.HexToRGB(value));
      showCurrentTime();
      strncpy(lightConfig.color, value.c_str(), sizeof(lightConfig.color) - 1);
      config.setLightColor(lightConfig.color);
      break;
    }
    case LightOperationType::SetAutoBrightness: {
      bool enabled = (value == "1");
      if(enabled) {
        ledController.enableAutoBrightness(lightConfig.autoBrightnessConfig.illuminanceThresholdHigh, lightConfig.autoBrightnessConfig.illuminanceThresholdLow);

      } else {
        ledController.disableAutoBrightness();
      }
      lightConfig.autoBrightnessConfig.enabled = enabled;
      config.setAutoBrightness(lightConfig.autoBrightnessConfig);
      break;
    }
    case LightOperationType::SetBrightness: {
      uint8_t brightness = value.toInt();
      ledController.setBrightness(brightness);
      lightConfig.brightness = brightness;
      config.setLightBrightness(lightConfig.brightness);
      break;
    }
  }
}

void systemOperationHandler(SystemOperationType operation, const std::map<String, String>& params) {
  switch (operation) {
    case SystemOperationType::SetHaIntegration: {
      // Additional logic to handle HA integration
      break;
    }
    case SystemOperationType::SetNTPTime: {
      // Additional logic to handle NTP time
      break;
    }
    case SystemOperationType::SetNtpAutoUpdate: {
      // Additional logic to handle NTP auto update
      break;
    }
    case SystemOperationType::SetLightSchedule: {
      // Additional logic to handle light schedule
      break;
    }
    case SystemOperationType::SetClockFormat: {
      // Additional logic to handle clock format
      break;
    }
    case SystemOperationType::ResetConfig: {
      // Additional logic to reset configuration
      break;
    }
  }


  // switch (operation) {
  //   case WebUI::SystemOperationType::SetNTPServer: {
  //     strncpy(systemConfig.ntpConfig.server, value.c_str(), sizeof(systemConfig.ntpConfig.server) - 1);
  //     systemConfig.ntpConfig.server[sizeof(systemConfig.ntpConfig.server) - 1] = '\0';
  //     config.setSystemConfig(systemConfig);
  //     break;
  //   }
  //   case WebUI::SystemOperationType::SetTimezone: {
  //     strncpy(systemConfig.ntpConfig.timezone, value.c_str(), sizeof(systemConfig.ntpConfig.timezone) - 1);
  //     systemConfig.ntpConfig.timezone[sizeof(systemConfig.ntpConfig.timezone) - 1] = '\0';
  //     config.setSystemConfig(systemConfig);
  //     break;
  //   }
  //   case WebUI::SystemOperationType::SetMQTTConfig: {
  //     strncpy(systemConfig.mqttConfig.host, value.c_str(), sizeof(systemConfig.mqttConfig.host) - 1);
  //     systemConfig.mqttConfig.host[sizeof(systemConfig.mqttConfig.host) - 1] = '\0';
  //     config.setSystemConfig(systemConfig);
  //     break;
  //   }
  //   case WebUI::SystemOperationType::SetMQTTUser: {
  //     strncpy(systemConfig.mqttConfig.user, value.c_str(), sizeof(systemConfig.mqttConfig.user) - 1);
  //     systemConfig.mqttConfig.user[sizeof(systemConfig.mqttConfig.user) - 1] = '\0';
  //     config.setSystemConfig(systemConfig);
  //     break;
  //   }
  //   case WebUI::SystemOperationType::SetMQTTPass: {
  //     strncpy(systemConfig.mqttConfig.pass, value.c_str(), sizeof(systemConfig.mqttConfig.pass) - 1);
  //     systemConfig.mqttConfig.pass[sizeof(systemConfig.mqttConfig.pass) - 1] = '\0';
  //     config.setSystemConfig(systemConfig);
  //     break;
  //   }
  //   case WebUI::SystemOperationType::SetMQTTTopic: {
  //     strncpy(systemConfig.mqttConfig.topic, value.c_str(), sizeof(systemConfig.mqttConfig.topic) - 1);
  //     systemConfig.mqttConfig.topic[sizeof(systemConfig.mqttConfig.topic) - 1] = '\0';
  //     config.setSystemConfig(systemConfig);
  //     break;
  //   }
  // }
}

// home assistant callbacks
void handleSwitchCommand(SwitchType switchType, bool state) {
    Serial.printf("Switch command received for %s: %s\n", switchType, state ? "ON" : "OFF");
    // Additional logic to handle switch commands
}

void handleIlluminanceSensorUpdate(const int value) {
    //Serial.printf("Illuminance sensor value: %d\n", value);

    if (systemConfig.mqttConfig.enabled && homeAssistant != nullptr) {
      char buf[8];
      itoa(value, buf, 10);
      homeAssistant->setSensorValue(SensorType::LightIntensity, buf);
    }
}

void enableMqtt() {
  if(systemConfig.mqttConfig.host != nullptr) {
      homeAssistant = new HomeAssistant(client, Defaults::PRODUCT, Defaults::FW_VERSION);
      homeAssistant->addSensor(SensorType::LightIntensity, "0", "mdi:brightness-5");
      homeAssistant->addSwitch(SwitchType::LED, false, "mdi:lightbulb");
      homeAssistant->setSwitchCommandCallback(handleSwitchCommand);

      ledController.registerIlluminanceSensorCallback(handleIlluminanceSensorUpdate);
      homeAssistant->connect(IPAddress(systemConfig.mqttConfig.host), handleMqttConnected, handleMqttDisconnected);
  }
}

void setup()
{
  isTick = true;
  Serial.begin(115200);
  Serial.printf("Starting with FW %s...\n", Defaults::FW_VERSION);

  config = Configuration();
  config.init();
  wifiConfig = config.getWifiConfig();
  systemConfig = config.getSystemConfig();
  lightConfig = config.getLightConfig();

  wordClock = new WClock(rtc);
  if (!wordClock->init())
  {
    Serial.println("RTC may lack power or may be missing");
    Serial.flush();
    abort();
  }

  if (!LittleFS.begin(true))
  {
      Serial.println("An error has occurred while mounting LittleFS");
      Serial.flush();
      abort();
  }
  Serial.println("LittleFS mounted successfully");


  WifiSetup wifiSetup = WifiSetup();
  if (wifiConfig.ssid != nullptr && wifiConfig.password != nullptr && 
        wifiSetup.connect(wifiConfig.ssid, wifiConfig.password, Defaults::WIFI_SCAN_TIMEOUT))
  {
    isSetup = false;

    bool clockResult = wordClock->begin(systemConfig.ntpConfig.timezone, systemConfig.ntpConfig.server);

    if(!clockResult) {
      Serial.println("Failed to initialize clock");
    } else {
      Serial.println("Successfully intitialized clock");
    }

    // Serial.printf("Light state recovered: %s\n", lightConfig.state ? "ON" : "OFF");
    // Serial.printf("Light color recovered: %s\n", lightConfig.color);
    // Serial.printf("Light brightness recovered: %d\n", lightConfig.brightness);
    // Serial.printf("Auto brightness state recovered: %s\n", lightConfig.autoBrightnessConfig.enabled ? "true" : "false");


    ledController = LED();
    ledController.init();
    if(lightConfig.autoBrightnessConfig.enabled) {
      ledController.enableAutoBrightness(lightConfig.autoBrightnessConfig.illuminanceThresholdHigh, lightConfig.autoBrightnessConfig.illuminanceThresholdLow);
    } else {
      ledController.setBrightness(lightConfig.brightness);
    } 
    ledController.setColor(ledController.HexToRGB(lightConfig.color));

    timeConverter = new TimeConverterDE();
  
    if (systemConfig.mqttConfig.enabled)
    {
      enableMqtt();
    }


    webui.init(lightOperationHandler, systemOperationHandler, handleFWUpload, isUpdateSuccess, &lightConfig, &systemConfig);
  }
  else
  {
    isSetup = true;
    if (!wifiSetup.enableHostAp(Defaults::PRODUCT, Defaults::DEFAULT_WIFI_SETUP_PASS))
    {
      Serial.println("Failed to start AP");
      Serial.flush();
      abort();
    } else {
        webui.initHostAP(handleWiFiCredentials);
    }
  }

  initialized = true;
  ledController.test();
}



void loop()
{
  if (!isSetup && initialized)
  {
    unsigned long now = millis();
    wordClock->loop();
    if (now - lastUpdate > Defaults::LED_INTERVAL)
    {
      lastUpdate = now;
      showCurrentTime();
    }
    
    ledController.loop();
    if (systemConfig.mqttConfig.enabled && homeAssistant != nullptr)
    {
      homeAssistant->loop();
    }
  }
}
