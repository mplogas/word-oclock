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
    portEXIT_CRITICAL(&ledsMux);
}

void LED::setColor(const CRGB& color) {
    portENTER_CRITICAL(&ledsMux);
    this->color = color;
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
    portEXIT_CRITICAL(&ledsMux);
}

// another bug: it takes a while until the first time is displayed (max 1 minute), due to the loop. need to address that in main, probably
void LED::setDark(bool dark) {
    portENTER_CRITICAL(&ledsMux);
    this->isDark = dark;
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
    portEXIT_CRITICAL(&ledsMux);
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

        bool autoMode = false;
        bool pendingSnapshot = false;
        portENTER_CRITICAL(&ledsMux);
        autoMode = this->autoBrightness;
        pendingSnapshot = this->pendingFrame;
        portEXIT_CRITICAL(&ledsMux);

        if(currentMillis - this->lastBrightnessUpdate > BRIGHTNESS_UPDATE_INTERVAL && autoMode) {
            this->lastBrightnessUpdate = currentMillis;
            handleAutoBrightness();
        }

        const bool ledIntervalElapsed = (currentMillis - this->lastLEDUpdate) > LED_UPDATE_INTERVAL;

        if((pendingSnapshot || ledIntervalElapsed) && !this->testMode) {
            this->lastLEDUpdate = currentMillis;
            std::vector<std::pair<int, int>> ledSnapshot;
            CRGB colorSnapshot;
            bool darkSnapshot;
            uint8_t brightnessSnapshot;

            portENTER_CRITICAL(&ledsMux);
            ledSnapshot = activeLEDs;
            colorSnapshot = this->color;
            darkSnapshot = this->isDark;
            brightnessSnapshot = this->brightness;
            this->pendingFrame = false;
            portEXIT_CRITICAL(&ledsMux);

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