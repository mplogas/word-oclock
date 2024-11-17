#include "storage.h"

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
    if (strlen(path) == 0) {
        Serial.println("Invalid path");
        return String(); // Return empty String
    }

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return String(); // Return empty String
    }

    String fileContent;
    while (file.available()) {
        fileContent += file.readStringUntil('\n');
    }

    file.close();

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
        Serial.printf("File %s written with message %s\n", path, message);
    }
    else
    {
        Serial.printf("Write failed: %s\n", path);
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