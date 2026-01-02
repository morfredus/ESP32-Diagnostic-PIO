
# ESP32 Diagnostic Suite (v3.33.1)

> **Note**: v3.31.0 introduces **readable web interface source code** with automatic minification! Develop in clean, maintainable code while keeping optimized firmware size.

Comprehensive diagnostic firmware for ESP32 microcontrollers with interactive web dashboard, automated hardware tests, and bilingual content (FR/EN). The firmware targets PlatformIO with ESP32 Arduino Core 3.3.3 and supports ESP32-S3 and ESP32 Classic targets.


## 🐛 Version 3.33.1 - ESP32 Classic Compilation Fix

**Patch Release:**
- **Fixed critical compilation error** for ESP32 Classic (`esp32devkitc` environment)
- **Added missing pin definitions**: `MOTION_SENSOR` (GPIO 34) and `NEOPIXEL` (GPIO 2)
- **ESP32-S3 unaffected** - this fix only impacts ESP32 Classic users

See [CHANGELOG.md](CHANGELOG.md) for full details.

## Previous: Version 3.33.0 - TFT, NeoPixel & BOOT

**What's new:**
- Smooth, flicker-free TFT progress bar, optimized partial redraw
- NeoPixel purple during progress, Earthbeat restored if cancelled
- Clean boot screen return, robust state machine, refactored code
- Detailed internal documentation

## ✨ Version 3.31.0 - Readable Web Interface Source Code

**New Developer Workflow:**
- **Readable Source Files**: Edit clean, formatted HTML/CSS/JavaScript in `web_src/`
- **Automatic Minification**: Python scripts convert readable code to optimized firmware
- **Memory Preserved**: CSS ~26%, JS ~18-28% size reduction maintained
- **Full Documentation**: Complete workflow guide in `web_src/README.md`

**Quick Start:**
```bash
# Install Python dependencies (one-time)
pip install rcssmin rjsmin jsbeautifier cssbeautifier

# Extract current code (optional)
python tools/extract_web_sources.py

# Edit files in web_src/, then minify
python tools/minify_web.py

# Build and upload as usual
pio run --target upload
```

See [web_src/README.md](web_src/README.md) for detailed instructions.

## Previous: Version 3.30.0 - Runtime TFT Driver Switching

**New Features:**
- **Dynamic TFT driver switching** from Web UI - No recompilation needed!
- Switch between `ILI9341` and `ST7789` drivers on-the-fly
- Both drivers loaded simultaneously for instant switching
- New driver selector in TFT configuration section
- No reboot required for driver changes

**How to use:**
1. Open the Web UI and navigate to the TFT screen section
2. Select your desired driver from the new "Driver" dropdown (ILI9341 or ST7789)
3. Click "Apply Config" - the driver switches immediately!
4. Test your display with the built-in test buttons

**Default driver at boot:**
Edit `include/config.h`:
```cpp
#define TFT_USE_ILI9341  // or TFT_USE_ST7789
```

**See:** [CHANGELOG.md](CHANGELOG.md) for full version 3.30.0 details.

## Previous: Version 3.29.0

**Dynamic TFT Controller Selection:**
- Dynamic selection of TFT controller (`ILI9341` or `ST7789`) via `config.h`
- Resolution configurable in `config.h`
- Display output identical regardless of controller

**See:** [CHANGELOG.md](CHANGELOG.md) for full version 3.29.0 details.

## Previous: Version 3.28.4

**Input Devices Fixed:**
- ✅ **Rotary Encoder Initialization** - Works immediately after boot
- ✅ **Button Monitoring API** - Backend endpoints added
- ✅ **Automatic Setup** - All input devices initialize in `setup()`

**See:** [CHANGELOG.md](CHANGELOG.md) for full version 3.28.3 details.

## Previous: Version 3.28.2

**BUTTON_BOOT JavaScript Error Fixed:**
- ✅ **BUTTON_BOOT Error FIXED** - ReferenceError on Input Devices page resolved
- ✅ **Root Cause Identified** - GPIO constants were injected in wrong location
- ✅ **All Button Constants Working** - BUTTON_BOOT, BUTTON_1, BUTTON_2 now properly available

**See:** [CHANGELOG.md](CHANGELOG.md) for full version 3.28.2 details.

## Previous: Version 3.28.1

**MISO Integration & SD Card Fixes:**
- ✅ **MISO Display Fixed** - No more "MISO: undefined" in web interface
- ✅ **SD Card on ESP32-S3 Works** - Fixed SPI bus selection (HSPI vs FSPI)
- ✅ **MISO Configuration Sync** - Changes in web UI now persist correctly
- ⚠️ **BUTTON_BOOT** - NOT actually fixed despite documentation (fixed in v3.28.2)

**See:** [docs/RELEASE_NOTES_3.28.1.md](docs/RELEASE_NOTES_3.28.1.md) for details.

## Previous: Version 3.25.0

**Dynamic GPIO Pin Remapping Restored:**
- **Dynamic pin remapping works again** - Users can change GPIO pins via Web UI without recompilation
- **Two-layer architecture** - UPPERCASE defines in `board_config.h`, lowercase variables in `main.cpp`
- **All handlers functional** - I2C, RGB LED, Buzzer, DHT, Light, Distance, Motion sensors

**See:** [docs/RELEASE_NOTES_3.25.0.md](docs/RELEASE_NOTES_3.25.0.md) for complete technical details.

## Previous: Version 3.22.1

**Real-time Wi-Fi connectivity feedback via NeoPixel/WS2812 RGB LED:**
- **Yellow (connection in progress)** during Wi-Fi connection attempt at startup
- **Green heartbeat (0, 50, 0) pulsing every 1 second** when Wi-Fi connected
- **Red heartbeat (50, 0, 0) pulsing every 1 second** when Wi-Fi disconnected
- **Violet flash (255, 0, 255)** on BOOT button long press confirmation
- **Non-blocking heartbeat** managed in main loop - updates every 1 second
- **Test isolation** - heartbeat pauses during `/api/neopixel-test` and `/api/neopixel-pattern` calls, automatically resumes after

## Previous: version 3.21.0

**Complete ESP32 Classic Pin Mapping Revision** - Hardware migration required:
- **11 pin changes** to resolve boot issues (strapping pins GPIO 4/12/15) and USB-UART communication (protect GPIO 1/3)
- **Improved buttons**: GPIO 5/12 with internal pull-up ⚠️ (docs previously stated 32/33 or 34/35 - both INCORRECT!)
- **Safe RGB LEDs**: Moved away from strapping pins (12→13, 15→25)
- **GPS stability**: PPS moved from GPIO 4 (strapping) to GPIO 36 (dedicated input-only)
- **Detailed documentation**: See `docs/PIN_MAPPING_CHANGES_FR.md` for numbered list of changes with technical explanations
- **Impact**: ESP32-S3 unchanged, ESP32 Classic requires hardware rewiring
- **Safety**: Added detailed reminders in `board_config.h` (3.3V voltages, strapping, current limits)

## Previous: version 3.20.4

**Project Name Refactoring** - Eliminated hardcoded project name strings:
- All occurrences of "ESP32 Diagnostic" replaced with `PROJECT_NAME` macro from platformio.ini
- Affected files: `main.cpp` (5 locations), `languages.h`, `web_interface.h`
- Project name now centrally defined in build configuration
- Improves maintainability and customization flexibility

## Previous: version 3.20.3

**Code Optimization Version** - Systematic memory efficiency improvements:
- Applied 9 optimizations ([OPT-001] through [OPT-009]) eliminating 90+ string allocations
- Buffer-based formatting for version strings, uptime, GPIO lists, and test results
- Created reusable constants (DEFAULT_TEST_RESULT_STR, OK_STR, FAIL_STR) used in 30+ locations
- Converted 13 debug/status messages to snprintf approach (2-9 allocations ? 1)
- **No functionality changes** - all pins, tests, and features remain identical
- Successfully compiled and validated on ESP32-S3 and ESP32 CLASSIC targets

## Previous: version 3.20.2
1. **Dynamic Pin References in Web UI**: Web interface now displays correct GPIO pins based on compiled target
   - All hardcoded pin values replaced with dynamic references from `board_config.h`
   - RGB LED pins, DHT sensor, light sensor, distance sensor, motion sensor, and PWM now read from configuration
   - Pin values injected as JavaScript constants, ensuring UI always displays actual hardware pins
   - Serial startup banner shows TARGET identification and actual pin values for verification

## Version 3.20.1 Features
1. **USB/OTG Stability (ESP32-S3)**: USB D-/D+ lines freed by moving defaults away from GPIO19/20.
   - Default I2C: SDA=15, SCL=16
   - RGB LED: Red=21, Green=41, Blue=42 ⚠️ (docs stated 45/47 - INCORRECT! board_config.h uses 41/42)
   - OTG stabilized; note: GPIO41/42 chosen to avoid strapping pin conflicts.

## Version 3.20.0 Features
1. **Advanced Button Handling**: Enhanced button features with visual feedback and interactive controls
   - **BOOT Button (GPIO 0)**: Long press (2 seconds) triggers system restart with progress bar on TFT screen; release before 2s clears screen
   - **Button 1 (GPIO 38)**: Short press cycles through RGB LED colors (Off ? Red ? Green ? Blue ? White)
   - **Button 2 (GPIO 39)**: Short press triggers confirmation beep
   - Real-time visual feedback during long-press operations on TFT
   - Debouncing and long-press detection for reliable operation

## Version 3.19.0 Features
1. **Pin Mapping Isolation**: Board GPIO definitions extracted to `include/board_config.h`; `config.h` now contains only common configuration parameters.
   - All pin definitions for ESP32-S3 (N16R8 / N8R8) and ESP32 Classic consolidated in one place
   - Buttons remain on GPIO 38/39 (ESP32-S3) to avoid USB upload/reset conflicts
   - No other pin values changed
2. **Secrets Management**: Wi-Fi configuration file renamed from `wifi-config.h` to `secrets.h`
   - Old `wifi-config.h` headers now emit compile error to force migration
   - `secrets.h` protected in `.gitignore` to prevent credential leaks
   - Use `include/secrets-example.h` as template
3. **Complete Documentation**: All user-facing documents updated to reflect v3.19.0 architecture changes and features
4. **Version Consistency**: `PROJECT_VERSION` updated to 3.19.0 in `platformio.ini`

## Version 3.18.2 Features
1. **Translation Keys**: Added missing translation keys for full GPS and environmental sensor interface support.
   - `gps_status`: GPS status indicator
   - `temperature_avg`: Average temperature label for multi-sensor display
   - `pressure_hpa`: Barometric pressure with unit display
   - `altitude_calculated`: Altitude calculated from pressure

## Version 3.18.1 Features
1. **AHT20 Sensor Fix**: Corrected bit extraction algorithm for humidity and temperature values (proper 20-bit extraction from 6-byte response).
2. **Environmental Sensors API**: Fixed JSON structure for web interface compatibility.

## Version 3.18.0 Features
1. **GPS Module Support**: Complete integration of NEO-6M/NEO-8M/NEO-M GPS receiver.
   - NMEA sentence parsing (RMC, GGA, GSA, GSV)
   - Track latitude, longitude, altitude, speed, heading
   - Satellite count and signal quality (HDOP, VDOP, PDOP)
   - Web interface integration with GPS status display
2. **Environmental Sensors**: AHT20 (temperature/humidity) + BMP280 (pressure/altitude) on I2C.
   - Real-time streaming to web interface
   - Altitude calculation from barometric pressure
   - Sensor status indicators (OK/Error/Not detected)
3. **Web Interface Enhancements**: GPS module section in Wireless page, environmental sensors in Sensors page.

## Version 3.17.1 Features
1. **Refreshed Pin Mapping (ESP32-S3 & Classic)**: ⚠️ **HISTORICAL NOTE**: This version's GPIO assignments have been revised. Current values in `board_config.h` differ from this version's documentation. See PIN_MAPPING.md for current GPIOs. ESP32-S3 current: RGB LED R=21/G=41/B=42 (NOT 47/48/45), HC-SR04 TRIG=2/ECHO=35 (NOT 3/6), Buzzer=6/PWM=20 (NOT both 14), TFT RST=14 (NOT 13). ESP32 Classic current: Buttons BTN1=5/BTN2=12 (NOT 34/35), HC-SR04 TRIG=1 (NOT 32).
2. **Docs + Version**: Mapping guides (EN/FR), README, feature matrices, usage guides, and build notes aligned with new defaults; `PROJECT_VERSION` bumped to `3.17.1`.

## Version 3.17.0 Features
1. **Hardware Buttons (Optional)**: Simple actions tied to BTN1/BTN2 without changing pin mapping.
   - BTN1: short press ? brief buzzer beep (user feedback).
   - BTN2: short press ? cycle RGB LED colors (red ? green ? blue ? white).
   - Enable via `ENABLE_BUTTONS` in `include/config.h`.

## Version 3.16.0 Features
1. **Network Monitoring**: Automatic logging of connected client IP addresses in serial monitor for improved diagnostics.
2. **WiFi Status Display**: Real-time network connection status in WiFi tab showing current IP address, SSID, gateway, DNS, and signal strength.
3. **OLED Resolution Configuration**: Dynamic OLED screen resolution (width � height) configuration via web interface.
4. **TFT Configuration Interface**: Complete TFT screen configuration including pin mapping (MOSI, SCLK, CS, DC, RST, BL), resolution, and rotation parameters.
5. **New API Endpoint**: `/api/tft-config` for complete TFT configuration with real-time validation.
6. **Enhanced Screen Info**: Updated `/api/screens-info` to include full resolution and pin details for both OLED and TFT.

## Version 3.15.1 Features
**Critical Fix**: Resolved web interface loading failure on ESP32 Classic (`esp32devkitc`) caused by memory exhaustion. JavaScript content is now streamed in small chunks from PROGMEM, eliminating heap crashes on boards without PSRAM. All environments benefit from improved memory efficiency.

## Version 3.15.0 Features
1. **Multi-Environment Support**: Three distinct build environments for different ESP32 boards:
   - `esp32s3_n16r8` (default): ESP32-S3 with 16MB Flash + 8MB PSRAM
   - `esp32s3_n8r8`: ESP32-S3 with 8MB Flash + 8MB PSRAM
   - `esp32devkitc`: ESP32 Classic with 4MB Flash (no PSRAM)
2. **Hardware-Specific Pin Mapping**: Dedicated pin configurations in `board_config.h` (automatically included by `config.h`) for each target.
3. **ESP32-S3 Configuration (adjusted in 3.19.0)**: I2C (SDA=21, SCL=20), RGB LED (R=19, G=47, B=45), NeoPixel=48, buttons BTN1=38 / BTN2=39, GPS RX=18/TX=17/PPS=8, TFT MOSI=11/SCLK=12/CS=10/DC=9/RST=13/BL=7, sensors (PWM/Buzzer=14, DHT=5, Motion=46, Light=4, HC-SR04 TRIG=3/ECHO=6).
4. **ESP32 Classic Configuration (adjusted in 3.17.1)**: I2C (SDA=21, SCL=22), RGB LED (R=12, G=14, B=15), buttons BTN1=34 / BTN2=35, GPS RX=16/TX=17/PPS=4, TFT MOSI=23/SCLK=18/CS=19/DC=27/RST=26/BL=13, sensors (PWM/Buzzer=5, DHT=25, Motion=36, Light=2, HC-SR04 TRIG=32/ECHO=33).
5. **Shared Mappings**: Common pin assignments between ESP32-S3 N8R8 and ESP32 Classic where hardware permits.

## Version 3.14.0 Key Points
1. **TFT Web Interface**: Complete cartridge for testing TFT ST7789 display (240x240) via web interface.
2. **Individual TFT Tests**: 8 tests available (boot screen, colors, shapes, text, lines, animation, progress, final message).
3. **Boot Screen Restore Buttons**: Quick restore of boot display for both OLED and TFT.
4. **TFT REST API**: `/api/tft-test`, `/api/tft-step`, `/api/tft-boot` for programmatic control.
5. **OLED REST API**: `/api/oled-boot` to restore OLED boot screen.

## Project Structure
- `src/main.cpp` � firmware entry point, main loop, scheduler, and HTTP handlers.
- `include/languages.h` � translation catalog and hot language selector.
- `include/web_interface.h` � HTML/CSS/JS templates served by firmware.
- `include/board_config.h` � board-specific pin mapping (selected via TARGET_ESP32_*).
- `include/secrets-example.h` � template for storing Wi-Fi credentials (copy to `include/secrets.h`).
- `platformio.ini` � PlatformIO configuration for all supported ESP32 targets.
- `docs/` � complete wiki gathering installation, usage, API, and deployment guides (FR/EN).

## Documentation
| Topic | English | French |
|-------|---------|--------|
| Wiki (home) | [home.md](docs/home.md) | [home_FR.md](docs/home_FR.md) |
| Overview | [OVERVIEW.md](docs/OVERVIEW.md) | [OVERVIEW_FR.md](docs/OVERVIEW_FR.md) |
| Feature Matrix | [FEATURE_MATRIX.md](docs/FEATURE_MATRIX.md) | [FEATURE_MATRIX_FR.md](docs/FEATURE_MATRIX_FR.md) |
| Diagnostic Modules | [DIAGNOSTIC_MODULES.md](docs/DIAGNOSTIC_MODULES.md) | [DIAGNOSTIC_MODULES_FR.md](docs/DIAGNOSTIC_MODULES_FR.md) |
| Web Interface | [WEB_INTERFACE.md](docs/WEB_INTERFACE.md) | [WEB_INTERFACE_FR.md](docs/WEB_INTERFACE_FR.md) |
| REST API | [API_REFERENCE.md](docs/API_REFERENCE.md) | [API_REFERENCE_FR.md](docs/API_REFERENCE_FR.md) |
| Build & Deploy | [BUILD_AND_DEPLOY.md](docs/BUILD_AND_DEPLOY.md) | [BUILD_AND_DEPLOY_FR.md](docs/BUILD_AND_DEPLOY_FR.md) |
| Security | [SECURITY.md](docs/SECURITY.md) | [SECURITY_FR.md](docs/SECURITY_FR.md) |
| FAQ | [FAQ.md](docs/FAQ.md) | [FAQ_FR.md](docs/FAQ_FR.md) |
| Installation | [INSTALL.md](docs/INSTALL.md) | [INSTALL_FR.md](docs/INSTALL_FR.md) |
| Configuration | [CONFIG.md](docs/CONFIG.md) | [CONFIG_FR.md](docs/CONFIG_FR.md) |
| Usage Guide | [USAGE.md](docs/USAGE.md) | [USAGE_FR.md](docs/USAGE_FR.md) |
| Firmware Architecture | [ARCHITECTURE.md](docs/ARCHITECTURE.md) | [ARCHITECTURE_FR.md](docs/ARCHITECTURE_FR.md) |
| Troubleshooting | [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | [TROUBLESHOOTING_FR.md](docs/TROUBLESHOOTING_FR.md) |
| Contributing | [CONTRIBUTING.md](CONTRIBUTING.md) | [CONTRIBUTING_FR.md](CONTRIBUTING_FR.md) |
| Changelog | [CHANGELOG.md](CHANGELOG.md) | [CHANGELOG_FR.md](CHANGELOG_FR.md) |


## Export & Print Reports (TXT, JSON, CSV, Print)

All export and print formats now include:
- **Full environmental sensor data**: AHT20 (temperature, humidity, status), BMP280 (temperature, pressure, altitude, status), average temperature, global status.
- **Full GPS data**: module status, fix, satellites, latitude, longitude, altitude, speed, HDOP, date/time.
- All previous board, memory, Wi-Fi, GPIO, test, and performance information.
- Printable HTML report includes all these sections in a clear, structured layout.

See the "Export" tab in the web interface for download/print options. Data is available in all formats for both manual and automated analysis.

---

## Main Capabilities
...existing code...

## Compatibility & Requirements
- **Boards:**
  - ESP32-S3-DevKitC-1 N16R8 (16MB Flash, 8MB PSRAM OPI/QSPI) � Primary target
  - ESP32-S3-DevKitC-1 N8R8 (8MB Flash, 8MB PSRAM)
  - ESP32-DevKitC Classic (4MB Flash, no PSRAM)
- **Platform:** PlatformIO with ESP32 Arduino Core 3.3.3+
- **Build Environments:**
  - `esp32s3_n16r8` (default): ESP32-S3 with huge_app partition, PSRAM enabled, OPI support
  - `esp32s3_n8r8`: ESP32-S3 with huge_app partition, 8MB Flash
  - `esp32devkitc`: ESP32 Classic with default partition, 4MB Flash
- **Environment Selection:** Modify `default_envs` in `platformio.ini` or use `pio run -e <environment>`
- **Libraries:** Adafruit BusIO ^1.17, Adafruit GFX ^1.12, Adafruit ST7735/ST7789 ^1.11, Adafruit NeoPixel ^1.12, U8g2 ^2.36 (automatic installation via platformio.ini).

## Support
- License: [MIT](LICENSE)
- Support & Issues: [open a GitHub issue](https://github.com/morfredus/ESP32-Diagnostic/issues/new/choose) with reproduction steps and board details.
- Contribution: [fork the repository](https://github.com/morfredus/ESP32-Diagnostic/fork) then see [CONTRIBUTING.md](CONTRIBUTING.md).











