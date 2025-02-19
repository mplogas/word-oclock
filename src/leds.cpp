#include "leds.h"

LED::LED() {
    sensorCallback = nullptr;
}

LED::~LED() {
    sensorCallback = nullptr;
}

void LED::init() {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(this->leds, NUM_LEDS);
    FastLED.setBrightness(Defaults::DEFAULT_LIGHT_BRIGHTNESS);
}

void LED::setBrightness(uint8_t brightness) {
    this->autoBrightness = false;
    this->brightness = brightness;
    //FastLED.setBrightness(brightness);
}

void LED::setColor(const CRGB& color) {
    this->color = color;
}

void LED::enableAutoBrightness(int illuminanceThresholdHigh, int illuminanceThresholdLow) {
    this->autoBrightness = true;
    this->illuminanceThresholdHigh = illuminanceThresholdHigh;
    this->illuminanceThresholdLow = illuminanceThresholdLow;
}

void LED::disableAutoBrightness() {
    this->autoBrightness = false;
}

void LED::registerIlluminanceSensorCallback(const IlluminanceSensorCallback &callback) {
    //make sure the callback is not null and not previously set
    if (callback && !sensorCallback) 
        this->sensorCallback = callback;
}

void LED::unregisterIlluminanceSensorCallback() {
    this->sensorCallback = nullptr;
}

void LED::setLEDs(const std::vector<std::pair<int, int>>& ledRanges) {
    if(ledRanges.empty() || this->testMode) return;
    activeLEDs = ledRanges;
}

// TODO / BUG: we need an isdark bool here to avoid that the loop displays the time again after the light was turned off 
// another bug: it takes a while until the first time is displayed (max 1 minute), due to the loop. need to address that in main, probably
void LED::clearLEDs() {
    //FastLED.clear(true);
    for( int i = 0; i < NUM_LEDS; i++) {
        this->leds[i] = CRGB::Black;
    }

    FastLED.show();
}

void LED::test() {
    if (!this->testMode) {
        this->testMode = true;
        this->testLED = 0;
    }
}

String LED::RGBtoHex(const CRGB& color) {
    char hex[8]; // '#RRGGBB\0'
    sprintf(hex, "#%02X%02X%02X", color.r, color.g, color.b);
    return String(hex);
}

CRGB LED::HexToRGB(const String& hex) {
    // Ensure the hex string starts with '#'
    long number;
    if (hex[0] == '#') {
        // Convert the hex string to a long integer, skipping the '#'
        number = strtol(hex.substring(1).c_str(), nullptr, 16);
    } else {
        number = strtol(hex.c_str(), nullptr, 16);        
    }
    return CRGB(number >> 16, (number >> 8) & 0xFF, number & 0xFF);
}

void LED::handleAutoBrightness() {
    if (!this->autoBrightness) {
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
    const float alpha = 0.20f;  // Smoothing factor (0 < alpha <= 1)
    uint8_t smoothedBrightness = static_cast<uint8_t>(this->brightness * (1.0f - alpha) + targetBrightness * alpha);
    // Serial.printf("Smoothed brightness: %.2d\n", smoothedBrightness);

    if(smoothedBrightness == this->brightness) return;

    this->brightness = smoothedBrightness;

    FastLED.setBrightness(this->brightness);
    FastLED.show();

    //Serial.printf("Updated brightness: %d\n", this->brightness);
}


void LED::loop() {
    unsigned long currentMillis = millis();

    if(this->testMode && currentMillis - this->lastTestLEDUpdate > TEST_LED_INTERVAL && this->testLED < NUM_LEDS) {
        this->lastTestLEDUpdate = currentMillis;
        this->leds[this->testLED] = CRGB::Black;

        if(this->testLED == NUM_LEDS - 1) {
            this->testMode = false;
            this->testLED = 0;
        } else {
            this->testLED++;
            this->leds[this->testLED] = CRGB::White;
        }

        FastLED.show();
    } else {
        if(currentMillis - this->lastIlluminanceUpdate > ILLUMINANCE_UPDATE_INTERVAL) {
            this->lastIlluminanceUpdate = currentMillis;
            this->illuminance = analogRead(LDR_PIN);
            //Serial.printf("Illuminance: %d\n", this->illuminance);
        }

        if(currentMillis - this->lastBrightnessUpdate > BRIGHTNESS_UPDATE_INTERVAL && this->autoBrightness == true) {
            this->lastBrightnessUpdate = currentMillis;
            handleAutoBrightness();
        }

        // Update LEDs from the stored activeLEDs with color and brightness (if auto brightness is disabled)
        if(currentMillis - this->lastLEDUpdate > LED_UPDATE_INTERVAL && !this->testMode) {
            this->lastLEDUpdate = currentMillis;
            for( int i = 0; i < NUM_LEDS; i++) {
                this->leds[i] = CRGB::Black;
            }

            if(activeLEDs.empty()) return;

            for(auto ledRange : activeLEDs) {
                int start = ledRange.first;
                int count = ledRange.second;                  
                for(int i = 0; i < count && (start + i) < NUM_LEDS; i++) {
                    this->leds[start + i] = this->color;
                }
            }

            if(!this->autoBrightness) {
                FastLED.setBrightness(this->brightness);
            }
            FastLED.show();
        }

        if(currentMillis - this->lastSensorUpdate > SENSOR_UPDATE_INTERVAL  && this->sensorCallback) 
        {
            this->lastSensorUpdate = currentMillis;    
            int value = this->illuminance;
            sensorCallback(value);
        }
    }

}