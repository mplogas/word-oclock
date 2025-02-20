#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <RTClib.h>
#include <Update.h>
#include <vector>

#include "defaults.h"
#include "configuration.h"
#include "wifisetup.h"
#include "wclock.h"
#include "webui.h"
#include "leds.h"
#include "homeassistant.h"
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
WClock *wordClock;
WoC_MQTT *haMqtt;
ITimeConverter *timeConverter;
LED ledController;

bool initialized = false;
unsigned long lastUpdate = 0;
bool pushStatus = false;
uint8_t lastHour = 0;
uint8_t lastMinute = 0;

void showCurrentTime(uint8_t hour, uint8_t minute)
{
  //Serial.printf("Current time: %d:%d\n", hour, minute);
  std::vector<std::pair<int, int>> leds = timeConverter->convertTime(hour, minute, (systemConfig.mode == Configuration::ClockMode::Regular), true);
  //Serial.printf("LEDs: %d\n", leds.size());
  ledController.setLEDs(leds);
}

// callbacks
void handleFWUpload(UpdateType updateType, const String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("Update type: %s\n", updateType == UpdateType::FIRMWARE ? "Firmware" : "Filesystem");
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, updateType == UpdateType::FIRMWARE ? U_FLASH : U_SPIFFS))
    { // start with max available size
      Update.printError(Serial);
    }
    else
      Serial.println("OTA update started!");
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

bool isUpdateSuccess()
{
  return !Update.hasError();
}

void handleWiFiCredentials(const String &ssid, const String &password)
{
  Serial.printf("SSID set to: %s\n", ssid.c_str());
  // Serial.printf("Password set to: %s\n", password.c_str());

  strlcpy(wifiConfig.ssid, ssid.c_str(), sizeof(wifiConfig.ssid));
  strlcpy(wifiConfig.password, password.c_str(), sizeof(wifiConfig.password));
  config.setWifiConfig(wifiConfig);
}

void setColor(const char *color)
{
  ledController.setColor(ledController.HexToRGB(color));
  strlcpy(lightConfig.color, color, sizeof(lightConfig.color));
  config.setLightColor(lightConfig.color);
}

void setBrightness(uint8_t brightness)
{
  ledController.setBrightness(brightness);
  lightConfig.brightness = brightness;
  config.setLightBrightness(lightConfig.brightness);
}

void setAutoBrightness(bool enabled)
{
  if (enabled)
  {
    ledController.enableAutoBrightness(lightConfig.autoBrightnessConfig.illuminanceThresholdHigh, lightConfig.autoBrightnessConfig.illuminanceThresholdLow);
  }
  else
  {
    ledController.disableAutoBrightness();
    ledController.setBrightness(lightConfig.brightness);
  }

  lightConfig.autoBrightnessConfig.enabled = enabled;
  config.setAutoBrightness(lightConfig.autoBrightnessConfig);
}

void setLightState(bool state)
{
  if (state)
  {
    ledController.setDark(false);
    showCurrentTime(lastHour, lastMinute);
  }
  else
  {
    ledController.setDark();
  }

  lightConfig.state = state;
  config.setLightState(lightConfig.state);
}

void lightSensorCallback(const int value)
{
  if (systemConfig.mqttConfig.enabled && haMqtt != nullptr)
  {
    haMqtt->setLightSensorValue(value);
  }
}

void pushInitialStatusToMqtt()
{
  if (systemConfig.mqttConfig.enabled && haMqtt != nullptr)
  {
    haMqtt->toggleLightState(lightConfig.state);
    haMqtt->setLightColor(lightConfig.color);
    haMqtt->setLightBrightness(lightConfig.brightness);
    haMqtt->toggleAutoBrightness(lightConfig.autoBrightnessConfig.enabled);
  }
}

void mqttCallback(MQTTEvent event, const char* payload) {
  switch (event)
  {
  case MQTTEvent::Connected:
    if(pushStatus) {
      pushInitialStatusToMqtt();
      pushStatus = false;
    }
    break;
  case MQTTEvent::Disconnected:
    Serial.println("MQTT Disconnected!");
    break;
  case MQTTEvent::BrightnessCommand:
    setBrightness(atoi(payload));
    break;
  case MQTTEvent::RGBCommand:
    setColor(payload);
    break;
  case MQTTEvent::StateCommand:
    setLightState(payload == "1"); //hmmm
    break;
  case MQTTEvent::AutoBrightnessSwitchCommand:
    setAutoBrightness(payload == "1");
    break;
  case MQTTEvent::Option1SwitchCommand:
    Serial.printf("Option1 switch command received: %s\n", payload);
    break;
  case MQTTEvent::Option2SwitchCommand:
    Serial.printf("Option2 switch command received: %s\n", payload);
    break;
  case MQTTEvent::Option3SwitchCommand:
    Serial.printf("Option3 switch command received: %s\n", payload);
    break;
  case MQTTEvent::Option4SwitchCommand:
    Serial.printf("Option4 switch command received: %s\n", payload);
    break;  
  default:
    break;
  }
}

void enableMqtt()
{
  if (systemConfig.mqttConfig.host != nullptr)
  {
    haMqtt = new WoC_MQTT(client, Defaults::PRODUCT, Defaults::FW_VERSION);
    haMqtt->connect(IPAddress(systemConfig.mqttConfig.host), mqttCallback, systemConfig.mqttConfig.username, systemConfig.mqttConfig.password, systemConfig.mqttConfig.topic);
    ledController.registerIlluminanceSensorCallback(lightSensorCallback);
    pushStatus = true;
  }
}

void disableMqtt()
{
  if (haMqtt != nullptr)
  {
    ledController.unregisterIlluminanceSensorCallback();
    haMqtt->disconnect();
    delete haMqtt;
    haMqtt = nullptr;
  }
}


void httpRequestCallback(ControlType type, const std::map<String, String> &params)
{
  // Serial.printf("Control type: %d\n", type);
  // Serial.printf("Params: %d\n", params.size());
  // for (auto const &x : params)
  // {
  //   Serial.printf("Key: %s, Value: %s\n", x.first.c_str(), x.second.c_str());
  // }

  switch (type)
  {
  case ControlType::LightStatus:
  {
    setLightState(params.at(FPSTR(WebUI::PARAM_ENABLED)) == FPSTR(WebUI::VALUE_ON));
    if(systemConfig.mqttConfig.enabled && haMqtt != nullptr) {
      haMqtt->toggleLightState(lightConfig.state);
    }
    break;
  }
  case ControlType::Color:
  {
    setColor(params.at(FPSTR(WebUI::PARAM_COLOR)).c_str());

    if(systemConfig.mqttConfig.enabled && haMqtt != nullptr) {
      haMqtt->setLightColor(lightConfig.color);
    }
    break;
  }
  case ControlType::AutoBrightness:
  {
    setAutoBrightness(params.at(FPSTR(WebUI::PARAM_AUTO_BRIGHTNESS_ENABLED)) == FPSTR(WebUI::VALUE_ON));
    if(systemConfig.mqttConfig.enabled && haMqtt != nullptr) {    
      haMqtt->toggleAutoBrightness(lightConfig.autoBrightnessConfig.enabled);
      if(!lightConfig.autoBrightnessConfig.enabled) {
        haMqtt->setLightBrightness(lightConfig.brightness);
      }
    }
    break;
  }
  case ControlType::Brightness:
  {
    setBrightness(params.at(FPSTR(WebUI::PARAM_BRIGHTNESS)).toInt());
    if(systemConfig.mqttConfig.enabled && haMqtt != nullptr) {
      haMqtt->setLightBrightness(lightConfig.brightness);
    }

    break;
  }
  case ControlType::HaIntegration:
  {
    systemConfig.mqttConfig.enabled = (params.at(FPSTR(WebUI::PARAM_ENABLED)) == FPSTR(WebUI::VALUE_ON));

    if (systemConfig.mqttConfig.enabled)
    {
      strlcpy(systemConfig.mqttConfig.host, params.at(FPSTR(WebUI::PARAM_BROKER_HOST)).c_str(), sizeof(systemConfig.mqttConfig.host));
      systemConfig.mqttConfig.port = params.at(FPSTR(WebUI::PARAM_BROKER_PORT)).toInt();
      strlcpy(systemConfig.mqttConfig.username, params.at(FPSTR(WebUI::PARAM_BROKER_USER)).c_str(), sizeof(systemConfig.mqttConfig.username));
      strlcpy(systemConfig.mqttConfig.password, params.at(FPSTR(WebUI::PARAM_BROKER_PASS)).c_str(), sizeof(systemConfig.mqttConfig.password));
      strlcpy(systemConfig.mqttConfig.topic, params.at(FPSTR(WebUI::PARAM_BROKER_DEFAULT_TOPIC)).c_str(), sizeof(systemConfig.mqttConfig.topic));
      config.setMqttConfig(systemConfig.mqttConfig);

      enableMqtt();
    }
    else
    {
      config.setMqttConfig(systemConfig.mqttConfig);
      disableMqtt();
    }
    break;
  }
  case ControlType::ClockFace:
  {
    if (params.at(FPSTR(WebUI::PARAM_OPTION)) == FPSTR(WebUI::VALUE_OFF))
    {
      systemConfig.mode = Configuration::ClockMode::Regular;
      Serial.println("Clock mode set to dreiviertel");
    }
    else
    {
      systemConfig.mode = Configuration::ClockMode::Option_1;
      Serial.println("Clock mode set to viertel vor");
    }
    config.setClockMode(systemConfig.mode);
    break;
  }
  case ControlType::ResetConfig:
  {
    config.reset();
    Serial.println("Configuration reset. Restarting...");
    delay(2000);
    ESP.restart();
    break;
  }
  case ControlType::Time:
  {
    // auto muh = params.at(FPSTR(WebUI::PARAM_TIME));
    // Serial.printf("Hour: %d\n", muh.substring(0,2).toInt());
    // Serial.printf("Minute: %d\n", muh.substring(3,5).toInt());
    wordClock->setTime(params.at(FPSTR(WebUI::PARAM_TIME)).substring(0, 2).toInt(), params.at(FPSTR(WebUI::PARAM_TIME)).substring(3, 5).toInt());
    break;
  }
  case ControlType::NTPSync:
  {
    systemConfig.ntpConfig.enabled = params.at(FPSTR(WebUI::PARAM_NTP_ENABLED)) == FPSTR(WebUI::VALUE_ON);

    if(systemConfig.ntpConfig.enabled) {
      strlcpy(systemConfig.ntpConfig.server, params.at(FPSTR(WebUI::PARAM_NTP_HOST)).c_str(), sizeof(systemConfig.ntpConfig.server));
      systemConfig.ntpConfig.interval = params.at(FPSTR(WebUI::PARAM_NTP_UPDATE_INTERVAL)).toInt();
      strlcpy(systemConfig.ntpConfig.timezone, params.at(FPSTR(WebUI::PARAM_NTP_TIMEZONE)).c_str(), sizeof(systemConfig.ntpConfig.timezone));
      config.setNtpConfig(systemConfig.ntpConfig);
  
      Serial.printf("NTP enabled with server: %s, interval: %d, timezone: %s\n", systemConfig.ntpConfig.server, systemConfig.ntpConfig.interval, systemConfig.ntpConfig.timezone);
      wordClock->enableNTP(systemConfig.ntpConfig.timezone, systemConfig.ntpConfig.server, systemConfig.ntpConfig.interval);
    } else {
      config.setNtpConfig(systemConfig.ntpConfig);
      wordClock->disableNTP();
    }

    break;
  }
  case ControlType::LightSchedule:
  {
    systemConfig.lightScheduleConfig.enabled = params.at(FPSTR(WebUI::PARAM_SCHEDULE_ENABLED)) == FPSTR(WebUI::VALUE_ON);

    if(systemConfig.lightScheduleConfig.enabled) {    
      systemConfig.lightScheduleConfig.startTime = params.at(FPSTR(WebUI::PARAM_SCHEDULE_START)).toInt();
      systemConfig.lightScheduleConfig.endTime = params.at(FPSTR(WebUI::PARAM_SCHEDULE_END)).toInt();

      config.setLightSchedule(systemConfig.lightScheduleConfig);
      wordClock->enableSchedule(systemConfig.lightScheduleConfig.startTime, systemConfig.lightScheduleConfig.endTime);
    } else {

      config.setLightSchedule(systemConfig.lightScheduleConfig);
      wordClock->disableSchedule();
    }
    break;
  }
  case ControlType::WiFiSetup:
  {
    strlcpy(wifiConfig.ssid, params.at(FPSTR(WebUI::PARAM_WIFI_SSID)).c_str(), sizeof(wifiConfig.ssid));
    strlcpy(wifiConfig.password, params.at(FPSTR(WebUI::PARAM_WIFI_PASS)).c_str(), sizeof(wifiConfig.password));
    config.setWifiConfig(wifiConfig);
    break;
  }
  default:
  {
    break;
  }
  }
}

const std::map<String, String> httpResponseCallback(PageType page)
{
  std::map<String, String> params;
  switch (page)
  {
  case PageType::LIGHT:
    params[FPSTR(WebUI::PARAM_ENABLED)] = lightConfig.state ? FPSTR(WebUI::VALUE_ON) : FPSTR(WebUI::VALUE_OFF);
    params[FPSTR(WebUI::PARAM_COLOR)] = lightConfig.color;
    params[FPSTR(WebUI::PARAM_BRIGHTNESS)] = String(lightConfig.brightness);
    params[FPSTR(WebUI::PARAM_AUTO_BRIGHTNESS_ENABLED)] = lightConfig.autoBrightnessConfig.enabled ? FPSTR(WebUI::VALUE_ON) : FPSTR(WebUI::VALUE_OFF);
    break;
  case PageType::SYSTEM:
    params[FPSTR(WebUI::PARAM_BROKER_ENABLED)] = systemConfig.mqttConfig.enabled ? FPSTR(WebUI::VALUE_ON) : FPSTR(WebUI::VALUE_OFF);
    params[FPSTR(WebUI::PARAM_BROKER_HOST)] = systemConfig.mqttConfig.host;
    params[FPSTR(WebUI::PARAM_BROKER_PORT)] = String(systemConfig.mqttConfig.port);
    params[FPSTR(WebUI::PARAM_BROKER_USER)] = systemConfig.mqttConfig.username;
    params[FPSTR(WebUI::PARAM_BROKER_PASS)] = systemConfig.mqttConfig.password;
    params[FPSTR(WebUI::PARAM_BROKER_DEFAULT_TOPIC)] = systemConfig.mqttConfig.topic;
    params[FPSTR(WebUI::PARAM_CLOCKFACE_OPTION)] = systemConfig.mode == Configuration::ClockMode::Option_1 ? FPSTR(WebUI::VALUE_ON) : FPSTR(WebUI::VALUE_OFF);
    break;
  case PageType::TIME:
  {
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", lastHour, lastMinute);
    params[FPSTR(WebUI::PARAM_TIME)] = timeStr;
    params[FPSTR(WebUI::PARAM_NTP_ENABLED)] = systemConfig.ntpConfig.enabled ? FPSTR(WebUI::VALUE_ON) : FPSTR(WebUI::VALUE_OFF);
    params[FPSTR(WebUI::PARAM_NTP_HOST)] = systemConfig.ntpConfig.server;
    params[FPSTR(WebUI::PARAM_NTP_UPDATE_INTERVAL)] = String(systemConfig.ntpConfig.interval);
    params[FPSTR(WebUI::PARAM_NTP_TIMEZONE)] = systemConfig.ntpConfig.timezone;
    params[FPSTR(WebUI::PARAM_SCHEDULE_START)] = String(systemConfig.lightScheduleConfig.startTime); 
    params[FPSTR(WebUI::PARAM_SCHEDULE_END)] = String(systemConfig.lightScheduleConfig.endTime);
    params[FPSTR(WebUI::PARAM_SCHEDULE_ENABLED)] = systemConfig.lightScheduleConfig.enabled ? FPSTR(WebUI::VALUE_ON) : FPSTR(WebUI::VALUE_OFF);
    break;
  }
  case PageType::FWUPDATE:
    params[FPSTR(WebUI::PARAM_FW_VERSION)] = Defaults::FW_VERSION;
    break;
  default:
    break;
  }
  return params;
}

void clockSchedulerCallback(SchedulerType type, uint8_t hour, uint8_t minute)
{
  lastHour = hour;
  lastMinute = minute;

  switch (type)
  {
  case SchedulerType::Timestamp:
    showCurrentTime(hour, minute);
    break;
  case SchedulerType::ScheduleStart:
    Serial.printf("Schedule start: %d:%d\n", hour, minute);
    ledController.setDark(false);
    showCurrentTime(hour, minute);
    break;
  case SchedulerType::ScheduleEnd:
    Serial.printf("Schedule end: %d:%d\n", hour, minute);
    ledController.setDark();
    break;
  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.printf("Starting with FW %s...\n", Defaults::FW_VERSION);

  config = Configuration();
  config.init();
  wifiConfig = config.getWifiConfig();
  systemConfig = config.getSystemConfig();
  lightConfig = config.getLightConfig();

  wordClock = new WClock(rtc);
  if (!wordClock->init(clockSchedulerCallback))
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

    if (systemConfig.ntpConfig.enabled)
    {
      wordClock->enableNTP(systemConfig.ntpConfig.timezone, systemConfig.ntpConfig.server, systemConfig.ntpConfig.interval);
    }
    else
    {
      wordClock->setTimeZone(systemConfig.ntpConfig.timezone);
    }

    ledController = LED();
    ledController.init();
    if (lightConfig.autoBrightnessConfig.enabled)
    {
      ledController.enableAutoBrightness(lightConfig.autoBrightnessConfig.illuminanceThresholdHigh, lightConfig.autoBrightnessConfig.illuminanceThresholdLow);
    }    
    else
    {
      ledController.setBrightness(lightConfig.brightness);
    }
    ledController.setColor(ledController.HexToRGB(lightConfig.color));
    ledController.setDark(!lightConfig.state);

    timeConverter = new TimeConverterDE();

    webui.init(httpRequestCallback, httpResponseCallback, handleFWUpload, isUpdateSuccess);

    if (systemConfig.mqttConfig.enabled)
    {
      enableMqtt();      
    }

    lastHour = wordClock->getHour();
    lastMinute = wordClock->getMinute();
    showCurrentTime(lastHour, lastMinute);
  }
  else
  {
    isSetup = true;
    if (!wifiSetup.enableHostAp(Defaults::PRODUCT, Defaults::DEFAULT_WIFI_SETUP_PASS))
    {
      Serial.println("Failed to start AP");
      Serial.flush();
      abort();
    }
    else
    {
      webui.initHostAP(httpRequestCallback);
    }
  }

  initialized = true;
  ledController.test();
}

void loop()
{
  if (!isSetup && initialized)
  {
    // unsigned long now = millis();
    // if (now - lastUpdate > Defaults::LED_INTERVAL)
    // {
    //   lastUpdate = now;
    //   Serial.printf("Free heap / min heap: %d / %d\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());
    // }

    wordClock->loop();
    ledController.loop();
    if (systemConfig.mqttConfig.enabled && haMqtt != nullptr)
    {
      haMqtt->loop();
    }
  }
}
