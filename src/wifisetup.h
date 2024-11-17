#ifndef WIFISETUP_H
#define WIFISETUP_H

#include <Arduino.h>
#include <Network.h>
#include <WiFi.h>

class WifiSetup
{
    private:
        unsigned long lastMillisWifi;
    public:
        WifiSetup();
        ~WifiSetup();
        bool connect(const char* ssid, const char* pass, u32_t timeout);
        bool enableHostAp(const char* ssid, const char* pass);
};



#endif