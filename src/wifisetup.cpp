#include "wifisetup.h"

WifiSetup::WifiSetup()
{
}

WifiSetup::~WifiSetup()
{
}

bool WifiSetup::connect(const char* ssid, const char* pass, u32_t timeout)
{
    if(strlen(ssid) == 0)
    {
        Serial.println("Invalid SSID");
        return false;
    }

    Serial.println("Connecting to WiFi...");
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    uint8_t status = WiFi.waitForConnectResult(timeout);
    
    if(status == WL_CONNECTED)
    {
        Serial.print("WIFI-Client IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.printf("Failed to connect. Status code: %d\n", status);
        return false;
    }
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