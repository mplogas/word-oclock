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

const char* Storage::readFile(StorageType type)
{
// Static buffer to hold the file content
    static char fileContent[256]; // Adjust the size as needed

    const char *path = getPath(type);
    if (strlen(path) == 0) {
        Serial.println("Invalid path");
        fileContent[0] = '\0'; // Return an empty string
        return fileContent;
    }

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        fileContent[0] = '\0'; // Return an empty string
        return fileContent;
    }

    size_t index = 0;
    while (file.available() && index < sizeof(fileContent) - 1) {
        char c = file.read();
        if (c == '\n') {
            break;
        }
        fileContent[index++] = c;
    }
    fileContent[index] = '\0'; // Null-terminate the string

    file.close(); // Close the file

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