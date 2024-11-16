#ifndef WIFISETUP_H
#define WIFISETUP_H

#include <Arduino.h>
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

WifiSetup::WifiSetup()
{
}

WifiSetup::~WifiSetup()
{
}

bool WifiSetup::connect(const char* ssid, const char* pass, u32_t timeout)
{
    //check length of ssid
    if(strlen(ssid) == 0)
    {
        Serial.println("Invalid SSID");
        return false;
    }

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, pass);
    unsigned long currentMillis = millis();
    lastMillisWifi = currentMillis;

    while (WiFi.status() != WL_CONNECTED)
    {
        currentMillis = millis();
        if (currentMillis - lastMillisWifi > timeout)
        {
        Serial.println("Failed to connect.");
        return false;
        }
    }

    Serial.print("WIFI-Client IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

bool WifiSetup::enableHostAp(const char* ssid, const char* pass)
{
    //check length of ssid
    if(strlen(ssid) == 0)
    {
        Serial.println("Invalid SSID");
        return false;
    }

    Serial.println("Starting AP...");
    WiFi.softAP(ssid, pass);

    Serial.print("WIFI-AP IP address: ");
    Serial.println(WiFi.softAPIP());

    return true;
}

#endif