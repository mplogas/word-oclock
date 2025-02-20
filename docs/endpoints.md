# Backend Endpoints

## Light Control

| **Endpoint** | **HTTP Verb** | **Parameters** |
|-------------|--------------|----------------|
| `/toggleLight` | GET | - `enabled` (string): "0" or "1" |
| `/setLightColor` | GET | - `color` (string): Hex color code (e.g., "#FF0000") |
| `/setAutoBrightness` | GET | - `enabled` (string): "0" or "1" |
| `/setBrightness` | GET | - `value` (number): 0-255 |

TODO: that's not particular RESTful. Move them to POST/PUT like the other endpoints.

## Time Configuration

| **Endpoint** | **HTTP Verb** | **Parameters** |
|-------------|--------------|----------------|
| `/setTime` | POST | - `time` (string): Time in HH:MM format |
| `/setNTPConfig` | POST | - `enabled` (string): "0" or "1"<br>- `ntpHost` (string): NTP server address<br>- `ntpInterval` (number): Update interval<br>- `ntpTimezone` (string): Timezone identifier |
| `/setLightSchedule` | POST | - `enabled` (string): "0" or "1"<br>- `scheduleStart` (string): Start time (HH:MM)<br>- `scheduleEnd` (string): End time (HH:MM) |

## System Configuration

| **Endpoint** | **HTTP Verb** | **Parameters** |
|-------------|--------------|----------------|
| `/setHaIntegration` | POST | - `enabled` (string): "0" or "1"<br>- `mqttHost` (string): MQTT broker address<br>- `mqttPort` (number): MQTT port<br>- `mqttUsername` (string, optional): MQTT username<br>- `mqttPassword` (string, optional): MQTT password<br>- `mqttTopic` (string): MQTT topic |
| `/setClockFace` | POST | - `option` (string): "0" or "1" |
| `/resetConfig` | POST | None |

## Firmware Update

| **Endpoint** | **HTTP Verb** | **Parameters** |
|-------------|--------------|----------------|
| `/update` | GET | None |
| `/update` | POST | - `updateType` (string): "firmware" or "filesystem"<br>- Binary file upload |

## WiFi Setup

| **Endpoint** | **HTTP Verb** | **Parameters** |
|-------------|--------------|----------------|
| `/` | POST | - `ssid` (string): WiFi network name<br>- `wifi-pass` (string): WiFi password |

## Pages

| **Endpoint** | **HTTP Verb** | **Description** |
|-------------|--------------|----------------|
| `/` | GET | Redirects to /light |
| `/light` | GET | Light control page |
| `/time` | GET | Time configuration page |
| `/system` | GET | System settings page |

## Static Resources

| **Path** | **Description** |
|---------|----------------|
| `/style.css` | CSS stylesheet |
| `/index.js` | JavaScript file |
| `/favicon.ico` | Favicon |

**Notes:**
- All success responses return "Success"
- Error responses return "Error!" with HTTP 400/500 status codes
- Static resources are cached for 86400 seconds (24 hours)