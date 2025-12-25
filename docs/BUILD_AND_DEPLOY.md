
# ESP32 Diagnostic Suite – Build & Deploy (v3.29.0)

> NEW in v3.29.0: Dynamic TFT controller selection (ILI9341 or ST7789) and resolution configuration in `config.h`. See README and CONFIG.md.

> WARNING: v3.29.0 adds dynamic TFT controller selection (ILI9341/ST7789) and resolution in `config.h`. Ensure your wiring and target match the documented pins. Read [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) and [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) before flashing.

This document describes the supported toolchains and the recommended release checklist for firmware version 3.16.0.

> **Important:** Version 3.12.0 has been migrated from Arduino IDE to **PlatformIO**. The original Arduino IDE version [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) repository is now **archived**. Bluetooth/BLE support has been **removed**.

## Build Environments

The project supports three distinct hardware configurations via PlatformIO environments defined in `platformio.ini`:

### 1. esp32s3_n16r8 (Default)
- **Board:** ESP32-S3-DevKitC-1 N16R8
- **Flash:** 16MB (QIO mode, 80MHz)
- **PSRAM:** 8MB OPI/QSPI (80MHz, enabled)
- **Partition:** `huge_app.csv`
- **Target Define:** `TARGET_ESP32_S3`
- **Pin Mapping:** Optimized for ESP32-S3 GPIO layout

### 2. esp32s3_n8r8
- **Board:** ESP32-S3-DevKitC-1 N8R8
- **Flash:** 8MB
- **PSRAM:** 8MB (enabled)
- **Partition:** `huge_app.csv`
- **Target Define:** `TARGET_ESP32_S3`
- **Pin Mapping:** Same as N16R8 (ESP32-S3 layout)

### 3. esp32devkitc
- **Board:** ESP32-DevKitC (Classic)
- **Flash:** 4MB
- **PSRAM:** Not available
- **Partition:** `default.csv`
- **Target Define:** `TARGET_ESP32_CLASSIC`
- **Pin Mapping:** Adapted for ESP32 Classic GPIO constraints

## Toolchain Setup

The project uses PlatformIO for build management. All dependencies are declared in `platformio.ini`:

1. Open the project in **Visual Studio Code** with the **PlatformIO IDE** extension.
2. Select your target environment in `platformio.ini`:
   ```ini
   [platformio]
   default_envs = esp32s3_n16r8  ; Change to esp32s3_n8r8 or esp32devkitc
   ```
3. Run **Build** (Ctrl+Alt+B) to compile, then **Upload** (Ctrl+Alt+U) to flash.
4. Open **Serial Monitor** (Ctrl+Alt+S) at 115200 baud to verify.

Alternatively, use the PlatformIO CLI:
```bash
# Build & upload default environment
pio run --target upload
pio device monitor --baud 115200

# Build specific environment
pio run -e esp32s3_n16r8 --target upload
pio run -e esp32s3_n8r8 --target upload
pio run -e esp32devkitc --target upload

# Build all environments
pio run

# Clean build
pio run --target clean
```

**Note:** Arduino IDE and Arduino CLI are **no longer supported** for this version. Use the archived [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) repository for Arduino IDE compatibility.

## Build Status (2025-11-27)
1. `esp32s3_n16r8`: ? Build OK, ? Upload OK, ? Tested
2. `esp32s3_n8r8`: ? Build OK, ? Compilation validated
3. `esp32devkitc`: ? Build OK, ? Hardware testing pending

## Pin Configuration Notes

Each environment has dedicated pin mappings in `include/config.h`:
- **ESP32-S3:** I2C on SDA=15/SCL=16, RGB LED on 21/45/47 (Red/Green/Blue), TFT on GPIOs 7-13, GPS on 18/17/8, sensors updated (see PIN_MAPPING.md). Keep GPIO19/20 free for USB D-/D+ to maintain OTG stability.
- **ESP32 Classic:** I2C on SDA=21/SCL=22, RGB LED on 12/14/15, TFT on GPIOs 13/18/19/23/26/27, GPS on 16/17/4, sensors updated (see PIN_MAPPING.md).

See [PIN_MAPPING.md](PIN_MAPPING.md) for complete reference.

## New in v3.17.1
Refreshed ESP32-S3 and ESP32 Classic default pin mappings (GPS, TFT, RGB, sensors, buttons) and synchronized documentation + version bump to 3.17.1.

## New in v3.16.0
Dynamic configuration of OLED and TFT displays through web interface, including resolution and pin mapping. Network monitoring with client IP logging. See CHANGELOG.md for details.

## New in v3.15.0
1. **Multi-environment support:** Three distinct build configurations with hardware-specific pin mappings.
2. **Conditional compilation:** `TARGET_ESP32_S3` and `TARGET_ESP32_CLASSIC` defines for platform-specific code.
3. **Shared mappings:** Common pin assignments between ESP32-S3 N8R8 and ESP32 Classic where possible.
4. **Documentation:** Updated pin mapping reference and build instructions for all three targets.

## Pre-deployment Checklist
- [ ] Update `secrets.h` with production SSID/passwords and optional enterprise credentials.
- [ ] Confirm `DIAGNOSTIC_VERSION` matches `3.17.1` in `platformio.ini` and documentation.
- [ ] Verify target environment is set correctly in `platformio.ini` (`default_envs`).
- [ ] Review pin mappings in `config.h` for your specific hardware configuration.
- [ ] Verify multilingual assets compile without warnings (`languages.h`).
- [ ] Run a full diagnostic cycle on a reference board and export JSON/CSV reports.
- [ ] Capture screenshots or printouts of the dashboard for release notes if required.

## Acceptance tests
| Test | Procedure | Expected result |
|------|-----------|-----------------|
| Connectivity smoke test | Connect to lab Wi-Fi, run quick diagnostics. | PASS with RSSI above -70�dBm, ping < 40�ms. |
| Memory soak | Run full diagnostics 5 times consecutively. | No heap exhaustion, PSRAM stable. |
| Peripheral validation | Attach OLED + NeoPixel chain, enable modules. | PASS with consistent frame rate and LED animation. |
| REST API sanity | Trigger `/api/run` then `/api/status`. | Receives queued status and JSON payload. |

## Release packaging
1. Tag the repository with `v3.9.0` (Git annotated tag).
2. Attach compiled binaries (`.bin`) for each target board if distributing via GitHub Releases.
3. Publish release notes referencing [CHANGELOG.md](../CHANGELOG.md).
4. Share links to the updated wiki (`docs/home.md`) for support teams.

## OTA considerations
- The firmware is primarily distributed via USB flashing. For OTA, integrate with your preferred bootloader and ensure the
  diagnostic binary remains under the OTA partition size.
- Always validate signature or checksum after OTA transfer to prevent corrupted diagnostics.

## Rollback strategy
- Keep a known-good image (e.g., v3.2.21-maint) ready for emergency downgrades.
- Document the rollback steps in the internal SOP, referencing this guide for re-flashing instructions.

## Related resources
- [Installation guide](INSTALL.md)
- [Configuration guide](CONFIG.md)
- [Release notes](../CHANGELOG.md)











