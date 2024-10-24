#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <time.h>
#include <RTClib.h>
#include <Update.h>

#include "constants.h"
#include "homeassistant.h"
#include "storage.h"

CRGB leds[NUM_LEDS];
boolean isTick;
boolean isSetup;

WiFiClient client;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Variables to save values from HTML form
String ssid;
String pass;


unsigned long lastMillisIlluminance = 0;
unsigned long lastMillisLED = 0;
unsigned long lastMillisWifi = 0;

String ledState;

RTC_DS3231 rtc;

unsigned long ota_progress_millis = 0;




// Initialize WiFi
bool initWiFi()
{
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  lastMillisWifi = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - lastMillisWifi > WIFI_SCAN_TIMEOUT)
    {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

void ledSwitchCommand(bool state, HASwitch* sender) {
    if (state) {
      Serial.println("LED turned ON");
    } else {
      Serial.println("LED turned OFF");
    }
}

bool getNTPtime()
{

  unsigned long lastMillis = millis();
  configTime(0, 0, NTP_SERVER);

  Serial.println("Waiting for NTP time sync: ");
  while (millis() - lastMillis < NTP_TIMEOUT)
  {
    time(&now);
    localtime_r(&now, &timeinfo);
    Serial.print(".");
    delay(100);

    if (getLocalTime(&timeinfo))
    {
      Serial.println(" Successfully obtained time");
      return true;
    }
  }

  Serial.println("Failed to get time from NTP");
  return false;

  // {
  //   uint32_t start = millis();
  //   do {
  //     time(&now);
  //     localtime_r(&now, &timeinfo);
  //     Serial.print(".");
  //     delay(10);
  //   } while (((millis() - start) < timeout));

  //   Serial.print("now ");  Serial.println(now);
  //   char time_output[30];
  //   strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
  //   Serial.println(time_output);
  //   Serial.println();
  // }
  // return true;
}

void setTimeZone(const char *timezone)
{
  setenv("TZ", timezone, 1);
  tzset();
}

String wifiSetupProcessor(const String &var)
{
  if (var == "PAGE_TITLE") {
    return PRODUCT;
  }

  return String();
}

String fwUpdateProcessor(const String &var) 
{
  if (var == "PAGE_TITLE") {
    return PRODUCT;
  } else if (var == "FW_VERSION") {
    return FW_VERSION;
  }

  return String();
}

String configurationProcessor(const String &var)
{
  if (var == "STATE")
  {
    if (ledState == "OFF")
    {
      ledState = "ON";
    }
    else
    {
      ledState = "OFF";
    }
    return ledState;
  } else if (var == "FW_VERSION") {
    return FW_VERSION;
  } else if (var == "PAGE_TITLE") {
    return PRODUCT;
  }
  return String();
}

void handleFWUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
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
void notFoundResponse(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void initWebserver()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html", false, configurationProcessor); });
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/firmware.html", "text/html", false, fwUpdateProcessor); });
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html", false, configurationProcessor); });
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html", false, configurationProcessor); });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              Serial.println("Update requested");
              if (!Update.hasError()) {
                Serial.println("Update successful");
                  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
                  response->addHeader("Connection", "close");
                  request->send(response);
                  ESP.restart();
              } else {
                Serial.println("Update failed");
                  AsyncWebServerResponse *response = request->beginResponse(500, "text/plain", "ERROR");
                  response->addHeader("Connection", "close");
                  request->send(response);
              } }, handleFWUpload);
  server.onNotFound(notFoundResponse);

  server.serveStatic("/", LittleFS, "/");
  server.begin();
}


void initHostAP()
{
  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)");
  // NULL sets an open Access Point
  WiFi.softAP(PRODUCT, NULL);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/wifimanager.html", "text/html", false, wifiSetupProcessor); });

  server.serveStatic("/", LittleFS, "/");

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    int params = request->params();
    for(int i=0;i<params;i++){
      const AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == SSID_INPUT) {
          ssid = p->value().c_str();
          Serial.print("SSID set to: ");
          Serial.println(ssid);
          // Write file to save value
          writeFile(LittleFS, SSID_PATH, ssid.c_str());
        }
        // HTTP POST pass value
        if (p->name() == WIFI_PASS_INPUT) {
          pass = p->value().c_str();
          Serial.print("Password set to: ");
          Serial.println(pass);
          // Write file to save value
          writeFile(LittleFS, WIFI_PASS_PATH, pass.c_str());
        }
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to the new address");
    delay(3000);
    ESP.restart(); });
  server.begin();
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


  if (initWiFi())
  {
    isSetup = false;

    setTimeZone(TZ_INFO);
    configTime(0, 0, NTP_SERVER);
    if (getNTPtime())
    {
      lastNTPtime = now;
      rtc.adjust(DateTime(now));
      Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S %Z");
    }

    initWebserver();
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(128);

    unsigned long m = millis();
    lastMillisIlluminance = m;
    lastMillisLED = m;
  }
  else
  {
    isSetup = true;
    initHostAP();
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
      sensor.setValue(buf);

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

    mqtt.loop();
  }
}
