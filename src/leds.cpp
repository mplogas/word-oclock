#include "leds.h"

LED::LED() : brightness(DEFAULT_BRIGHTNESS), color(CRGB::White) {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(this->leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
}

LED::~LED() {
    sensorCallback = nullptr;
}

void LED::setBrightness(uint8_t brightness) {
    //setAutoBrightness(false);
    this->brightness = brightness;
    FastLED.setBrightness(brightness);
    FastLED.show();
}

void LED::setColor(const CRGB& color) {
    this->color = color;
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}

void LED::setAutoBrightness(bool autoBrightness, int illuminanceThresholdHigh, int illuminanceThresholdLow) {
    this->autoBrightness = autoBrightness;
    if(autoBrightness) {
        this->illuminanceThresholdHigh = illuminanceThresholdHigh;
        this->illuminanceThresholdLow = illuminanceThresholdLow;
        handleAutoBrightness();
    } else {
        setBrightness(DEFAULT_BRIGHTNESS);
        this->illuminanceThresholdHigh = ILLUMINANCE_THRESHOLD_HIGH;
        this->illuminanceThresholdLow = ILLUMINANCE_THRESHOLD_LOW;
    }
}

void LED::registerIlluminanceSensorCallback(const IlluminanceSensorCallback &callback) {
    sensorCallback = callback;
}

void LED::setLEDs(const std::vector<std::pair<int, int>>& ledRanges) {

    fill_solid(this->leds, NUM_LEDS, CRGB::Black);

    for (const auto& range : ledRanges) {
        int start = range.first;
        int count = range.second;
        for (int i = start; i < start + count && i < NUM_LEDS; ++i) {
            this->leds[i] = this->color;
        }
    }
    FastLED.show();
}

void LED::clearLEDs() {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

String LED::RGBtoHex(const CRGB& color) {
    char hex[7];
    sprintf(hex, "%02X%02X%02X", color.r, color.g, color.b);
    return String(hex);
}

CRGB LED::HexToRGB(const String& hex) {
    long number = strtol(hex.c_str(), nullptr, 16);
    return CRGB(number >> 16, number >> 8 & 0xFF, number & 0xFF);
}

void LED::handleAutoBrightness() {
    if(!this->autoBrightness) {
        return;
    }

    int illuminance = analogRead(LDR_PIN);
    int targetBrightness = map(illuminance, this->illuminanceThresholdLow, this->illuminanceThresholdHigh, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    targetBrightness = constrain(targetBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);

    /*
        Smoothing with Exponential Moving Average:

        The smoothing factor alpha determines how much new readings affect the brightness.
        A smaller alpha results in smoother changes but slower responsiveness.
        An alpha of 0.1 means the new brightness is 10% of the target and 90% of the current brightness.
    */
    const float alpha = 0.2;  // Smoothing factor (0 < alpha <= 1)
    uint8_t smoothedBrightness = this->brightness * (1 - alpha) + targetBrightness * alpha; // Exponential moving average
    setBrightness(smoothedBrightness);
}

void LED::loop() {
    unsigned long currentMillis = millis();
    if(currentMillis - this->lastBrightnessUpdate > BRIGHTNESS_UPDATE_INTERVAL && this->autoBrightness) {
        this->lastBrightnessUpdate = currentMillis;
        handleAutoBrightness();
    }
    if(currentMillis - this->lastSensorUpdate > ILLUMINANCE_INTERVAL && this->sensorCallback) {
        this->lastSensorUpdate = currentMillis;
        int illuminance = analogRead(LDR_PIN);
        sensorCallback(illuminance);
    }

}