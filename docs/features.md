# Word Clock Features

## Key Features
- **Smart Word Clock** - Displays time in words using elegant LED matrix
- **Automatic Brightness** - Adapts to ambient light for optimal visibility
- **Smart Home Ready** - Native Home Assistant integration via MQTT
- **Easy Setup** - Web-based configuration and WiFi setup via access point
- **Flexible Display** - Custom colors, regional time formats, and scheduling

## Detailed Features List

### Core Functionality
- Displays time in words using LED matrix
- Supports different regional time formats (e.g., "VIERTEL VOR DREI" vs "DREIVIERTEL DREI")
- Real-time clock (RTC) backup for time keeping without network and across power outages
- Automatic time synchronization via NTP

### Light Control
- Adjustable LED brightness (0-255)
- RGB color control with hex color values (#RRGGBB)
- Auto-brightness feature using ambient light sensor
- Scheduled on/off times
- Four configurable option LEDs for status display*

### Network & Integration
- Initial WiFi setup via access point mode
- Home Assistant integration via MQTT
    - Auto-discovery of device in Home Assistant
    - Configurable MQTT broker settings
    - Host/IP address
    - Port
    - Username/Password (optional)
    - Custom topic prefix

### Configuration
- Web-based configuration interface
- Persistent settings stored in flash memory
- Configurable options:
  - NTP server and timezone
  - Light schedule
  - MQTT settings
  - Clock face options

### System
- Over-the-air (OTA) firmware updates
- File system updates for web interface
- Reset
- Modular design for easy feature extensions

### Hardware Support
- ESP32 microcontroller
- WS2812B LED strip (120 LEDs)
- DS3231 RTC module
- Light sensor for ambient light detection
- Supports various word layouts through modular design