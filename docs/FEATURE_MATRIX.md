# ESP32 Diagnostic Suite – Feature Matrix (v3.28.5)
# ESP32 Diagnostic Suite – Feature Matrix (v3.29.0)

> NEW in v3.29.0: Dynamic TFT controller selection (ILI9341 or ST7789) and resolution configuration in `config.h`.
| TFT test | ? | ? | ? | 4MB Flash, **no PSRAM**. Different pin mapping (see PIN_MAPPING.md). No GPIO 35-48 restrictions. |
| TFT test | ? | ? | ? | 4MB Flash, **no PSRAM**. Different pin mapping (see PIN_MAPPING.md). No GPIO 35-48 restrictions. Supports ST7789 (SPI, 240x240/240x320) and ILI9341 (SPI, 240x320) – configurable in `config.h`. |
> WARNING: This document reflects firmware v3.28.5 with EXACT pin mappings from `include/board_config.h`. All GPIO assignments have been verified and synchronized with the codebase. Previous documentation (v3.22.1) contained CRITICAL errors. Read [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) before flashing.

This matrix summarises the diagnostic coverage for the ESP32 boards supported by version 3.28.5 with three distinct build environments.
Use it to plan validation campaigns and to verify whether optional peripherals require additional wiring.

> **Important:** Current release 3.28.5 (PlatformIO) ships without BLE; for BLE diagnostics, use the archived [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) Arduino IDE version.

**✨ New in v3.28.5:** Input device monitoring fixes - rotary encoder button state reads real GPIO, button monitoring uses direct constant access.

**✨ New in v3.28.4:** Button monitoring API endpoint corrections - added `/api/button-state` (singular) for individual button queries.

**✨ New in v3.28.3:** Rotary encoder auto-initialization in setup(), button monitoring API backend endpoints.

**✨ New in v3.28.x:** Rotary encoder (HW-040) support with real-time monitoring. SD card testing API endpoints. TFT MISO pin configuration via web interface. GPIO 13 sharing warning (TFT/SD MISO).

**✨ New in v3.27.x:** Hardware button monitoring (BUTTON_BOOT, BUTTON_1, BUTTON_2) with real-time state updates.

**✨ New in v3.22.0:** MQTT Publisher for real-time system metrics publishing (memory, WiFi, sensors) to Home Assistant, NodeRED, InfluxDB and other MQTT brokers. Non-blocking with auto-reconnect. **Disabled by default.**

**⚠️ Updated in v3.28.2:** Documentation fully synchronized with `board_config.h`. All GPIO errors corrected (11+ critical fixes from v3.22.1).

## Legend
- ? � Supported out of the box by the firmware.
- ?? � Supported with optional configuration or hardware tweaks (see notes column).
- ? � Not available for the target board or requires custom firmware extensions.

## Core board capabilities
| Board | Environment | Wi-Fi scan | BLE scan | PSRAM probe | Flash integrity | NeoPixel test | OLED test | TFT test | Notes |
|-------|-------------|-----------|----------|-------------|-----------------|---------------|-----------|----------|-------|
| ESP32-S3 N16R8 | `esp32s3_n16r8` | ? | ? | ? | ? | ? | ? | ? | **Primary target.** 16MB Flash, 8MB PSRAM OPI. GPIO 35-48 reserved. Dedicated pin mapping. |
| ESP32-S3 N8R8 | `esp32s3_n8r8` | ? | ? | ? | ? | ? | ? | ? | 8MB Flash, 8MB PSRAM. Shares pin mapping with N16R8. GPIO 35-48 reserved. |
| ESP32 Classic | `esp32devkitc` | ? | ? | ? | ? | ? | ? | ? | 4MB Flash, **no PSRAM**. Different pin mapping (see PIN_MAPPING.md). No GPIO 35-48 restrictions. |

## Peripheral bus coverage
| Bus / Peripheral | Default pins | Supported boards | Notes |
|------------------|--------------|------------------|-------|
| I2C primary bus | **ESP32-S3:** SDA 15, SCL 16 ⚠️ (was 21/20 in v3.22.1 – WRONG!)<br>**ESP32 Classic:** SDA 21, SCL 22 | All supported boards | Used for OLED, sensor packs, EEPROM. Pins vary by target. |
| I2C secondary bus | Disabled by default | ESP32, ESP32-S3 | Enable via `ENABLE_SECONDARY_I2C` flag in `config.h`. |
| SPI test bus | **Varies by target** (see PIN_MAPPING.md) | ESP32, ESP32-S3 | Validates external flash/SD adaptors. |
| UART loopback | TX0/RX0 & optional UART1/UART2 | All | Requires jumper wire TX↔RX on chosen UART. |
| TFT ST7789 display | **ESP32-S3:** MISO=13 ⚠️ (shared with SD), MOSI=11, SCLK=12, CS=10, DC=9, RST=14 ⚠️ (was 13 – WRONG!), BL=7<br>**ESP32 Classic:** MISO=19, MOSI=23, SCLK=18, CS=27, DC=14, RST=25, BL=32 | All supported boards | 240x240 display. GPIO 13 shared with SD card on ESP32-S3. |
| SD Card (SPI) | **ESP32-S3:** MISO=13 ⚠️ (shared with TFT), MOSI=11, SCLK=12, CS=1<br>**ESP32 Classic:** Varies by configuration | ESP32-S3 (full), ESP32 Classic (varies) | Added v3.28.0. GPIO 13 sharing requires proper CS management. |
| RGB LED (separate) | **ESP32-S3:** R=21, G=41 ⚠️ (was 45 – WRONG!), B=42 ⚠️ (was 47 – WRONG!)<br>**ESP32 Classic:** R=13, G=26, B=33 | All | Individual RGB control pins. S3: GPIO 41/42 NOT 45/47! |
| **NeoPixel Wi-Fi indicator** | **ESP32-S3:** GPIO 48<br>**ESP32 Classic:** -1 (disabled) | ESP32-S3 only | Single RGB LED showing Wi-Fi status: yellow (connecting), green pulsing (connected), red pulsing (disconnected), violet flash (reboot). |
| Buttons | **ESP32-S3:** BOOT=0 (read-only), BTN1=38, BTN2=39<br>**ESP32 Classic:** BOOT=0 (read-only), BTN1=5 ⚠️ (was 32 in v3.22.1 – WRONG!), BTN2=12 ⚠️ (was 33 – WRONG!) | All supported boards | BUTTON_BOOT non-configurable. S3: internal pull-ups. Classic: GPIO 5/12 with internal pull-ups. |
| Rotary Encoder (HW-040) | **ESP32-S3:** CLK=47, DT=45, SW=40<br>**ESP32 Classic:** CLK=4, DT=13, SW=26 | All supported boards | Added v3.27.x. Real-time position and button monitoring via web interface. |

## Optional module checklist
| Module | Firmware flag | Default state | Description |
|--------|---------------|---------------|-------------|
| OLED diagnostics | `ENABLE_OLED_TESTS` | Enabled when OLED wiring detected | Runs pixel sweep, contrast, I2C recovery. |
| NeoPixel animator | `ENABLE_NEOPIXEL_TESTS` | Enabled | Exercises colour wheel and stress test at 800 kHz. |
| **Wi-Fi heartbeat** | None (always enabled) | Enabled | Real-time Wi-Fi status on NeoPixel LED. New in v3.21.1. |
| MQTT reporter | `ENABLE_MQTT_BRIDGE` | Disabled | Publishes summary payloads; requires broker credentials in `config.h`. |
| REST automation | `ENABLE_REST_API` | Enabled | Provides JSON endpoints for CI/CD or fleet tools. |
| Power monitor | `ENABLE_POWER_MONITOR` | Disabled | Reads INA219/INA3221 sensors over I2C. |

## Environmental assumptions
- ESP32 Arduino Core **3.3.3** (PlatformIO `espressif32@6.6.x` or Arduino Boards Manager 3.1.x+).
- USB power supply delivering at least 500�mA during OLED/NeoPixel stress tests.
- Stable Wi-Fi network (2.4�GHz) for throughput benchmarks; captive portals are not supported.

## Related documents
- [Configuration guide](CONFIG.md) � enabling or disabling optional modules.
- [Diagnostic modules deep dive](DIAGNOSTIC_MODULES.md) � execution order and metrics captured for each check.
- [Troubleshooting](TROUBLESHOOTING.md) � steps to follow when a module is reported as unavailable.











