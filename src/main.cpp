#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
// #include <ArduinoHA.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncTCP.h>
#include <LittleFS.h>
// #include <time.h>
#include <RTClib.h>
#include <Update.h>

#include "constants.h"
#include "wifisetup.h"
#include "storage.h"
#include "homeassistant.h"
#include "wclock.h"
#include "webui.h"

CRGB leds[NUM_LEDS];
boolean isTick;
boolean isSetup;

RTC_DS3231 rtc;
WiFiClient client;
AsyncWebServer server(80);
WifiSetup wifiSetup;
Storage storage(LittleFS);
WClock* wordClock;
HomeAssistant* homeAssistant;
WebUI webui(server, PRODUCT, FW_VERSION);


unsigned long lastMillisIlluminance = 0;
unsigned long lastMillisLED = 0;
unsigned long lastMillisWifi = 0;

String ledState;


unsigned long ota_progress_millis = 0;



void ledSwitchCommand(bool state, HASwitch* sender) {
    if (state) {
      Serial.println("LED turned ON");
    } else {
      Serial.println("LED turned OFF");
    }
}



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

bool updateCallback() {
    // Logic to determine if the update was successful
    return !Update.hasError();
}

void handleWiFiCredentials(const String &ssid, const String &password) {
    Serial.printf("SSID set to: %s\n", ssid.c_str());
    Serial.printf("Password set to: %s\n", password.c_str());
    
    storage.writeFile(StorageType::SSID, ssid.c_str());
    storage.writeFile(StorageType::WIFI_PASS, password.c_str());
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

// Callback for LED switch commands
void handleLedSwitchCommand(bool state) {
    if (state) {
        Serial.println("LED turned ON");
    } else {
        Serial.println("LED turned OFF");
    }
}



void setup()
{
  isTick = true;
  Serial.begin(115200);
  Serial.printf("Starting with FW %s...\n", FW_VERSION);

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (!storage.init())
  {
    Serial.flush();
    abort();
  }

  String ssid = storage.readFile(StorageType::SSID);
  String pass = storage.readFile(StorageType::WIFI_PASS);

  if (wifiSetup.connect(ssid.c_str(), pass.c_str(), WIFI_SCAN_TIMEOUT))
  {
    isSetup = false;

    wordClock = new WClock(rtc);
    wordClock->begin();
    wordClock->setTimeZone();

    homeAssistant = new HomeAssistant(client, PRODUCT, FW_VERSION);
    homeAssistant->addSensor(SensorType::LightIntensity, "0", "mdi:brightness-5");
    homeAssistant->addSwitch(SwitchType::LED, false, "mdi:lightbulb");
    homeAssistant->setSwitchCommandCallback([](SwitchType switchType, bool state) {
      if (switchType == SwitchType::LED) {
          if (state) {
              // Turn on LED
              digitalWrite(LED_PIN, HIGH);
          } else {
              // Turn off LED
              digitalWrite(LED_PIN, LOW);
          }
      }
    });

    homeAssistant->connect(IPAddress(192, 168, 1, 1), handleMqttConnected, handleMqttDisconnected);
    webui.init(updateCallback, handleFWUpload);

    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(128);

    unsigned long m = millis();
    lastMillisIlluminance = m;
    lastMillisLED = m;

  }
  else
  {
    isSetup = true;
    if (!wifiSetup.enableHostAp(PRODUCT, DEFAULT_WIFI_PASS))
    {
      Serial.println("Failed to start AP");
      Serial.flush();
      abort();
    } else {
        webui.initHostAP(handleWiFiCredentials);
    }
  }
}

void loop()
{
  if (!isSetup)
  {
    unsigned long currentMillis = millis();
    int illuminance = 0;
    if (currentMillis - lastMillisIlluminance > ILLUMINANCE_INTERVAL)
    {
      lastMillisIlluminance = currentMillis;
      illuminance = analogRead(LDR_PIN);
      //Serial.println(illuminance);

      char buf[8];
      itoa(illuminance, buf, 10);
      homeAssistant->setSensorValue(SensorType::LightIntensity, buf);

      if (illuminance > 2000)
      {
        FastLED.setBrightness(255);
      }
      else if (illuminance > 1000)
      {
        FastLED.setBrightness(128);
      }
      else
      {
        FastLED.setBrightness(64);
      }
      FastLED.show();
    }

    if (currentMillis - lastMillisLED > LED_INTERVAL)
    {
      lastMillisLED = currentMillis;
      if (isTick)
      {
        isTick = false;
        leds[2] = CRGB::Black;
        leds[4] = CRGB::Black;
        leds[6] = CRGB::Black;

        leds[0] = CRGB::Red;
        leds[4] = CRGB::Blue;
        leds[8] = CRGB::White;
      }
      else
      {
        isTick = true;
        leds[0] = CRGB::Black;
        leds[4] = CRGB::Black;
        leds[8] = CRGB::Black;

        leds[2] = CRGB::Red;
        leds[4] = CRGB::Blue;
        leds[6] = CRGB::White;
      }
      FastLED.show();
    }

    homeAssistant->loop();
  }
}
