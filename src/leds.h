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
    void setDark(bool dark = true);
    //void clearLEDs();
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
    static const int SENSOR_UPDATE_INTERVAL = 1000; // Sensor callback update interval
    static const int TEST_LED_INTERVAL = 85; // Test LED interval
    static const int LED_UPDATE_INTERVAL = Defaults::LED_INTERVAL; // LED update interval
    static const int FAST_REFRESH_INTERVAL_MS = 100; // ~100 Hz ceiling
    static const uint32_t FASTLED_WARN_US = 6000;
    static const uint32_t FASTLED_FATAL_US = 8000;
    CRGB leds[NUM_LEDS];
    uint8_t brightness = Defaults::DEFAULT_LIGHT_BRIGHTNESS;
    bool autoBrightness = false;
    bool testMode = false;
    bool isDark = false;
    bool pendingFrame = false;
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
    unsigned long lastLEDUpdate  = 0;
    unsigned long lastFastRefresh = 0;
    unsigned long lastRegularTick = 0;
    bool immediateUpdatePending = false;
    bool brightnessUpdatePending = false;
    bool regularUpdatePending = false;
    bool renderInProgress = false;
    uint32_t lastRenderDurationUs = 0;
    uint32_t maxRenderDurationUs = 0;
    IlluminanceSensorCallback sensorCallback;
    std::vector<std::pair<int, int>> activeLEDs;
    void handleAutoBrightness();
    bool canRunFastRefresh(unsigned long now) const;
    bool takeImmediateUpdate();
    bool takeTestUpdate(unsigned long now);
    bool takeBrightnessUpdate(unsigned long now);
    bool takeRegularUpdate(unsigned long now);
    void performRender(const std::vector<std::pair<int, int>>& ledSnapshot,
                       const CRGB& colorSnapshot,
                       bool darkSnapshot,
                       uint8_t brightnessSnapshot);
    void scheduleRegularTick(unsigned long now);
    void logRenderDuration(uint32_t durationUs);
};

#endif // LEDS_H