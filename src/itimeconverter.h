#ifndef ITIMECONVERTER_H
#define ITIMECONVERTER_H

#include <Arduino.h>
#include <vector>

class ITimeConverter {
public:
    virtual ~ITimeConverter() = default;
    virtual std::vector<std::pair<int, int>> convertTime(const uint8_t hours, const uint8_t minutes, bool mode, bool optionAsMinutes) = 0;
};

#endif // ITIMECONVERTER_H