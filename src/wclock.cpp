#include "wclock.h"


WClock::WClock(RTC_DS3231& rtc, const char *timezone, const char *ntpServer) : rtc(rtc){
    tzInfo = timezone;
    ntpServer = ntpServer;    
}

WClock::~WClock()
{
}

bool WClock::init()
{
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        return false;
    }

    initialized = true;
    return true;
}

bool WClock::begin()
{
    if(!initialized)
    {
        return false;
    }

    configTime(0, 0, ntpServer);
    setTimeZone(tzInfo);

    timeStatus();

    return update();
}

bool WClock::getNTPTime()
{
    if(!initialized)
    {
        return false;
    }

    if (getLocalTime(&timeinfo, ntpTimeout))
    {
        Serial.println("Successfully obtained time");
        Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S %Z");
        lastNTPtime = now;
        return true;
    }

    return false;
}

void WClock::setTimeZone(const char *timezone)
{
  setenv("TZ", timezone, 1);
  tzset();
  tzInfo = timezone;
}

bool WClock::update()
{
    setTimeZone(tzInfo);

    if(getNTPTime())
    {
        rtc.adjust(DateTime(now));
    } 

    DateTime now = rtc.now();
    if (now.year() < 2024)
    {
        Serial.println("RTC is not initialized");
        return false;
    }

    return true;
}

void WClock::loop()
{
    //update ntp every 6 hours
    if (millis() - lastNTPtime > 21600000)
    {
        update();
    }
}