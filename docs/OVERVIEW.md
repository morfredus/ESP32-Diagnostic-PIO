# ESP32 Diagnostic Suite – Overview (v3.28.5)

> WARNING: v3.28.5 fixes ESP32 Classic pin mapping duplicates and retains ESP32-S3 GPIO remapping. Ensure your wiring and target match the documented pins. Read [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) and [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) before flashing.
The ESP32 Diagnostic Suite delivers a production-ready toolkit for validating ESP32-based boards before deployment. Version 3.15.1 includes **critical memory optimizations for ESP32 Classic** and multi-environment support with three distinct build configurations optimized for different ESP32 hardware variants.

## Mission statement
- Provide a consistent methodology to qualify ESP32-S3 and ESP32 boards with multiple hardware variants.
- Support three specific boards with dedicated environments: **ESP32-S3 N16R8** (primary), **ESP32-S3 N8R8**, and **ESP32 Classic DevKitC**.
- Hardware-specific pin mappings via conditional compilation for optimal peripheral placement.
- Offer a web-first control surface for laboratory and field diagnostics with minimal setup overhead.
- Keep firmware, documentation, and exported reports aligned so QA teams can trace issues quickly.

| Area | Highlights |
|------|------------|
| Web dashboard | Responsive HTML interface served directly by the ESP32 with live status badges, activity log, and export actions. |
| Automated checks | Schedulers covering connectivity, memory, peripheral buses, display, and LED pipelines. |
| Reporting | Downloadable TXT/JSON/CSV bundles, printable view, and REST API hooks for CI integration. |
| Localisation | Runtime FR/EN switching at both the firmware (serial output) and interface levels. |
| Extensibility | Modular test registry, event bus for user-defined callbacks, and optional OLED/NeoPixel helpers. |

## New in v3.29.0: Dynamic TFT Controller Selection

- You can now select the TFT controller (`ILI9341` or `ST7789`) and resolution directly in `include/config.h`:
```cpp
#define TFT_CONTROLLER      "ST7789" // or "ILI9341"
#define TFT_WIDTH           240
#define TFT_HEIGHT          320
```
- The display output is identical regardless of the controller.
- See the updated documentation and changelog for details.

## Release 3.15.1 focus

**Critical Memory Fix**: Version 3.15.1 resolves web interface loading failures on ESP32 Classic boards by implementing chunked PROGMEM streaming for JavaScript content, reducing peak heap allocation by ~50KB.

## Release 3.15.0 focus (Multi-environment support)
- **Multi-environment support:** Three PlatformIO build environments (`esp32s3_n16r8`, `esp32s3_n8r8`, `esp32devkitc`) with hardware-specific pin configurations.
- **Conditional compilation:** Target-specific code paths using `TARGET_ESP32_S3` and `TARGET_ESP32_CLASSIC` defines.
- **Flexible deployment:** Single codebase supports ESP32-S3 (16MB/8MB Flash with PSRAM) and ESP32 Classic (4MB Flash, no PSRAM).
- **Complete documentation:** Updated pin mapping reference, build guides, and configuration details for all three targets.
- **TFT ST7789 display support:** Full support for 240x240 TFT displays with boot splash screen and real-time WiFi/IP status (v3.11.0+).
- **Dual display support:** Simultaneous OLED and TFT operation for enhanced diagnostics visualization.
- **Enhanced stability:** Resolved JavaScript loading issues, FPSTR() compilation errors, and memory optimization with streaming delivery.
- **Configuration validation:** Finalized pin mapping for ESP32-S3 DevKitC-1 N16R8 with TFT backlight on GPIO 15.
- **PlatformIO optimization:** Enhanced PSRAM configuration with improved stability and performance.

## Wiki map
Use this map to jump directly to the right knowledge category:

- [Home](home.md) � entry point, release highlights, and high-level goals.
- [Installation](INSTALL.md) � environment preparation, toolchain requirements, USB drivers, and flashing options.
- [Configuration](CONFIG.md) � Wi-Fi credentials, localisation defaults, diagnostic scheduling, and logging preferences.
- [Usage](USAGE.md) � day-to-day operator workflows, export formats, and interpreting the dashboard.
- [Architecture](ARCHITECTURE.md) � firmware layers, task scheduling, storage layout, and interface templating.
- [Troubleshooting](TROUBLESHOOTING.md) � recovery procedures, error code matrix, and safe reset scenarios.
- [Contributing](CONTRIBUTING.md) � coding standards, review process, and test expectations.
- [Feature matrix](FEATURE_MATRIX.md) � hardware coverage per board family and optional peripherals.
- [Diagnostic modules](DIAGNOSTIC_MODULES.md) � deep dive into each automated test and the metrics it emits.
- [Web interface guide](WEB_INTERFACE.md) � UI anatomy, component states, and accessibility guidelines.
- [REST API reference](API_REFERENCE.md) � endpoints, payloads, rate limits, and automation scenarios.
- [Build & deploy](BUILD_AND_DEPLOY.md) � firmware compilation, QA gating, and release checklist.
- [Security guidelines](SECURITY.md) � Wi-Fi hardening, credential rotation, and OTA update safety.
- [FAQ](FAQ.md) � quick answers to the most common integration, support, and deployment questions.

## Terminology
- **Diagnostic cycle** � the ordered execution of all enabled modules with aggregated results stored in RAM until export.
- **Snapshot report** � a frozen copy of the latest diagnostic cycle exported via web UI, REST API, or serial command.
- **Controller profile** � pre-defined configuration bundle (board type, interface options) loaded at boot.
- **Maintenance release** � a version delivering documentation and stability improvements without altering the feature surface.

## Related resources
- [CHANGELOG.md](../CHANGELOG.md) � chronological list of fixes and enhancements.
- [ESP-IDF compatibility table](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) for comparing platform expectations.
- [GitHub issues](https://github.com/morfredus/ESP32-Diagnostic/issues) � track bugs or request enhancements.
- **Note:** The original Arduino IDE version [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) is archived.











