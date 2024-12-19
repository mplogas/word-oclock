#ifndef TIMECONVERTERDE_H
#define TIMECONVERTERDE_H

#include <Arduino.h>
#include <vector>
#include "itimeconverter.h"

class TimeConverterDE : public ITimeConverter
{
public:
    TimeConverterDE();
    ~TimeConverterDE();
    std::vector<std::pair<int, int>> convertTime(const uint8_t hours, const uint8_t minutes, bool isRegionalFormat, bool optionAsMinutes) override;

private:
    const std::pair<int, int> LEDS_ERROR = {0, 1};
    const std::pair<int, int> LEDS_ES = {1, 2};
    const std::pair<int, int> LEDS_IST = {3, 2};
    const std::pair<int, int> LEDS_UHR = {5, 3};
    const std::pair<int, int> LEDS_VOR = {8, 3};
    const std::pair<int, int> LEDS_NACH = {11, 4};

    // Minutes
    const std::pair<int, int> LEDS_FUENF = {15, 4};
    const std::pair<int, int> LEDS_ZEHN = {19, 4};
    const std::pair<int, int> LEDS_VIERTEL = {23, 5};
    const std::pair<int, int> LEDS_DREIVIERTEL = {28, 9};
    const std::pair<int, int> LEDS_ZWANZIG = {37, 6};
    const std::pair<int, int> LEDS_HALB = {43, 4};

    // Hours
    const std::pair<int, int> LEDS_H_EIN = {0, 3};
    const std::pair<int, int> LEDS_H_EINS = {3, 4};
    const std::pair<int, int> LEDS_H_ZWEI = {7, 4};
    const std::pair<int, int> LEDS_H_DREI = {11, 4};
    const std::pair<int, int> LEDS_H_VIER = {15, 4};
    const std::pair<int, int> LEDS_H_FUENF = {19, 4};
    const std::pair<int, int> LEDS_H_SECHS = {23, 5};
    const std::pair<int, int> LEDS_H_SIEBEN = {28, 6};
    const std::pair<int, int> LEDS_H_ACHT = {34, 4};
    const std::pair<int, int> LEDS_H_NEUN = {38, 4};
    const std::pair<int, int> LEDS_H_ZEHN = {42, 4};
    const std::pair<int, int> LEDS_H_ELF = {46, 1}; 
    const std::pair<int, int> LEDS_H_ZWOELF = {0, 5};

    // Options or Home Assistant status
    const std::pair<int, int> LEDS_OPTION_1 = {5, 5};
    const std::pair<int, int> LEDS_OPTION_2 = {10, 5};
    const std::pair<int, int> LEDS_OPTION_3 = {15, 5};
    const std::pair<int, int> LEDS_OPTION_4 = {20, 5};

    std::pair<int, int> getHourLEDs(uint8_t hour);
    std::pair<int, int> getMinuteLEDs(uint8_t minutes, bool isRegionalFormat = false);
};

#endif // TIMECONVERTERDE_H
