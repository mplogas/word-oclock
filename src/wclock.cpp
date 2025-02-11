#include "wclock.h"

WClock::WClock(RTC_DS3231 &rtc) : rtc(rtc)
{
}

WClock::~WClock()
{
    schedulerCallback = nullptr;
}

bool WClock::init(const SchedulerCallback &schedulerCb)
{
    schedulerCallback = schedulerCb;
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        return false;
    }

    initialized = true;
    return true;
}

uint8_t WClock::getHour()
{
    if (!initialized)
    {
        return 0;
    }

    return timeinfo.tm_hour;
}

uint8_t WClock::getMinute()
{
    if (!initialized)
    {
        return 0;
    }

    return timeinfo.tm_min;
}

void WClock::enableSchedule(long start, long end)
{
    if (!initialized)
    {
        return;
    }

    long hourStart = start / 3600;
    long minuteStart = (start % 3600) / 60;
    long hourEnd = end / 3600;
    long minuteEnd = (end % 3600) / 60;

    if (!initialized || hourStart > 23 || hourEnd > 23 || minuteStart > 59 || minuteEnd > 59)
    {
        Serial.println("Invalid schedule parameters");
        return;
    }

    scheduleStartHour = hourStart;
    scheduleStartMinute = minuteStart;
    scheduleEndHour = hourEnd;
    scheduleEndMinute = minuteEnd;
    scheduleEnabled = true;
}

void WClock::disableSchedule()
{
    scheduleEnabled = false;
    scheduleRunning = false;
    scheduleStartHour = 0;
    scheduleStartMinute = 0;
    scheduleEndHour = 0;
    scheduleEndMinute = 0;
}

void WClock::setTime(uint8_t hour, uint8_t minute)
{
    if (!initialized || hour > 23 || minute > 59)
    {
        Serial.println("Invalid time parameters");
        return;
    }

    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = 0;
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
}

void WClock::setTimeZone(const char *timezone)
{
    if (!initialized)
    {
        return;
    }
    this->timezone = timezone;

    char buf[64];
    for(const TzEntry &tz : TIMEZONES)
    {
        strcpy_P(buf, (char*)pgm_read_ptr(&(tz.name)));

        if(strcmp(timezone, buf) == 0) {
            Serial.print("Setting timezone to ");
            Serial.println(buf);
            strcpy_P(buf, (char*)pgm_read_ptr(&(tz.posixTz)));
            Serial.println(buf);
            setenv("TZ", buf, 1);
            tzset();
            return;
        }
    }

    Serial.println("Invalid timezone");
}

void WClock::enableNTP(const char *timezone, const char *ntpServer, long ntpUpdateInterval)
{
    if (!initialized)
    {
        return;
    }

    this->ntpServer = ntpServer;
    this->ntpUpdateInterval = ntpUpdateInterval;
    this->timezone = timezone;
    ntpEnabled = true;

    updateInternal(true);
}

void WClock::disableNTP()
{
    ntpEnabled = false;
}

void WClock::synchronizeNTP()
{
    if (!initialized)
    {
        return;
    }

    updateInternal(true);
}

bool WClock::updateInternal(bool ntpUpdate)
{
    if (ntpUpdate)
    {
        fetchNTPTime();
        setTimeZone(timezone);
        if (fetchLocalTime(timeinfo)) {
            rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
            Serial.println("Synced time from NTP");
            return true;
        }
    }
    else
    {
        DateTime now = rtc.now();
        timeinfo.tm_year = now.year() - 1900;
        timeinfo.tm_mon = now.month() - 1;
        timeinfo.tm_mday = now.day();
        timeinfo.tm_hour = now.hour();
        timeinfo.tm_min = now.minute();
        timeinfo.tm_sec = now.second();
        // Serial.println("Synced time from RTC");
        return true;
    }

    return false;
}

void WClock::fetchNTPTime()
{
    if (!initialized)
    {
        return;
    }

    configTime(0, timeinfo.tm_isdst == 0 ? 0 : 3600, this->ntpServer, NTP_SERVER_2, NTP_SERVER_3);
}

bool WClock::fetchLocalTime(struct tm &timeinfo) {

    if (getLocalTime(&timeinfo, ntpTimeout))
    {
        Serial.println("Successfully obtained time");
        Serial.println(&timeinfo, "%d.%m.%Y %H:%M:%S %Z");
        lastNTPtime = millis();
        return true;
    }
    else
    {
        Serial.println("Failed to get local time");
        // DateTime now = rtc.now();
        // Serial.println(now.timestamp(now.TIMESTAMP_FULL));
        return false;
    }
}

void WClock::loop()
{
    if (!initialized)
    {
        return;
    }

    long now = millis();
    if (now - lastTimeInfoUpdate > TIMEINFO_UPDATE_INTERVAL)
    {
        lastTimeInfoUpdate = now;
        updateInternal();

        if (timeinfo.tm_hour != lastHour || timeinfo.tm_min != lastMinute)
        {
            lastHour = timeinfo.tm_hour;
            lastMinute = timeinfo.tm_min;
            if (schedulerCallback)
            {
                schedulerCallback(SchedulerType::Timestamp, timeinfo.tm_hour, timeinfo.tm_min);
            }
        }

        if (scheduleEnabled && !scheduleRunning && timeinfo.tm_hour == scheduleStartHour && timeinfo.tm_min == scheduleStartMinute)
        {
            if (schedulerCallback)
            {
                schedulerCallback(SchedulerType::ScheduleStart, timeinfo.tm_hour, timeinfo.tm_min);
                scheduleRunning = true;
            }
        }
        if (scheduleEnabled && scheduleRunning && timeinfo.tm_hour == scheduleEndHour && timeinfo.tm_min == scheduleEndMinute)
        {
            if (schedulerCallback)
            {
                schedulerCallback(SchedulerType::ScheduleEnd, timeinfo.tm_hour, timeinfo.tm_min);
                scheduleRunning = false;
            }
        }
    }

    if (ntpEnabled && millis() - lastNTPtime > ntpUpdateInterval * 60000) // minutes to ms
    {
        lastNTPtime = now;
        updateInternal(true);
    }
}