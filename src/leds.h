// leds.h

#ifndef LEDS_H
#define LEDS_H

#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <functional>
#include "defaults.h"

using IlluminanceSensorCallback = std::function<void(const int value)>;

class LED {
public:
    LED();
    ~LED();
    void init();
    void setBrightness(uint8_t brightness);
    void setColor(const CRGB& color);
    void setLEDs(const std::vector<std::pair<int, int>>& ledRanges);
    void clearLEDs();
    void enableAutoBrightness(int illuminanceThresholdHigh, int illuminanceThresholdLow);
    void disableAutoBrightness();
    void registerIlluminanceSensorCallback(const IlluminanceSensorCallback &callback);
    void unregisterIlluminanceSensorCallback();
    void test();
    void loop();
    static String RGBtoHex(const CRGB& color);
    static CRGB HexToRGB(const String& hex);
private:
    static const int NUM_LEDS = 121; // Number of LEDs (should be 121 (or 47 or 9 for prototyping))
    static const int DATA_PIN = 25; // LED data pin
    static const int LDR_PIN = 35; // Pin for Light-Dependent Resistor
    static const uint8_t BRIGHTNESS_MIN = 10; // min brightness for auto brightness
    static const uint8_t BRIGHTNESS_MAX = 255; // max brightness for auto brightness
    static const unsigned long BRIGHTNESS_UPDATE_INTERVAL = 150; // Brightness update interval
    static const int ILLUMINANCE_UPDATE_INTERVAL = 500; // Illuminance update interval
    static const int SENSOR_UPDATE_INTERVAL = 1000; // Sensor update interval
    static const int TEST_LED_INTERVAL = 85; // Test LED interval
    CRGB leds[NUM_LEDS];
    uint8_t brightness = Defaults::DEFAULT_LIGHT_BRIGHTNESS;
    bool autoBrightness = false;
    bool testMode = false;
    int testLED = 0;
    uint16_t illuminance = 0;
    int illuminanceThresholdHigh = 0;
    int illuminanceThresholdLow = 0;
    CRGB color = CRGB::White;
    uint8_t smoothedBrightness = Defaults::DEFAULT_LIGHT_BRIGHTNESS;
    unsigned long lastBrightnessUpdate  = 0;
    unsigned long lastSensorUpdate  = 0;
    unsigned long lastIlluminanceUpdate  = 0;
    unsigned long lastTestLEDUpdate  = 0;
    IlluminanceSensorCallback sensorCallback;
    void handleAutoBrightness();
};

#endif // LEDS_H