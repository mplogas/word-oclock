#include "leds.h"

LED::LED() {
    sensorCallback = nullptr;
}

LED::~LED() {
    sensorCallback = nullptr;
}

void LED::init() {
    FastLED.addLeds<WS2812B, DATA_PIN>(this->leds, NUM_LEDS);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

void LED::setBrightness(uint8_t brightness) {
    this->autoBrightness = false;
    this->brightness = brightness;
    FastLED.setBrightness(brightness);
    FastLED.show();
}

void LED::setColor(const CRGB& color, bool show) {
    this->color = color;
    if (show) FastLED.showColor(color);
}

void LED::setAutoBrightness(bool autoBrightness, int illuminanceThresholdHigh, int illuminanceThresholdLow) {
    this->autoBrightness = autoBrightness;
    if(autoBrightness) {
        this->illuminanceThresholdHigh = illuminanceThresholdHigh;
        this->illuminanceThresholdLow = illuminanceThresholdLow;
    } else {
        setBrightness(DEFAULT_BRIGHTNESS);
        this->illuminanceThresholdHigh = ILLUMINANCE_THRESHOLD_HIGH;
        this->illuminanceThresholdLow = ILLUMINANCE_THRESHOLD_LOW;
    }
}

void LED::registerIlluminanceSensorCallback(const IlluminanceSensorCallback &callback) {
    //make sure the callback is not null and not previously set
    if (callback && !sensorCallback) 
        this->sensorCallback = callback;
}

void LED::setLEDs(const std::vector<std::pair<int, int>>& ledRanges) {

    FastLED.clear();

    for (const auto& range : ledRanges) {
        int start = range.first;
        int count = range.second;
        Serial.printf("Setting LEDs from %d to %d\n", start, start + count -1);
        for (int i = start; i < start + count && i < NUM_LEDS; i++) {
            Serial.printf("Setting LED %d\n", i);
            this->leds[i] = color;
        }
    }
    FastLED.show();
}

void LED::clearLEDs() {
    //FastLED.clear(true);
    for( int i = 0; i < NUM_LEDS; i++) {
        this->leds[i] = CRGB::Black;
    }

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
    if (!this->autoBrightness) {
        // Serial.println("Auto brightness is disabled");
        return;
    }

    // Map the illuminance to the brightness range
    int targetBrightness = map(this->illuminance, this->illuminanceThresholdLow, this->illuminanceThresholdHigh, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    targetBrightness = constrain(targetBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    // Serial.printf("Target brightness: %d\n", targetBrightness);
    
    if(targetBrightness == this->brightness) return;

    // Smoothing with Exponential Moving Average
    /*
        Smoothing with Exponential Moving Average:

        The smoothing factor alpha determines how much new readings affect the brightness.
        A smaller alpha results in smoother changes but slower responsiveness.
        An alpha of 0.1 means the new brightness is 10% of the target and 90% of the current brightness.
    */
    const float alpha = 0.1f;  // Smoothing factor (0 < alpha <= 1)
    float currentBrightness = static_cast<float>(this->brightness);
    uint8_t smoothedBrightness = static_cast<uint8_t>(currentBrightness * (1.0f - alpha) + targetBrightness * alpha);
    // Serial.printf("Smoothed brightness: %.2f\n", smoothedBrightness);

    if(smoothedBrightness == this->brightness) return;

    this->brightness = smoothedBrightness;

    FastLED.setBrightness(this->brightness);
    FastLED.show();

    Serial.printf("Updated brightness: %d\n", this->brightness);
}


void LED::loop() {
    unsigned long currentMillis = millis();
    if(currentMillis - this->lastIlluminanceUpdate > ILLUMINANCE_UPDATE_INTERVAL) {
        this->lastIlluminanceUpdate = currentMillis;
        this->illuminance = analogRead(LDR_PIN);
        // Serial.printf("Illuminance: %d\n", this->illuminance);
    }

    if(currentMillis - this->lastBrightnessUpdate > BRIGHTNESS_UPDATE_INTERVAL && this->autoBrightness == true) {
        this->lastBrightnessUpdate = currentMillis;
        handleAutoBrightness();
    }

    if(currentMillis - this->lastSensorUpdate > SENSOR_UPDATE_INTERVAL  && this->sensorCallback) 
    {
        this->lastSensorUpdate = currentMillis;    
        int value = this->illuminance;
        sensorCallback(value);
    }

}