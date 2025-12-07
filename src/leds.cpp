#include "leds.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static portMUX_TYPE ledsMux = portMUX_INITIALIZER_UNLOCKED;

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
    portENTER_CRITICAL(&ledsMux);
    this->autoBrightness = false;
    this->brightness = brightness;
    this->pendingFrame = true;
    this->immediateUpdatePending = true;
    portEXIT_CRITICAL(&ledsMux);
}

void LED::setColor(const CRGB& color) {
    portENTER_CRITICAL(&ledsMux);
    this->color = color;
    this->pendingFrame = true;
    this->immediateUpdatePending = true;
    portEXIT_CRITICAL(&ledsMux);
}

void LED::enableAutoBrightness(int illuminanceThresholdHigh, int illuminanceThresholdLow) {
    portENTER_CRITICAL(&ledsMux);
    this->autoBrightness = true;
    this->illuminanceThresholdHigh = illuminanceThresholdHigh;
    this->illuminanceThresholdLow = illuminanceThresholdLow;
    portEXIT_CRITICAL(&ledsMux);
}

void LED::disableAutoBrightness() {
    portENTER_CRITICAL(&ledsMux);
    this->autoBrightness = false;
    portEXIT_CRITICAL(&ledsMux);
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
    portENTER_CRITICAL(&ledsMux);
    activeLEDs = ledRanges;
    this->pendingFrame = true;
    this->immediateUpdatePending = true;
    portEXIT_CRITICAL(&ledsMux);
}

// another bug: it takes a while until the first time is displayed (max 1 minute), due to the loop. need to address that in main, probably
void LED::setDark(bool dark) {
    portENTER_CRITICAL(&ledsMux);
    this->isDark = dark;
    this->pendingFrame = true;
    this->immediateUpdatePending = true;
    portEXIT_CRITICAL(&ledsMux);
}


// void LED::clearLEDs() {
//     //FastLED.clear(true);
//     for( int i = 0; i < NUM_LEDS; i++) {
//         this->leds[i] = CRGB::Black;
//     }

//     FastLED.show();
// }

void LED::test() {
    if (!this->testMode) {
        this->testMode = true;
        this->testLED = 0;
        this->immediateUpdatePending = true;
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

    int targetBrightness = map(this->illuminance, this->illuminanceThresholdLow, this->illuminanceThresholdHigh, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    targetBrightness = constrain(targetBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);

    uint8_t currentBrightness;
    portENTER_CRITICAL(&ledsMux);
    currentBrightness = this->brightness;
    portEXIT_CRITICAL(&ledsMux);

    if(targetBrightness == currentBrightness) return;

    const float alpha = 0.20f;
    uint8_t smoothedBrightness = static_cast<uint8_t>(currentBrightness * (1.0f - alpha) + targetBrightness * alpha);

    if(smoothedBrightness == currentBrightness) return;

    portENTER_CRITICAL(&ledsMux);
    this->brightness = smoothedBrightness;
    this->pendingFrame = true;
    this->brightnessUpdatePending = true;
    portEXIT_CRITICAL(&ledsMux);
}


bool LED::canRunFastRefresh(unsigned long now) const {
    return (now - lastFastRefresh) >= FAST_REFRESH_INTERVAL_MS;
}

bool LED::takeImmediateUpdate() {
    bool shouldRun = false;
    portENTER_CRITICAL(&ledsMux);
    if (immediateUpdatePending) {
        immediateUpdatePending = false;
        shouldRun = true;
    }
    portEXIT_CRITICAL(&ledsMux);
    return shouldRun;
}

bool LED::takeTestUpdate(unsigned long now) {
    if (!testMode) {
        return false;
    }

    if (now - lastTestLEDUpdate < TEST_LED_INTERVAL || this->testLED >= NUM_LEDS) {
        return false;
    }

    lastTestLEDUpdate = now;
    this->leds[this->testLED] = CRGB::Black;

    if(this->testLED == NUM_LEDS - 1) {
        this->testMode = false;
        this->testLED = 0;
    } else {
        this->testLED++;
        this->leds[this->testLED] = CRGB::White;
    }

    return true;
}

bool LED::takeBrightnessUpdate(unsigned long now) {
    bool shouldCheck = false;
    portENTER_CRITICAL(&ledsMux);
    shouldCheck = autoBrightness;
    portEXIT_CRITICAL(&ledsMux);

    if (!shouldCheck) {
        return false;
    }

    if (now - lastBrightnessUpdate < BRIGHTNESS_UPDATE_INTERVAL) {
        return false;
    }

    lastBrightnessUpdate = now;
    handleAutoBrightness();

    bool runRender = false;
    portENTER_CRITICAL(&ledsMux);
    if (brightnessUpdatePending) {
        brightnessUpdatePending = false;
        runRender = true;
    }
    portEXIT_CRITICAL(&ledsMux);
    return runRender;
}

void LED::scheduleRegularTick(unsigned long now) {
    if (now - lastRegularTick >= 1000) {
        lastRegularTick = now;
        regularUpdatePending = true;
    }
}

bool LED::takeRegularUpdate(unsigned long now) {
    scheduleRegularTick(now);
    bool shouldRun = false;
    if (!(now - lastLEDUpdate > LED_UPDATE_INTERVAL)) {
        return false;
    }

    portENTER_CRITICAL(&ledsMux);
    if (regularUpdatePending || pendingFrame) {
        regularUpdatePending = false;
        pendingFrame = false;
        shouldRun = true;
    }
    portEXIT_CRITICAL(&ledsMux);
    return shouldRun;
}

void LED::performRender(const std::vector<std::pair<int, int>>& ledSnapshot,
                        const CRGB& colorSnapshot,
                        bool darkSnapshot,
                        uint8_t brightnessSnapshot) {
    const uint32_t renderStart = micros();

    for(int i = 0; i < NUM_LEDS; i++) {
        this->leds[i] = CRGB::Black;
    }

    if(!darkSnapshot && !ledSnapshot.empty()) {
        for(auto ledRange : ledSnapshot) {
            int start = ledRange.first;
            int count = ledRange.second;                  
            for(int i = 0; i < count && (start + i) < NUM_LEDS; i++) {
                this->leds[start + i] = colorSnapshot;
            }
        }
    }

    FastLED.setBrightness(brightnessSnapshot);
    renderInProgress = true;
    FastLED.show();
    renderInProgress = false;

    lastRenderDurationUs = micros() - renderStart;
    if (lastRenderDurationUs > maxRenderDurationUs) {
        maxRenderDurationUs = lastRenderDurationUs;
    }
    logRenderDuration(lastRenderDurationUs);
}

void LED::logRenderDuration(uint32_t durationUs) {
    if (durationUs > FASTLED_FATAL_US) {
        Serial.printf("[LED] Render took %uus (max %uus)\n", durationUs, maxRenderDurationUs);
    } else if (durationUs > FASTLED_WARN_US) {
        Serial.printf("[LED] Render slow: %uus\n", durationUs);
    }
}

void LED::loop() {
    const unsigned long now = millis();

    if (!canRunFastRefresh(now)) {
        return;
    }

    lastFastRefresh = now;

    if(now - this->lastIlluminanceUpdate > ILLUMINANCE_UPDATE_INTERVAL) {
        this->lastIlluminanceUpdate = now;
        this->illuminance = analogRead(LDR_PIN);
    }

    if(now - this->lastSensorUpdate > SENSOR_UPDATE_INTERVAL  && this->sensorCallback) 
    {
        this->lastSensorUpdate = now;    
        int value = this->illuminance;
        sensorCallback(value);
    }

    if (renderInProgress) {
        return;
    }

    bool shouldRender = false;
    bool testRender = false;

    if (!shouldRender) {
        shouldRender = takeImmediateUpdate();
    }

    if (!shouldRender) {
        testRender = takeTestUpdate(now);
        shouldRender = testRender;
    }

    if (!shouldRender) {
        shouldRender = takeBrightnessUpdate(now);
    }

    if (!shouldRender) {
        shouldRender = takeRegularUpdate(now);
    }

    if (!shouldRender) {
        return;
    }

    this->lastLEDUpdate = now;

    if (testRender) {
        uint8_t brightnessSnapshot;
        portENTER_CRITICAL(&ledsMux);
        brightnessSnapshot = this->brightness;
        portEXIT_CRITICAL(&ledsMux);

        const uint32_t renderStart = micros();
        FastLED.setBrightness(brightnessSnapshot);
        renderInProgress = true;
        FastLED.show();
        renderInProgress = false;
        lastRenderDurationUs = micros() - renderStart;
        if (lastRenderDurationUs > maxRenderDurationUs) {
            maxRenderDurationUs = lastRenderDurationUs;
        }
        logRenderDuration(lastRenderDurationUs);
        return;
    }

    std::vector<std::pair<int, int>> ledSnapshot;
    CRGB colorSnapshot;
    bool darkSnapshot;
    uint8_t brightnessSnapshot;

    portENTER_CRITICAL(&ledsMux);
    ledSnapshot = activeLEDs;
    colorSnapshot = this->color;
    darkSnapshot = this->isDark;
    brightnessSnapshot = this->brightness;
    pendingFrame = false;
    portEXIT_CRITICAL(&ledsMux);

    performRender(ledSnapshot, colorSnapshot, darkSnapshot, brightnessSnapshot);
}