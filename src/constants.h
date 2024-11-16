#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PRODUCT "WordClock-v2"
#define FW_VERSION "0.1-test"

// temporary
const char *NTP_SERVER = "pool.ntp.org";
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3"; // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for UI


#define LED_PIN 25
#define LDR_PIN 36

#define ILLUMINANCE_INTERVAL 200
#define LED_INTERVAL 15000
#define WIFI_SCAN_TIMEOUT 10000
#define NTP_TIMEOUT 10000


//it's collected here for now but belongs into the corresponding files


// webserver
static const char *SSID_INPUT = "ssid";
static const char *WIFI_PASS_INPUT = "wifi-pass";
static const char *BROKER_INPUT = "broker";
static const char *BROKER_USER_INPUT = "broker-user";
static const char *BROKER_PASS_INPUT = "broker-pass";
static const char *MQTT_TOPIC_INPUT = "mqtt-topic";

// LEDs
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define BRIGHTNESS 128
#define NUM_LEDS 9

#endif