#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>

enum StorageType
{
    SSID,
    WIFI_PASS,
    BROKER,
    BROKER_USER,
    BROKER_PASS,
    MQTT_TOPIC,
    TIMEZONE_INFO
};

class Storage
{
    private:
        fs::FS &fs;
        static constexpr const char *SSID_PATH = "/wifi-ssid.txt";
        static constexpr const char *WIFI_PASS_PATH = "/wifi-pass.txt";
        static constexpr const char *BROKER_PATH = "/broker.txt";
        static constexpr const char *BROKER_USER_PATH = "/broker-user.txt";
        static constexpr const char *BROKER_PASS_PATH = "/broker-pass.txt";
        static constexpr const char *MQTT_TOPIC_PATH = "/mqtt-topic.txt";
        static constexpr const char *TIMEZONE_INFO_PATH = "/tz-info.txt";
        const char* getPath(StorageType type); 
    public:
        Storage(fs::FS &fs);
        bool init();
        String readFile(StorageType type);
        void writeFile(StorageType type, const char *message);
};

#endif
