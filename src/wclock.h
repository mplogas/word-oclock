#ifndef WCLOCK_H
#define WCLOCK_H

#include <Arduino.h>
#include <SPI.h>
#include <time.h>
#include <RTClib.h>

class WClock
{
private:
    static constexpr const char *NTP_SERVER_2 = "1.pool.ntp.org";
    static constexpr const char *NTP_SERVER_3 = "2.pool.ntp.org";
    //static const long NTP_UPDATE_INTERVAL = 21600000;
    static const int TIMEINFO_UPDATE_INTERVAL = 1000;
    static const u16_t ntpTimeout = 15000;
    const char *ntpServer;
    const char *tzInfo;
    RTC_DS3231 rtc;
    bool ntpEnabled = false;
    struct tm timeinfo;
    long ntpUpdateInterval = 21600; // seconds, 6h
    uint32_t lastNTPtime = 0;
    uint32_t lastTimeInfoUpdate = 0;
    bool getNTPTime(struct tm &timeinfo);
    bool initialized = false;
    bool updateInternal(bool ntpUpdate = false); 
public:
    WClock(RTC_DS3231& rtc);
    ~WClock();
    void setTime(uint8_t hour, uint8_t minute);
    void setTimeZone(const char *timezone);
    bool enableNTP(const char *timezone, const char *ntpServer, long ntpUpdateInterval);
    void disableNTP();
    bool initRTC(); // just rtc initialization
    //bool begin(const char *timezone, const char *ntpServer); // this is split because rtc intitialization happens prior to having wifi connection so ntp would fail. 
    bool synchronizeNTP(bool tzUpdate = false);
    uint8_t getHour();
    uint8_t getMinute();
    void loop();
};


#endif
