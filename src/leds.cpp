#include "leds.h"

LED::LED() : brightness(DEFAULT_BRIGHTNESS), color(CRGB::White) {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
}

LED::~LED() {
    sensorCallback = nullptr;
}

void LED::setBrightness(uint8_t brightness) {
    this->brightness = brightness;
    FastLED.setBrightness(brightness);
    FastLED.show();
}

void LED::setColor(const CRGB& color) {
    this->color = color;
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}

void LED::toggleLEDs(const std::vector<std::pair<int, int>>& ledRanges) {
    for (const auto& range : ledRanges) {
        int start = range.first;
        int count = range.second;
        for (int i = start; i < start + count && i < NUM_LEDS; ++i) {
            leds[i] = leds[i] ? CRGB::Black : color;
        }
    }
    FastLED.show();
}

void LED::handleAutoBrightness(int illuminanceThresholdHigh, int illuminanceThresholdLow) {
    if(!autoBrightness) {
        return;
    }

    int illuminance = analogRead(LDR_PIN);

    int targetBrightness = map(illuminance, illuminanceThresholdLow, illuminanceThresholdHigh, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    targetBrightness = constrain(targetBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);

    /*
        Smoothing with Exponential Moving Average:

        The smoothing factor alpha determines how much new readings affect the brightness.
        A smaller alpha results in smoother changes but slower responsiveness.
        An alpha of 0.1 means the new brightness is 10% of the target and 90% of the current brightness.
    */
    const float alpha = 0.1;  // Smoothing factor (0 < alpha <= 1)
    uint8_t smoothedBrightness = brightness * (1 - alpha) + targetBrightness * alpha; // Exponential moving average
    setBrightness(smoothedBrightness);
}

void LED::loop() {

    unsigned long currentMillis = millis();
    if(currentMillis - lastBrightnessUpdate > BRIGHTNESS_UPDATE_INTERVAL && autoBrightness) {
        lastBrightnessUpdate = currentMillis;
        handleAutoBrightness();
    }
    if(currentMillis - lastSensorUpdate > ILLUMINANCE_INTERVAL && sensorCallback) {
        lastSensorUpdate = currentMillis;
        int illuminance = analogRead(LDR_PIN);
        sensorCallback(illuminance);
    }
}