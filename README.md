# E-ink Hall Display

An Art Deco styled e-ink information display for your hallway, showing real-time weather, rain forecasts, electric scooter availability, and public transport incidents.

<div align="center">
    <img src="https://github.com/user-attachments/assets/31ffbdf6-3f92-41f2-b42c-0c5eb1c5372c" height="400">
</div>div>

## Features

-   **Weather Forecast**: Current conditions plus 5-hour hourly forecast with temperatures and icons
-   **Rain Predictions**: Next hour rain forecast in 5-minute intervals
-   **Electric Scooter Tracking**: Live count and top 5 scooters by range in your area
-   **Public Transport Alerts**: Real-time incident notifications for Lyon TCL network
-   **Smart Refresh**: Motion-activated updates to preserve e-ink display lifespan
-   **Art Deco Design**: Beautiful retro-styled interface with custom ornamental elements
-   **Battery Efficient**: Intelligent display refresh logic minimizes power consumption

## Hardware Components

-   **ESP32 Development Board** (ESP32-DevKitC or similar)
-   **Waveshare 7.5" E-Paper Display Module** (Model: 7.50inV2p)
-   **Power Supply**: 5V micro-USB for ESP32
-   **Optional**: Motion sensor (for smart refresh triggering)

### Wiring

The project uses the standard Waveshare ESP32 e-Paper Driver Board pinout:

| E-Paper Pin | ESP32 Pin |
| ----------- | --------- |
| CLK         | GPIO13    |
| MOSI        | GPIO14    |
| CS          | GPIO15    |
| DC          | GPIO27    |
| RST         | GPIO26    |
| BUSY        | GPIO25    |

## Software Requirements

### ESPHome Device

-   [ESPHome](https://esphome.io/) 2023.x or later
-   Python 3.8 or later (for ESPHome CLI)

### Home Assistant Integration

-   [Home Assistant](https://www.home-assistant.io/) 2024.x or later
-   Required integrations:
    -   Weather integration (e.g., Met.no, OpenWeatherMap)
    -   Météo France (for rain predictions)
    -   Motion sensor integration
    -   File/folder access for shell command outputs

### External APIs

-   **Dott Scooters**: Public GBFS API (no authentication required)
-   **Grand Lyon Data**: Account required for TCL transit data ([create account](https://data.grandlyon.com/))

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/eink-hall.git
cd eink-hall
```

### 2. Configure Secrets

Create a `secrets.yaml` file in the project root:

```yaml
wifi_ssid: 'YourWiFiSSID'
wifi_password: 'YourWiFiPassword'
```

### 3. Customize Location (Optional)

If you're not in Lyon, France, you'll need to modify:

**Scooter bounding box** (`home_assistant_config/scripts/update_trottinettes.sh`):

```bash
TOP_LEFT_LAT=45.76811428558231
TOP_LEFT_LON=4.856690919710467
BOTTOM_RIGHT_LAT=45.76762793009454
BOTTOM_RIGHT_LON=4.857606657786966
```

**Weather entity** (`home_assistant_config/template.yaml`):

-   Replace `weather.lyon` with your weather entity
-   Replace `sensor.lyon_next_rain` with your rain forecast sensor

### 4. Install ESPHome

```bash
pip install esphome
```

### 5. Compile and Upload

```bash
# Validate configuration
esphome config eink-hall.yaml

# Compile firmware
esphome compile eink-hall.yaml

# Upload to device (first time via USB)
esphome upload eink-hall.yaml

# For subsequent updates (OTA over WiFi)
esphome upload eink-hall.yaml --device eink-hall.local
```

### 6. Configure Home Assistant

Copy the contents of `home_assistant_config/` to your Home Assistant configuration directory:

```bash
# Copy to your Home Assistant config directory
cp home_assistant_config/*.yaml /config/
cp -r home_assistant_config/scripts /config/scripts/

# Create data directory for script outputs
mkdir -p /config/script_data

# Make scripts executable
chmod +x /config/scripts/*.sh
```

Add to your `configuration.yaml`:

```yaml
template: !include template.yaml
command_line: !include command_line.yaml
shell_command: !include config.yaml
automation: !include automations.yaml
```

Restart Home Assistant after configuration.

## Usage Commands

### Development

```bash
# View live logs from the device
esphome logs eink-hall.yaml

# Run full cycle: compile + upload + logs
esphome run eink-hall.yaml

# Clean build files
esphome clean eink-hall.yaml
```

### Home Assistant

The display automatically updates when:

-   Motion is detected in the hallway
-   New data is received from sensors

You can also manually trigger updates:

-   Use the "Eink-Hall - Refresh Screen" button in Home Assistant
-   Call `script.update_screen` service

To manually update external data:

```bash
# From Home Assistant
service: shell_command.update_trottinettes
service: shell_command.update_tcl_incidents
```

## Configuration

### Display Refresh Timing

Edit `eink-hall.yaml` to customize:

-   **Boot delay**: Line 14 (default: 5 seconds)
-   **Data check interval**: Line 114 (default: every 10 seconds)
-   **Motion loop duration**: Line 104 (default: 2 minutes)
-   **Motion loop refresh rate**: Line 79 (default: 15 seconds)

### Custom Fonts

Place custom fonts in the `fonts/` directory and reference them in `eink-hall.yaml`. Current fonts:

-   **GothamRnd-Book.ttf**: Body text
-   **RousseauDeco.ttf**: Art Deco titles
-   **materialdesignicons-webfont.ttf**: Icons

### Layout Customization

The display layout is rendered in the `display:` lambda section (line 591+ in `eink-hall.yaml`). Modify the `top_*` variables to adjust section positions.

## Architecture

The system consists of three main components:

1. **Home Assistant**: Aggregates data from multiple sources (weather APIs, scooter APIs, transit APIs) into a single sensor entity
2. **ESP32 + ESPHome**: Subscribes to the aggregated sensor and renders the display
3. **E-ink Display**: Shows the information with intelligent refresh logic to preserve display lifespan

Data flows from external APIs → Home Assistant scripts → Home Assistant sensors → ESPHome → E-ink display.
