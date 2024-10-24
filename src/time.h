#ifndef TIME_H
#define TIME_H

#include <Arduino.h>
#include <time.h>
#include <RTClib.h>

class Clock
{
private:
    const char *NTP_SERVER = "pool.ntp.org";
    const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3"; // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for UI
    const uint16_t NTP_TIMEOUT = 10000;
    RTC_DS3231 rtc;
    struct tm timeinfo;
    time_t now;
    long unsigned lastNTPtime = 0;
public:
    Clock(RTC_DS3231& rtc);
    ~Clock();
    void setTimeZone(const char *timezone);
    bool begin();
    bool getNTPtime();
};

Clock::Clock(RTC_DS3231& rtc) : rtc(rtc) {}

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
    return true;
}

bool Clock::getNTPtime()
{

  unsigned long lastMillis = millis();
  configTime(0, 0, NTP_SERVER);

  Serial.println("Waiting for NTP time sync: ");
  while (millis() - lastMillis < NTP_TIMEOUT)
  {
    time(&now);
    localtime_r(&now, &timeinfo);
    Serial.print(".");
    delay(100);

    if (getLocalTime(&timeinfo))
    {
      Serial.println(" Successfully obtained time");
      return true;
    }
  }

  Serial.println("Failed to get time from NTP");
  return false;
}


#endif
