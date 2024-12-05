// leds.h

#ifndef LEDS_H
#define LEDS_H

#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <functional>

using IlluminanceSensorCallback = std::function<void(const int value)>;

class LED {
public:
    static constexpr int ILLUMINANCE_THRESHOLD_HIGH = 10000; // High illuminance threshold
    static constexpr int ILLUMINANCE_THRESHOLD_LOW = 300; // Low illuminance threshold
    LED();
    ~LED();
    void init();
    void setBrightness(uint8_t brightness);
    void setColor(const CRGB& color);
    void setLEDs(const std::vector<std::pair<int, int>>& ledRanges);
    void clearLEDs();
    void setAutoBrightness(bool autoBrightness, int illuminanceThresholdHigh =  ILLUMINANCE_THRESHOLD_HIGH, int illuminanceThresholdLow = ILLUMINANCE_THRESHOLD_LOW);
    void registerIlluminanceSensorCallback(const IlluminanceSensorCallback &callback);
    void loop();
    static String RGBtoHex(const CRGB& color);
    static CRGB HexToRGB(const String& hex);
private:
    static const int NUM_LEDS = 9; // Number of LEDs (should be 121 (or 47 or 9 for prototyping))
    static const int DATA_PIN = 25; // LED data pin
    static const int LDR_PIN = 35; // Pin for Light-Dependent Resistor
    static const int DEFAULT_BRIGHTNESS = 50; // Default brightness
    static const uint8_t BRIGHTNESS_MIN = 20; // start brightness for auto brightness
    static const uint8_t BRIGHTNESS_MAX = 255; // max brightness for auto brightness
    static const unsigned long BRIGHTNESS_UPDATE_INTERVAL = 250; // Brightness update interval
    static const int ILLUMINANCE_UPDATE_INTERVAL = 500; // Illuminance update interval
    static const int SENSOR_UPDATE_INTERVAL = 1000; // Sensor update interval
    CRGB leds[NUM_LEDS];
    uint8_t brightness = DEFAULT_BRIGHTNESS;
    bool autoBrightness = false;
    uint16_t illuminance = 0;
    int illuminanceThresholdHigh = ILLUMINANCE_THRESHOLD_HIGH;
    int illuminanceThresholdLow = ILLUMINANCE_THRESHOLD_LOW;
    CRGB color = CRGB::White;
    uint8_t smoothedBrightness  = DEFAULT_BRIGHTNESS;
    unsigned long lastBrightnessUpdate  = 0;
    unsigned long lastSensorUpdate  = 0;
    unsigned long lastIlluminanceUpdate  = 0;
    IlluminanceSensorCallback sensorCallback;
    void handleAutoBrightness();
};

#endif // LEDS_H