#ifndef WCLOCK_H
#define WCLOCK_H

#include <Arduino.h>
#include <SPI.h>
#include <time.h>
#include <RTClib.h>

class WClock
{
private:
    static constexpr const char *NTP_SERVER_1 = "0.pool.ntp.org";
    static constexpr const char *NTP_SERVER_2 = "1.pool.ntp.org";
    static constexpr const char *NTP_SERVER_3 = "2.pool.ntp.org";
    static constexpr const char *TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3"; // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for UI

    const uint16_t ntpTimeout = 15000;
    const char *ntpServer;
    const char *tzInfo;
    RTC_DS3231 rtc;
    struct tm timeinfo;
    uint32_t lastNTPtime = 0;
    bool getNTPTime();
    bool initialized = false;
public:
    WClock(RTC_DS3231& rtc, const char *timezone = WClock::TIMEZONE, const char *ntpServer = WClock::NTP_SERVER_1);
    ~WClock();
    void setTimeZone(const char *timezone = WClock::TIMEZONE);
    bool init();
    bool begin();
    bool update();
    void loop();
};


#endif
