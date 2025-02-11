#ifndef WCLOCK_H
#define WCLOCK_H

#include <Arduino.h>
#include <SPI.h>
#include <time.h>
#include <RTClib.h>
#include "callbacktypes.h"
#include "timezone_data.h"

using SchedulerCallback = std::function<void(SchedulerType type, uint8_t hour, uint8_t minute)>;

class WClock
{
private:
    static constexpr const char *NTP_SERVER_2 = "1.pool.ntp.org";
    static constexpr const char *NTP_SERVER_3 = "2.pool.ntp.org";
    // static const long NTP_UPDATE_INTERVAL = 21600000;
    static const int TIMEINFO_UPDATE_INTERVAL = 1000;
    SchedulerCallback schedulerCallback;
    static const u16_t ntpTimeout = 15000;
    const char *ntpServer;
    const char *timezone;
    RTC_DS3231 rtc;
    bool ntpEnabled = false;
    struct tm timeinfo;
    long ntpUpdateInterval = 21600; // seconds, 6h
    uint32_t lastNTPtime = 0;
    uint32_t lastTimeInfoUpdate = 0;
    bool scheduleEnabled = false;
    bool scheduleRunning = false;
    uint8_t scheduleStartHour = 0;
    uint8_t scheduleStartMinute = 0;
    uint8_t scheduleEndHour = 0;
    uint8_t scheduleEndMinute = 0;
    uint8_t lastHour = 0;
    uint8_t lastMinute = 0;
    void fetchNTPTime();
    bool fetchLocalTime(struct tm &timeinfo);
    bool initialized = false;
    bool updateInternal(bool ntpUpdate = false);

public:
    WClock(RTC_DS3231 &rtc);
    ~WClock();
    bool init(const SchedulerCallback &schedulerCb);
    void setTime(uint8_t hour, uint8_t minute);
    void setTimeZone(const char *timezone);
    void enableNTP(const char *timezone, const char *ntpServer, long ntpUpdateInterval);
    void synchronizeNTP();
    void disableNTP();
    void enableSchedule(long start, long end);
    void disableSchedule();
    uint8_t getHour();
    uint8_t getMinute();
    void loop();
};

#endif
