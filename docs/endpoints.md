# Backend Endpoints

## `firmware.html`

| **Endpoint** | **HTTP Verb** | **Parameters**                             |
|--------------|---------------|--------------------------------------------|
| `/update`    | POST          | - `file` (file upload, accepts `.bin` files)|

---

## `wifimanager.html`

| **Endpoint** | **HTTP Verb** | **Parameters**                                  |
|--------------|---------------|-------------------------------------------------|
| `/`          | POST          | - `ssid` (string): WiFi network name            |
|              |               | - `wifi-pass` (string): WiFi password           |

---

## `index.html`

| **Endpoint**             | **HTTP Verb** | **Parameters**                                    |
|--------------------------|---------------|---------------------------------------------------|
| `/update`                | GET           | None                                              |
| `/saveMQTTSettings`      | POST          | - `brokerIP` (string): MQTT broker IP address     |
|                          |               | - `brokerPort` (number): MQTT broker port         |
|                          |               | - `mqttUsername` (string): MQTT username          |
|                          |               | - `mqttPassword` (string): MQTT password          |
|                          |               | - `defaultTopic` (string): Default MQTT topic     |
| `/saveNTPSettings`       | POST          | - `ntpServer` (string): NTP server address        |
|                          |               | - `timezone` (string): Selected timezone          |
| `/saveLightSchedule`     | POST          | - `startTime` (string): Start time (HH:MM)         |
|                          |               | - `endTime` (string): End time (HH:MM)             |
| `/saveNTPSchedule`       | POST          | - `ntpInterval` (number): Update interval in hours |
| `/saveAutoBrightness`    | POST          | - `startBrightness` (number): Start brightness     |
|                          |               | - `endBrightness` (number): Max brightness        |
| `/resetWordClock`        | POST          | None                                              |

---
**Notes:**

- **`/update` in `firmware.html`:** Handles firmware uploads through the `/update` endpoint using a POST request with a `file` parameter.

- **`/` in `wifimanager.html`:** Manages WiFi configurations via the root endpoint `/` using a POST request with `ssid` and `wifi-pass` parameters.

- **`index.html` Endpoints:**
  - **`/update` (GET):** Accessed via a hyperlink (`<a href="/update">`).
  
  - **`/saveMQTTSettings` (POST):** Handles MQTT settings with the following parameters:
    - `brokerIP` (string): MQTT broker IP address
    - `brokerPort` (number): MQTT broker port
    - `mqttUsername` (string): MQTT username
    - `mqttPassword` (string): MQTT password
    - `defaultTopic` (string): Default MQTT topic
  
  - **`/saveNTPSettings` (POST):** Handles NTP settings with `ntpServer` and `timezone` parameters.
  
  - **`/saveLightSchedule` (POST):** Handles light scheduling with the following parameters:
    - `startTime` (string): Start time in HH:MM format
    - `endTime` (string): End time in HH:MM format
  
  - **`/saveNTPSchedule` (POST):** Handles NTP scheduling with the following parameter:
    - `ntpInterval` (number): Update interval in hours
  
  - **`/saveAutoBrightness` (POST):** Handles auto brightness settings with the following parameters:
    - `startBrightness` (number): Start brightness level
    - `endBrightness` (number): Maximum brightness level
  
  - **`/resetWordClock` (POST):** Resets the WordClock configuration without additional parameters.

---
