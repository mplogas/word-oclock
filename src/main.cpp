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

// callbacks
void handleFWUpload(AsyncWebServerRequest *request, const String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  Serial.printf("Upload: %s, Index: %u, Len: %u, Final: %u\n", filename.c_str(), index, len, final);
  if (!index)
  {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    { // start with max available size
      Update.printError(Serial);
    } else Serial.println("OTA update started!");
  }
  if (Update.write(data, len) != len)
  {
    Update.printError(Serial);
  } else Serial.printf("Written: %u\n", index + len);
  if (final)
  {
    if (Update.end(true))
    { // true to set the size to the current progress
      Serial.printf("UpdateSuccess: %u\nRebooting...\n", index + len);
    }
    else
    {
      Update.printError(Serial);
    }
  }
}

bool handleUpdateResult() {
    // Logic to determine if the update was successful
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

void handleSwitchCommand(SwitchType switchType, bool state) {
    Serial.printf("Switch command received for %s: %s\n", switchType, state ? "ON" : "OFF");
    // Additional logic to handle switch commands
}

void handleIlluminanceSensorUpdate(const int value) {
    Serial.printf("Illuminance sensor value: %d\n", value);

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


    ledController = LED();
    ledController.init();
    //ledController.enableAutoBrightness(systemConfig.autoBrightnessConfig.illuminanceThresholdHigh, systemConfig.autoBrightnessConfig.illuminanceThresholdLow);
    //ledController.setBrightness(255);

    timeConverter = new TimeConverterDE();
  
    if (systemConfig.mqttConfig.enabled)
    {
      enableMqtt();
    }


    webui.init(handleUpdateResult, handleFWUpload);
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
    if (now - lastUpdate > Defaults::LED_INTERVAL)
    {
      lastUpdate = now;
      if (isTick)
      {
        isTick = false;
        ledController.setColor(CRGB::Red);
        ledController.setLEDs(tickLEDs);
        Serial.println("Tick");
      }
      else
      {
        isTick = true;
        ledController.setColor(CRGB::Blue);
        ledController.setLEDs(tockLEDs);

        Serial.println("Tock");
      }

      FastLED.show();
    }

    ledController.loop();

    if (systemConfig.mqttConfig.enabled && homeAssistant != nullptr)
    {
      homeAssistant->loop();
    }
  }
}
