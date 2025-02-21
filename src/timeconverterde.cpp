#include "timeconverterde.h"

TimeConverterDE::TimeConverterDE() {
    // Constructor implementation (if needed)
}

TimeConverterDE::~TimeConverterDE() {
    // Destructor implementation (if needed)
}

// isRegionalFormat toggles between "viertel nach" / "viertel vor" and "viertel" / "dreiviertel"
// optionAsMinutes toggles between showing the minutes as LEDs or not (so they can be used for homeassistant status notifications)
std::vector<std::pair<int, int>> TimeConverterDE::convertTime(const uint8_t hours, const uint8_t minutes, bool isRegionalFormat, bool optionAsMinutes) {
    std::vector<std::pair<int, int>> ledArray;

    // TODO: 20 vor eins funktioniert nicht

    // Normalize hours
    uint8_t normalizedHours = (hours % 12 == 0) ? 12 : (hours % 12);

    // Round minutes down to nearest 5
    uint8_t roundedMinutes = (minutes / 5) * 5;

    uint8_t displayHour = (minutes >= 25) ? normalizedHours + 1 : normalizedHours;
    if(displayHour == 13) {
        displayHour = 1;
    }

    // Add "es ist"
    ledArray.push_back(LEDS_ES); // "es"
    ledArray.push_back(LEDS_IST); // "ist"

    // Determine the appropriate phrase
    switch (roundedMinutes) {
        case 0:
            // "es ist [hour] uhr"
            ledArray.push_back(getHourLEDs(displayHour)); // Hour word
            ledArray.push_back(LEDS_UHR); // "uhr"
            break;
        case 15:
            // "es ist viertel [next hour]"
            if(isRegionalFormat) {
                ledArray.push_back(LEDS_VIERTEL); // "viertel"
                ledArray.push_back(getHourLEDs(normalizedHours + 1)); // Next hour
            } else {
                ledArray.push_back(LEDS_VIERTEL); // "viertel"
                ledArray.push_back(LEDS_NACH); // "nach"
                ledArray.push_back(getHourLEDs(normalizedHours)); // Current hour
             }            
            break;
        case 30:
            // "es ist halb [next hour]"
            ledArray.push_back(LEDS_HALB); // "halb"
            ledArray.push_back(getHourLEDs(displayHour)); // Next hour
            break;
        case 45:
            // "es ist dreiviertel [next hour]"
            if(isRegionalFormat) {
                ledArray.push_back(LEDS_DREIVIERTEL); // "dreiviertel"
            } else {
                ledArray.push_back(LEDS_VIERTEL); // "viertel"
                ledArray.push_back(LEDS_VOR); // "vor"
            }
            ledArray.push_back(getHourLEDs(displayHour)); // Next hour
            break;
        default:
            if (roundedMinutes <= 20) {
                // "es ist [minutes] nach [hour]"
                ledArray.push_back(getMinuteLEDs(roundedMinutes)); // Minutes word
                ledArray.push_back(LEDS_NACH); // "nach"
                ledArray.push_back(getHourLEDs(normalizedHours)); // Current hour
            } else if(roundedMinutes == 25) {
                // "es ist fuenf vor halb"
                ledArray.push_back(getMinuteLEDs(30 - roundedMinutes)); // Minutes word
                ledArray.push_back(LEDS_VOR); // "vor"
                ledArray.push_back(LEDS_HALB); // "halb"
            } else if(roundedMinutes == 35) {
                // "es ist fuenf nach halb"
                ledArray.push_back(getMinuteLEDs(roundedMinutes - 30)); // Minutes word
                ledArray.push_back(LEDS_NACH); // "nach"
                ledArray.push_back(LEDS_HALB); // "halb"
            } else {
                // "es ist [minutes] vor [next hour]"
                ledArray.push_back(getMinuteLEDs(60 - roundedMinutes)); // Minutes word
                ledArray.push_back(LEDS_VOR); // "vor"
            }
            ledArray.push_back(getHourLEDs(displayHour)); // Next hour
            break;
    }

    // Add the option LEDs
    if (optionAsMinutes) {
        //get the minutes from rounded down minutes to now
        uint8_t minutesToDisplay = minutes - roundedMinutes;
        switch (minutesToDisplay) {
            case 1:
                ledArray.push_back(LEDS_OPTION_1);
                break;
            case 2:
                ledArray.push_back(LEDS_OPTION_1);
                ledArray.push_back(LEDS_OPTION_2);
                break;
            case 3:
                ledArray.push_back(LEDS_OPTION_1);
                ledArray.push_back(LEDS_OPTION_2);
                ledArray.push_back(LEDS_OPTION_3);
                break;
            case 4:
                ledArray.push_back(LEDS_OPTION_1);
                ledArray.push_back(LEDS_OPTION_2);
                ledArray.push_back(LEDS_OPTION_3);
                ledArray.push_back(LEDS_OPTION_4);
                break;
            default:
                break;
        }
    } 

    return ledArray;
}

std::pair<int, int> TimeConverterDE::getHourLEDs(uint8_t hour) {
    switch (hour) {
        case 1: return LEDS_H_EINS;
        case 2: return LEDS_H_ZWEI;
        case 3: return LEDS_H_DREI;
        case 4: return LEDS_H_VIER;
        case 5: return LEDS_H_FUENF;
        case 6: return LEDS_H_SECHS;
        case 7: return LEDS_H_SIEBEN;
        case 8: return LEDS_H_ACHT;
        case 9: return LEDS_H_NEUN;
        case 10: return LEDS_H_ZEHN;
        case 11: return LEDS_H_ELF;
        case 12: return LEDS_H_ZWOELF;
        default: return LEDS_ERROR; // if something goes wrong
    }
}

std::pair<int, int> TimeConverterDE::getMinuteLEDs(uint8_t minutes, bool isRegionalFormat) {
    switch (minutes) {
        case 5: return LEDS_FUENF;
        case 10: return LEDS_ZEHN;
        case 15: return LEDS_VIERTEL;
        case 20: return LEDS_ZWANZIG;
        case 25: return LEDS_FUENF; // "fünf vor halb"
        case 35: return LEDS_FUENF; // "fünf nach halb"
        case 40: return LEDS_ZWANZIG; // "zwanzig vor"
        case 45: return (isRegionalFormat) ?  LEDS_VIERTEL : LEDS_DREIVIERTEL;
        case 50: return LEDS_ZEHN; // "zehn vor"
        case 55: return LEDS_FUENF; // "fünf vor"
        default: return LEDS_ERROR; // if something goes wrong
    }
}