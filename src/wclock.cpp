#include "wclock.h"


WClock::WClock(RTC_DS3231& rtc) : rtc(rtc){
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

bool WClock::begin(const char *timezone, const char *ntpServer)
{
    tzInfo = timezone;
    this->ntpServer = ntpServer; 

    configTime(0, 0, this->ntpServer, NTP_SERVER_2, NTP_SERVER_3);
    return update(true);
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
        lastNTPtime = millis();
        return true;
    } else {
        Serial.println("Failed to get NTP time, using RTC for now");
        DateTime now = rtc.now();
        Serial.println(now.timestamp(now.TIMESTAMP_FULL));
        return false;
    }


    // this seems to be broken: if (getLocalTime(&timeinfo, ntpTimeout))
    // it immediately returns false, however the implementation in esp32-hal-time.c is correct
    /*
        bool getLocalTime(struct tm *info, uint32_t ms) {
            uint32_t start = millis();
            time_t now;
            while ((millis() - start) <= ms) {
                time(&now);
                localtime_r(&now, info);
                if (info->tm_year > (2016 - 1900)) {
                return true;
                }
                delay(10);
            }
            return false;
        }
    */
    // so, let's do it manually
    // uint32_t start = millis();
    // time_t now;
    // bool success = false;
    // Serial.print("Attempting to get NTP time");
    // while (millis() - start <= ntpTimeout)
    // {
    //     Serial.print(".");
    //     time(&now);
    //     localtime_r(&now, &timeinfo);
    //     if (timeinfo.tm_year > (2016 - 1900))
    //     {
    //         success = true;
    //         break;
    //     }
    //     delay(1000);
    // }
    
    // if(!success)
    // {
    //     Serial.println("Failed to get NTP time");
    //     return false;
    // }

    // Serial.println("Successfully obtained time");
    // Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S %Z");
    // lastNTPtime = now;
    // return true;
}

void WClock::setTimeZone(const char *timezone)
{
  setenv("TZ", timezone, 1);
  tzset();
  tzInfo = timezone;
}

bool WClock::update(bool tzUpdate)
{
    if(!initialized)
    {
        return false;
    }

    if (tzUpdate) setTimeZone(tzInfo);

    if(getNTPTime())
    {
        rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
        return true;
    } else {
        Serial.println("Failed to update time");
        return false;
    }
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
    //update ntp every 6 hours
    if (millis() - lastNTPtime > 21600000)
    {
        update();
    }
}