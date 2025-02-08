#include "wclock.h"


WClock::WClock(RTC_DS3231& rtc) : rtc(rtc){
}

WClock::~WClock()
{
}

bool WClock::initRTC()
{
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        return false;
    }

    initialized = true;
    return true;
}

bool WClock::enableNTP(const char *timezone, const char *ntpServer, long ntpUpdateInterval)
{
    tzInfo = timezone;
    this->ntpServer = ntpServer; 

    configTime(0, 0, this->ntpServer, NTP_SERVER_2, NTP_SERVER_3);
    ntpEnabled = true;
    return synchronizeNTP(true);
}

void WClock::disableNTP()
{
    ntpEnabled = false;
}

bool WClock::getNTPTime(struct tm &timeinfo)
{
    if(!initialized)
    {
        return false;
    }

    if (getLocalTime(&timeinfo, ntpTimeout))
    {
        Serial.println("Successfully obtained time");
        Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S %Z");
        lastNTPtime = millis();
        return true;
    } else {
        Serial.println("Failed to get NTP time, using RTC for now");
        // DateTime now = rtc.now();
        // Serial.println(now.timestamp(now.TIMESTAMP_FULL));
        return false;
    }
}

bool WClock::updateInternal(bool ntpUpdate)
{
    if (ntpUpdate && getNTPTime(timeinfo))
    {
        rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
        Serial.println("Synced time from NTP");
        return true;
    } else {
        DateTime now = rtc.now();
        timeinfo.tm_year = now.year() - 1900;
        timeinfo.tm_mon = now.month() - 1;
        timeinfo.tm_mday = now.day();
        timeinfo.tm_hour = now.hour();
        timeinfo.tm_min = now.minute();
        timeinfo.tm_sec = now.second();
        //Serial.println("Synced time from RTC");
        return false;
    }

    return true;
}

void WClock::setTime(uint8_t hour, uint8_t minute)
{
    if(!initialized)
    {
        return;
    }

    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = 0;
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
}

void WClock::setTimeZone(const char *timezone)
{
  setenv("TZ", timezone, 1);
  tzset();
  tzInfo = timezone;
}

bool WClock::synchronizeNTP(bool tzUpdate)
{
    if(!initialized)
    {
        return false;
    }

    if (tzUpdate) setTimeZone(tzInfo);

    return updateInternal(true);
}

uint8_t WClock::getHour()
{
    if(!initialized)
    {
        return 0;
    }

    return timeinfo.tm_hour;
}

uint8_t WClock::getMinute()
{
    if(!initialized)
    {
        return 0;
    }

    return timeinfo.tm_min;
}

void WClock::loop()
{
    if(!initialized)
    {
        return;
    }
    
    long now = millis();
    if (now - lastTimeInfoUpdate > TIMEINFO_UPDATE_INTERVAL)
    {
        lastTimeInfoUpdate = now;
        updateInternal();
    }

    if (ntpEnabled && millis() - lastNTPtime > ntpUpdateInterval * 1000)
    {
        lastNTPtime = now;
        updateInternal(true);
    }
}