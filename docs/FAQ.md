# ESP32 Diagnostic Suite – FAQ (v3.29.0)

> NEW in v3.29.0: Dynamic TFT controller selection (ILI9341 or ST7789) and resolution configuration in `config.h`. See README and CONFIG.md.

> WARNING: v3.28.5 fixes ESP32 Classic pin mapping duplicates and retains ESP32-S3 GPIO remapping. Ensure your wiring and target match the documented pins. Read [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) and [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) before flashing.

## General
**Q: Which boards are officially supported?**
A: Three boards are officially supported in this version:
- ESP32-S3-DevKitC-1 N16R8 (16MB Flash, 8MB PSRAM) � Primary target
- ESP32-S3-DevKitC-1 N8R8 (8MB Flash, 8MB PSRAM)
- ESP32-DevKitC (4MB Flash, no PSRAM)

See [FEATURE_MATRIX.md](FEATURE_MATRIX.md) for detailed capability comparison.

**Q: Can I run the diagnostics without Wi-Fi?**
A: Yes. The suite can operate in offline hotspot mode or via USB serial commands. Network-specific modules will be skipped.

**Q: Is there an OTA update mechanism?**
A: Not built-in. You can integrate the binary with your preferred OTA loader. Review [BUILD_AND_DEPLOY.md](BUILD_AND_DEPLOY.md) for
packaging steps.

## Setup & configuration
**Q: Where do I store Wi-Fi credentials?**
A: Copy `secrets-example.h` to `secrets.h`, populate your SSIDs/passwords, and keep the file out of version control.

**Q: How do I enable or disable optional modules?**
A: Update the flags in `config.h` or use the REST endpoint `POST /api/modules/<id>/toggle`. Refer to
[DIAGNOSTIC_MODULES.md](DIAGNOSTIC_MODULES.md).

**Q: Can I change the default language?**
A: Set `DEFAULT_LANGUAGE` in `config.h` to either `LANG_EN` or `LANG_FR`.

## Running diagnostics
**Q: What's the difference between full and quick diagnostics?**
A: Full runs all modules sequentially. Quick skips peripheral stress tests and focuses on connectivity, memory, and reporting.

**Q: How long does a full run take?**
A: On an ESP32-S3 with Wi-Fi connected, expect ~90 seconds including throughput benchmarks and OLED/NeoPixel tests.

**Q: Where are reports stored?**
A: In RAM until exported. Use the web dashboard or `/api/report` to download them immediately after completion.

## Troubleshooting
**Q: The OLED module is marked as unavailable.**
A: Check I2C wiring (SDA/SCL), confirm the display address (0x3C by default), and ensure `ENABLE_OLED_TESTS` remains enabled.

**Q: NeoPixel tests fail on my board.**
A: Verify the LED pin assignment in `config.h` matches your hardware wiring. Some boards require powering the LED strip separately (5V external power).

**Q: REST API requests return 409 errors.**
A: Another diagnostic run is active. Wait for completion or issue `POST /api/stop` before retrying.

## Development
**Q: How do I add custom diagnostics?**
A: Implement your logic in `registerCustomDiagnostics()` inside your main source file. Emit PASS/WARN/FAIL states and push data
into the report builder.

**Q: Are unit tests available?**
A: The project focuses on hardware-driven integration tests. Use the web dashboard and exported reports to validate behaviour.

**Q: How do I contribute translations?**
A: Submit pull requests updating `languages.h` with new strings and ensure both EN/FR variants stay in sync.

## Support
**Q: Where can I report issues?**
A: Use the [GitHub issue tracker](https://github.com/morfredus/ESP32-Diagnostic/issues) and attach the latest JSON report. Note: The original Arduino IDE version [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) is archived.

**Q: Is commercial support available?**
A: Not officially. Community contributions are welcome via GitHub discussions and pull requests.











