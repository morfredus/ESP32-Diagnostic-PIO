## TFT Controller and Resolution Selection (v3.29.0)

You can now select the TFT controller (`ILI9341` or `ST7789`) and set the resolution directly in `include/config.h`:
```cpp
#define TFT_CONTROLLER      "ST7789" // or "ILI9341"
#define TFT_WIDTH           240
#define TFT_HEIGHT          320
```
The display output is identical regardless of the controller.
See the changelog for details.
# Configuration (EN) – v3.28.5

> WARNING: v3.28.5 fixes ESP32 Classic pin mapping duplicates and retains ESP32-S3 GPIO remapping. Ensure your wiring and target match the documented pins. Read [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) and [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) before flashing.

## Build Environment Selection

**New in v3.15.0:** The firmware supports three hardware configurations via PlatformIO environments:

- **`esp32s3_n16r8`** (default): ESP32-S3 with 16MB Flash, 8MB PSRAM
- **`esp32s3_n8r8`**: ESP32-S3 with 8MB Flash, 8MB PSRAM  
- **`esp32devkitc`**: ESP32 Classic with 4MB Flash, no PSRAM

Select your target in `platformio.ini`:
```ini
[platformio]
default_envs = esp32s3_n16r8  ; Change to esp32s3_n8r8 or esp32devkitc
```

Or via command line:
```bash
pio run -e esp32devkitc --target upload
```

## Hardware-Specific Pin Mappings

Pin configurations are defined in `include/board_config.h` (automatically included via `config.h`) using conditional compilation:

- **ESP32-S3 targets** (`esp32s3_n16r8`, `esp32s3_n8r8`): Use `TARGET_ESP32_S3` define
- **ESP32 Classic** (`esp32devkitc`): Use `TARGET_ESP32_CLASSIC` define

See [PIN_MAPPING.md](PIN_MAPPING.md) for complete pin reference for each environment.

## Wi-Fi credentials
The firmware reads credentials from `include/secrets.h` (copy `include/secrets-example.h`). The file is excluded from version control via `.gitignore`.

```cpp
// secrets.h (example)
const char* WIFI_SSID_1 = "MySSID";
const char* WIFI_PASS_1 = "MyPassword";

// Optional backup network
const char* WIFI_SSID_2 = "BackupSSID";
const char* WIFI_PASS_2 = "BackupPassword";
```

- Declare at least one SSID/password pair.
- Avoid storing production credentials in version control.

## Language settings
- French is the default language.
- Use `/api/set-language?lang=en` or the UI toggle to switch to English.
- Translation strings live in `languages.h` inside the `Translations` structure. Add new languages by extending this file and exposing them in the UI.
- Release 3.8.0 keeps the shared catalog in sync while adding the Wi-Fi splash and NimBLE safeguards with no extra configuration required.

## Display configuration

### OLED configuration
- Default pins: SDA=15, SCL=16 (ESP32-S3 default in this project; keep GPIO19/20 free for USB D-/D+).
- Change wiring on the fly with `/api/oled-config?sda=<pin>&scl=<pin>`.
- After updating pins, the firmware automatically reinitialises the display and stores the mapping in RAM.

### TFT ST7789 configuration (v3.11.0+)
- **Pins vary by environment** (configured in `include/config.h`):

**ESP32-S3 (esp32s3_n16r8 / esp32s3_n8r8):**
  - MISO: GPIO 13 (shared with SD card), MOSI: GPIO 11, SCLK: GPIO 12
  - CS: GPIO 10, DC: GPIO 9
  - RST: GPIO 14 ⚠️ (was GPIO 13 in old docs - INCORRECT!), BL: GPIO 7

**ESP32 Classic (esp32devkitc):**
  - MOSI: GPIO 23, SCLK: GPIO 18
  - CS: GPIO 27, DC: GPIO 14
  - RST: GPIO 25, BL: GPIO 32

- TFT resolution: 240x240 pixels
- Displays boot splash screen and real-time WiFi/IP status
- Enable/disable via `ENABLE_TFT_DISPLAY` flag in config.h
- Supports simultaneous OLED and TFT operation (dual display mode)
- **Note:** On ESP32 Classic, SCLK (GPIO 18) conflicts with HC-SR04 ECHO pin

## Sensors configuration

## Hardware buttons (v3.17.1+)
- Optional feature enabled by `ENABLE_BUTTONS` in `include/config.h`.
- Uses existing `PIN_BUTTON_1` and `PIN_BUTTON_2` defined per target in `board_config.h`.
- Default actions:
  - BTN1: short press plays a brief buzzer tone for user feedback.
  - BTN2: short press cycles RGB LED colors (red ? green ? blue ? white).
- ESP32-S3 default pins: BUTTON_BOOT=0 (read-only), BTN1=38, BTN2=39 to avoid boot/reset conflicts.
- ESP32 Classic: BUTTON_BOOT=0 (read-only), BTN1=5, BTN2=12 with internal pull-up ⚠️ (old docs stated 32/33 or 34/35 - ALL INCORRECT!).


### Distance Sensor (HC-SR04)

- TRIG is an output pin; ECHO is an input pin.
- Power the sensor at 5V and protect the ECHO line with a resistor divider (5V to 3.3V) before the ESP32 input.
- ESP32-S3 with Octal PSRAM/Flash (e.g., DevKitC-1 N16R8): avoid using GPIO 35..48 for TRIG/ECHO because these pins are reserved by the OPI memory interface. The firmware will flag this configuration as invalid.
- Defaults ESP32-S3: TRIG = GPIO 2 ⚠️ (old docs said 3 - WRONG!), ECHO = GPIO 35 ⚠️ (old docs said 6 - WRONG!).
- Defaults ESP32 Classic: TRIG = GPIO 1, ECHO = GPIO 35.
- See `docs/PIN_MAPPING.md` for current GPIO assignments from `board_config.h`.

## Bluetooth� naming rules
- Names must be 3�31 characters, ASCII alphanumeric plus hyphen or underscore.
- Invalid input returns HTTP 400 without restarting advertising.
- Successful renaming persists until reboot.

## Advanced options
- **PSRAM:** Automatically enabled for ESP32-S3 N16R8 and N8R8 builds via `platformio.ini` configuration.
- **BLE support:** Not available in this PlatformIO version. BLE functionality has been removed.
- **Debugging:** Open the Serial Monitor to observe Wi-Fi connection retries (mirrored on OLED if present), diagnostic progress, and sensor readings. Poll `/api/memory-details` for precise fragmentation metrics.
- **Multi-board support:** Select the appropriate environment in PlatformIO (`esp32s3_n16r8`, `esp32s3_n8r8`, or `esp32devkitc`) before building.











