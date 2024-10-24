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
        const char *SSID_PATH = "/wifi-ssid.txt";
        const char *WIFI_PASS_PATH = "/wifi-pass.txt";
        const char *BROKER_PATH = "/broker.txt";
        const char *BROKER_USER_PATH = "/broker-user.txt";
        const char *BROKER_PASS_PATH = "/broker-pass.txt";
        const char *MQTT_TOPIC_PATH = "/mqtt-topic.txt";
        const char *TIMEZONE_INFO_PATH = "/tz-info.txt";
        const char* getPath(StorageType type); 
    public:
        Storage(fs::FS &fs);
        bool init();
        String readFile(StorageType type);
        void writeFile(StorageType type, const char *message);
};

Storage::Storage(fs::FS &fs) : fs(fs) {}

bool Storage::init()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("An error has occurred while mounting LittleFS");
        return false;
    }
    Serial.println("LittleFS mounted successfully");
    return true;
}

String Storage::readFile(StorageType type)
{
    const char *path = getPath(type);
    if(strlen(path) == 0) {
        Serial.println("Invalid path");
        return String();
    }

    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        return String();
    }

    String fileContent;
    while (file.available())
    {
        fileContent = file.readStringUntil('\n');
        break;
    }
    return fileContent;
}

void Storage::writeFile(StorageType type, const char *message) {
    const char *path = getPath(type);
    if(strlen(path) == 0) {
        Serial.println("Invalid path");
        return;
    }

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    
    if (file.print(message))
    {
        Serial.println("File written");
    }
    else
    {
        Serial.println("Write failed");
    }
}

const char *Storage::getPath(StorageType type)
{
    switch (type)
    {
    case SSID:
        return SSID_PATH;
        break;
    case WIFI_PASS:
        return WIFI_PASS_PATH;
        break;
    case BROKER:
        return BROKER_PATH;
        break;
    case BROKER_USER:
        return BROKER_USER_PATH;
        break;
    case BROKER_PASS:
        return BROKER_PASS_PATH;
        break;
    case MQTT_TOPIC:
        return MQTT_TOPIC_PATH;
        break;
    case TIMEZONE_INFO:
        return TIMEZONE_INFO_PATH;
        break;
    default:
        return "";
        break;
    }
}

#endif
