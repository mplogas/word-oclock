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
    LED();
    ~LED();
    void setBrightness(uint8_t brightness);
    void setColor(const CRGB& color);
    void toggleLEDs(const std::vector<std::pair<int, int>>& ledRanges);
    void setAutoBrightness(bool autoBrightness);
    void setAutoBrightness(bool autoBrightness, int illuminanceThresholdHigh, int illuminanceThresholdLow);
    void registerIlluminanceSensorCallback(const IlluminanceSensorCallback &callback);
    void loop();
private:
    static const int NUM_LEDS = 9; // Number of LEDs (should be 121)
    static const int DATA_PIN = 25; // LED data pin
    static const int LDR_PIN = 36; // Pin for Light-Dependent Resistor
    static const int DEFAULT_BRIGHTNESS = 128; // Default brightness
    static const uint8_t BRIGHTNESS_MIN = 50;
    static const uint8_t BRIGHTNESS_MAX = 255;
    static const unsigned long BRIGHTNESS_UPDATE_INTERVAL = 500; // Brightness update interval
    static const int ILLUMINANCE_THRESHOLD_HIGH = 10000; // High illuminance threshold
    static const int ILLUMINANCE_THRESHOLD_LOW = 300; // Low illuminance threshold
    static const int ILLUMINANCE_INTERVAL = 500; // Illuminance update interval
    CRGB leds[NUM_LEDS];
    uint8_t brightness;
    bool autoBrightness;
    CRGB color;
    uint8_t smoothedBrightness;
    unsigned long lastBrightnessUpdate;
    unsigned long lastSensorUpdate;
    IlluminanceSensorCallback sensorCallback;
    void handleAutoBrightness(int illuminanceThresholdHigh = ILLUMINANCE_THRESHOLD_HIGH, int illuminanceThresholdLow = ILLUMINANCE_THRESHOLD_LOW);
};

#endif // LEDS_H