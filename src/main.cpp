#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
#include <time.h>
#include "RTClib.h"
#include <Update.h>

#define LED_PIN 25
#define LDR_PIN 36

#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define BRIGHTNESS 128
#define NUM_LEDS 9

#define WL_MAC_ADDR_LENGTH 6

#define BROKER_ADDR IPAddress(192, 168, 56, 56)
#define BROKER_USERNAME "user" // replace with your credentials
#define BROKER_PASSWORD "pass"

#define ILLUMINANCE_INTERVAL 200
#define LED_INTERVAL 15000
#define WIFI_SCAN_TIMEOUT 10000
#define NTP_TIMEOUT 10000

CRGB leds[NUM_LEDS];
boolean isTick;
boolean isSetup;

WiFiClient client;

HADevice device;
HAMqtt mqtt(client, device);
HASensor sensor("wc-lightintensity");

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Search for parameter in HTTP POST request
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
// Variables to save values from HTML form
String ssid;
String pass;

// File paths to save input values permanently
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";

unsigned long lastMillisIlluminance = 0;
unsigned long lastMillisLED = 0;
unsigned long lastMillisWifi = 0;

String ledState;

RTC_DS3231 rtc;
const char *NTP_SERVER = "pool.ntp.org";
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3"; // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for UI
struct tm timeinfo;
time_t now;
long unsigned lastNTPtime = 0;

unsigned long ota_progress_millis = 0;

void onOTAStart()
{
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final)
{
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000)
  {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success)
{
  // Log when OTA has finished
  if (success)
  {
    Serial.println("OTA update finished successfully!");
    ESP.restart();
  }
  else
  {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

void initLittleFS()
{
  if (!LittleFS.begin(true))
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
  }
}

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

void initMqtt()
{
  // Unique ID must be set!
  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));

  device.setName("NodeMCU");
  device.setSoftwareVersion("1.0.0");
  device.setModel("ESP32");
  device.setManufacturer("Espressif");
  // configure sensor (optional)
  sensor.setIcon("mdi:home");
  sensor.setName("Light Intensity");

  mqtt.begin(BROKER_ADDR);
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

// Replaces placeholder with LED state value
String processor(const String &var)
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
  }
  return String();
}

void handleFWUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    { // start with max available size
      Update.printError(Serial);
    }
  }
  if (Update.write(data, len) != len)
  {
    Update.printError(Serial);
  }
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
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html", false, processor); });
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/firmware.html", "text/html"); });
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html", false, processor); });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html", false, processor); });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request)
            {
      if (!Update.hasError()) {
          AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
          response->addHeader("Connection", "close");
          request->send(response);
          ESP.restart();
      } else {
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
  WiFi.softAP("ESP-WIFI-MANAGER", NULL);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/wifimanager.html", "text/html"); });

  server.serveStatic("/", LittleFS, "/");

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    int params = request->params();
    for(int i=0;i<params;i++){
      const AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == PARAM_INPUT_1) {
          ssid = p->value().c_str();
          Serial.print("SSID set to: ");
          Serial.println(ssid);
          // Write file to save value
          writeFile(LittleFS, ssidPath, ssid.c_str());
        }
        // HTTP POST pass value
        if (p->name() == PARAM_INPUT_2) {
          pass = p->value().c_str();
          Serial.print("Password set to: ");
          Serial.println(pass);
          // Write file to save value
          writeFile(LittleFS, passPath, pass.c_str());
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
  Serial.println("Starting...");

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  initLittleFS();

  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  Serial.println(ssid);
  Serial.println(pass);

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

    // ElegantOTA.begin(&server);    // Start ElegantOTA
    // // ElegantOTA callbacks
    // ElegantOTA.onStart(onOTAStart);
    // ElegantOTA.onProgress(onOTAProgress);
    // ElegantOTA.onEnd(onOTAEnd);

    initWebserver();
    initMqtt();
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

    // ElegantOTA.loop();
    mqtt.loop();
  }
}
