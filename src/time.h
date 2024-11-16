#ifndef TIME_H
#define TIME_H

#include <Arduino.h>
#include <time.h>
#include <RTClib.h>


struct tm timeinfo;

class Clock
{
private:
    const uint16_t ntpTimeout = 10000;
    const char *ntpServer;
    const char *tzInfo;
    RTC_DS3231 rtc;
    time_t now;
    long unsigned lastNTPtime = 0;
    bool getNTPTime();
    bool initialized = false;
public:
    Clock(RTC_DS3231& rtc, const char *timezone, const char *ntpServer);
    ~Clock();
    void setTimeZone(const char *timezone);
    bool begin();
    bool update();
    void loop();
};

Clock::Clock(RTC_DS3231& rtc, const char *timezone, const char *ntpServer) : rtc(rtc){
    tzInfo = timezone;
    ntpServer = ntpServer;    
}

Clock::~Clock()
{
}

bool Clock::begin()
{
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        return false;
    }

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

    initialized = true;
    return true;
}

bool Clock::getNTPTime()
{
    if(!initialized)
    {
        return false;
    }

    unsigned long lastMillis = millis();
    configTime(0, 0, NTP_SERVER);

    //in order to get rid of the blocking behavior I just add an update indicator and let the loop function handle the update and check for progress on the ntp update until ntpTimeout is reached
    //this way the main loop is not blocked by the ntp update, downside is that begin does not guarantee a successful ntp update

    Serial.println("Waiting for NTP time sync: ");
    while (millis() - lastMillis < ntpTimeout)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        Serial.print(".");
        delay(100);

        if (getLocalTime(&timeinfo))
        {
        Serial.println("Successfully obtained time");
        Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S %Z");
        lastNTPtime = now;
        return true;
        }
    }

    return false;
}

void Clock::setTimeZone(const char *timezone)
{
  setenv("TZ", timezone, 1);
  tzset();
  tzInfo = timezone;
}

bool Clock::update()
{
    if(getNTPTime())
    {
        rtc.adjust(DateTime(now));
        return true;
    }    

    return false;
}

void Clock::loop()
{
    //update ntp every 6 hours
    if (millis() - lastNTPtime > 21600000)
    {
        update();
    }
}

#endif
