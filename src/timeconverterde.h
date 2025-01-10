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
    const std::pair<int, int> LEDS_ES = {110, 2};
    const std::pair<int, int> LEDS_IST = {113, 3};
    const std::pair<int, int> LEDS_UHR = {11, 3};
    const std::pair<int, int> LEDS_VOR = {79, 3};
    const std::pair<int, int> LEDS_NACH = {82, 4};

    // Minutes
    const std::pair<int, int> LEDS_FUENF = {117, 4};
    const std::pair<int, int> LEDS_ZEHN = {106, 4};
    const std::pair<int, int> LEDS_VIERTEL = {92, 7};
    const std::pair<int, int> LEDS_DREIVIERTEL = {88, 11};
    const std::pair<int, int> LEDS_ZWANZIG = {99, 7};
    const std::pair<int, int> LEDS_HALB = {66, 4};

    // Hours
    const std::pair<int, int> LEDS_H_EIN = {61, 3};
    const std::pair<int, int> LEDS_H_EINS = {60, 4};
    const std::pair<int, int> LEDS_H_ZWEI = {62, 4};
    const std::pair<int, int> LEDS_H_DREI = {45, 4};
    const std::pair<int, int> LEDS_H_VIER = {33, 4};
    const std::pair<int, int> LEDS_H_FUENF = {51, 4};
    const std::pair<int, int> LEDS_H_SECHS = {16, 5};
    const std::pair<int, int> LEDS_H_SIEBEN = {55, 6};
    const std::pair<int, int> LEDS_H_ACHT = {23, 4};
    const std::pair<int, int> LEDS_H_NEUN = {37, 4};
    const std::pair<int, int> LEDS_H_ZEHN = {27, 4};
    const std::pair<int, int> LEDS_H_ELF = {41, 3};
    const std::pair<int, int> LEDS_H_ZWOELF = {71, 5};

    // Options or Home Assistant status
    const std::pair<int, int> LEDS_OPTION_1 = {2, 1};
    const std::pair<int, int> LEDS_OPTION_2 = {4, 1};
    const std::pair<int, int> LEDS_OPTION_3 = {6, 1};
    const std::pair<int, int> LEDS_OPTION_4 = {8, 1};

    std::pair<int, int> getHourLEDs(uint8_t hour);
    std::pair<int, int> getMinuteLEDs(uint8_t minutes, bool isRegionalFormat = false);
};

#endif // TIMECONVERTERDE_H
