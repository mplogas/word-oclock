#ifndef TIMEZONE_DATA_H
#define TIMEZONE_DATA_H

#include <Arduino.h>

struct TzEntry {
    const char* name;
    const char* posixTz;
};


const TzEntry TIMEZONES[] PROGMEM = {
    {"Europe/Berlin", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"Europe/London", "GMT0BST,M3.5.0/1,M10.5.0"},
    {"Europe/Paris", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"Europe/Rome", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"Atlantic/Reykjavik", "GMT0"},
    {"Etc/UTC", "UTC0"}
    // add more as needed: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json
};

const size_t TZ_COUNT = sizeof(TIMEZONES) / sizeof(TzEntry);

#endif