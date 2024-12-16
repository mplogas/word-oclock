#ifndef TIMECONVERTERDE_H
#define TIMECONVERTERDE_H

#include <Arduino.h>
#include <vector>
#include "itimeconverter.h"

class TimeConverterDE : public ITimeConverter {
public:
    TimeConverterDE();
    ~TimeConverterDE();
    std::vector<std::pair<int, int>> convertTime(const uint8_t hours, const uint8_t minutes, bool isRegionalFormat, bool optionAsMinutes) override;

private:
    const std::pair<int, int> LEDS_ERROR = {0, 1};
    const std::pair<int, int> LEDS_ES = {1, 2};
    const std::pair<int, int> LEDS_IST = {3, 5};
    const std::pair<int, int> LEDS_UHR = {34, 37};
    const std::pair<int, int> LEDS_VOR = {21, 24};
    const std::pair<int, int> LEDS_NACH = {25, 29};
    //minutes
    const std::pair<int, int> LEDS_FUENF = {6, 9};
    const std::pair<int, int> LEDS_ZEHN = {10, 13};
    const std::pair<int, int> LEDS_VIERTEL = {14, 20};
    const std::pair<int, int> LEDS_DREIVIERTEL = {14, 29};
    const std::pair<int, int> LEDS_ZWANZIG = {21, 27};
    const std::pair<int, int> LEDS_HALB = {30, 33};
    //hours
    const std::pair<int, int> LEDS_H_EIN = {38, 41};
    const std::pair<int, int> LEDS_H_EINS = {38, 42};
    const std::pair<int, int> LEDS_H_ZWEI = {43, 47};
    const std::pair<int, int> LEDS_H_DREI = {48, 52};
    const std::pair<int, int> LEDS_H_VIER = {53, 57};
    const std::pair<int, int> LEDS_H_FUENF = {58, 62};
    const std::pair<int, int> LEDS_H_SECHS = {63, 67};
    const std::pair<int, int> LEDS_H_SIEBEN = {68, 72};
    const std::pair<int, int> LEDS_H_ACHT = {73, 77};
    const std::pair<int, int> LEDS_H_NEUN = {78, 82};
    const std::pair<int, int> LEDS_H_ZEHN = {83, 87};
    const std::pair<int, int> LEDS_H_ELF = {88, 91};
    const std::pair<int, int> LEDS_H_ZWOELF = {92, 96};
    // minutes or homeassisstant status
    const std::pair<int, int> LEDS_OPTION_1 = {97, 101}; 
    const std::pair<int, int> LEDS_OPTION_2 = {102, 106}; 
    const std::pair<int, int> LEDS_OPTION_3 = {107, 111}; 
    const std::pair<int, int> LEDS_OPTION_4 = {112, 116};

    std::pair<int, int> getHourLEDs(uint8_t hour);
    std::pair<int, int> getMinuteLEDs(uint8_t minutes, bool isRegionalFormat = false);
};

#endif // TIMECONVERTERDE_H
