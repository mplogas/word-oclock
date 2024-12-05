#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <RTClib.h>
#include <Update.h>
#include <vector>

#include "constants.h"
#include "wifisetup.h"
#include "storage.h"
#include "homeassistant.h"
#include "wclock.h"
#include "webui.h"
#include "leds.h"

boolean isSetup;

RTC_DS3231 rtc;
WiFiClient client;
AsyncWebServer server(80);
WebUI webui(server, PRODUCT, FW_VERSION);
WifiSetup* wifiSetup;
Storage* storage;
WClock* wordClock;
HomeAssistant* homeAssistant;
LED ledController;

bool featureHA = true;
bool initialized = false;
unsigned long lastUpdate = 0;

// this if for (reduced) testing purposes
boolean isTick;
// Define the LED ranges for the two states
std::vector<std::pair<int, int>> tickLEDs = {
    {1, 3},   // From LED index 0, turn on 3 LEDs (0 to 2)
    {6, 3}   // From LED index 6, turn on 3 LEDs (6 to 9)
};

std::vector<std::pair<int, int>> tockLEDs = {
    {3, 3}   // From LED index 3, turn on 5 LEDs (3 to 5)
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
    
    storage->writeFile(StorageType::SSID, ssid.c_str());
    storage->writeFile(StorageType::WIFI_PASS, password.c_str());
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

    if (featureHA) {
      char buf[8];
      itoa(value, buf, 10);
      homeAssistant->setSensorValue(SensorType::LightIntensity, buf);
    }
}

void setup()
{
  isTick = true;
  Serial.begin(115200);
  Serial.printf("Starting with FW %s...\n", FW_VERSION);

  wordClock = new WClock(rtc);
  if (!wordClock->init())
  {
    Serial.println("RTC may lack power or may be missing");
    Serial.flush();
    abort();
  }

  storage = new Storage(LittleFS);
  if (!storage->init())
  {
    Serial.flush();
    abort();
  }

  String ssid = storage->readFile(StorageType::SSID);
  String pass = storage->readFile(StorageType::WIFI_PASS);

  wifiSetup = new WifiSetup();

  if (wifiSetup->connect(ssid.c_str(), pass.c_str(), WIFI_SCAN_TIMEOUT))
  {
    isSetup = false;

    bool clockResult = wordClock->begin();

    if(!clockResult) {
      Serial.println("Failed to initialize clock");
    } else {
      Serial.println("Successfully intitialized clock");
    }


    ledController = LED();
    ledController.init();
    ledController.setColor(CRGB::White);
    ledController.setAutoBrightness(true);
  
    if (featureHA)
    {
      homeAssistant = new HomeAssistant(client, PRODUCT, FW_VERSION);
      homeAssistant->addSensor(SensorType::LightIntensity, "0", "mdi:brightness-5");
      homeAssistant->addSwitch(SwitchType::LED, false, "mdi:lightbulb");
      homeAssistant->setSwitchCommandCallback(handleSwitchCommand);

      ledController.registerIlluminanceSensorCallback(handleIlluminanceSensorUpdate);

      homeAssistant->connect(IPAddress(192, 168, 56, 65), handleMqttConnected, handleMqttDisconnected);
    }

    webui.init(handleUpdateResult, handleFWUpload);
  }
  else
  {
    isSetup = true;
    if (!wifiSetup->enableHostAp(PRODUCT, DEFAULT_WIFI_PASS))
    {
      Serial.println("Failed to start AP");
      Serial.flush();
      abort();
    } else {
        webui.initHostAP(handleWiFiCredentials);
    }
  }

  initialized = true;
}



void loop()
{
  if (!isSetup && initialized)
  {
    unsigned long now = millis();
    if (now - lastUpdate > LED_INTERVAL)
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
        ledController.setColor(CRGB::DarkBlue);
        ledController.setLEDs(tockLEDs);

        Serial.println("Tock");
      }

      FastLED.show();
    }

    ledController.loop();

    if (featureHA)
    {
      homeAssistant->loop();
    }
  }
}
