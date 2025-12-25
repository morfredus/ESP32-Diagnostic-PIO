# Usage Guide (EN) – v3.29.0

> NEW in v3.29.0: Dynamic TFT controller selection (ILI9341 or ST7789) and resolution configuration in `config.h`. See README and CONFIG.md.

> WARNING: v3.28.5 fixes ESP32 Classic pin mapping duplicates and retains ESP32-S3 GPIO remapping. Ensure your wiring and target match the documented pins. Read [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) and [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) before flashing.

**Multi-environment support:** Version 3.17.1 refreshes default pin mappings for ESP32-S3 and ESP32 Classic while keeping the three hardware-specific build environments and the ESP32 Classic memory optimizations. Before first use, ensure you've selected and flashed the correct environment for your board (see [BUILD_AND_DEPLOY.md](BUILD_AND_DEPLOY.md)).

## 1. First boot
1. Open the Serial Monitor at 115200 baud immediately after flashing.
2. Wait for the firmware to connect to Wi-Fi. Successful connection prints the assigned IP and the URL `http://ESP32-Diagnostic.local`, and any attached OLED mirrors the association/DHCP/progress states.
3. If Wi-Fi fails, the console displays error hints (wrong password, AP not found, etc.). Update `secrets.h` accordingly.

## 2. Accessing the dashboard
- Navigate to the printed IP address or to `http://ESP32-Diagnostic.local` (mDNS) from a device on the same network.
- The interface defaults to French. Use the FR/EN toggle in the top-right corner to switch language without reloading the page.

## 3. Web interface overview
The dashboard is organised into tabs:
- **Overview** � hardware summary, memory usage, firmware version, Bluetooth� status.
- **LEDs** � onboard LED controls plus NeoPixel/WS2812B patterns.
- **Screens** � OLED diagnostics with step-by-step animations and manual trigger buttons.
- **Advanced Tests** � CPU/memory benchmarks, stress tests and flash partition listing.
- **GPIO** � interactive pin tester with live status.
- **Wi-Fi** � detailed scan results (RSSI, channel, security, PHY, bandwidth).
- **Performance** � benchmark history and runtime metrics.
- **Export** � download TXT/JSON/CSV reports or open the printable report.
- Version 3.15.1 supports three build environments with different pin mappings for I2C, TFT, RGB LED, and buttons. Hardware-specific configurations are automatically compiled based on the selected environment. ESP32 Classic benefits from memory optimizations for reliable web UI operation.

The navigation bar collapses gracefully on mobile devices and preserves active tab state.

## 4. Bluetooth� Low Energy tools
- BLE advertising starts automatically when the firmware detects a supported board.
- A dedicated card in the Overview tab shows device name, connection state, and allows renaming.
- Invalid names are rejected with HTTP 400 responses to keep the GATT service consistent.

## 5. REST API
All endpoints return JSON unless stated otherwise:
- `GET /api/test-gpio` � run the GPIO sweep.
- `GET /api/wifi-scan` � launch a Wi-Fi scan.
- `GET /api/benchmark` � execute CPU and memory benchmarks.
- `GET /api/set-language?lang=fr|en` � switch UI language.
- `GET /api/get-translations` � download current translation catalog.
- `GET /api/oled-config?sda=<pin>&scl=<pin>` � reconfigure OLED I2C pins.
- `GET /api/oled-test` � run the complete OLED test suite (takes ~25 seconds).
- `GET /api/oled-message?message=TEXT` � display a custom message on the OLED.
- `GET /api/memory-details` � return flash/PSRAM/SRAM metrics plus fragmentation percentage for post-run analysis.
- `GET /export/txt`, `/export/json`, `/export/csv` � download reports in various formats.

## 6. Reports and logging
- Serial output mirrors key actions (Wi-Fi status, BLE state, test results).
- Exported reports include board information, memory breakdown, benchmark metrics, Wi-Fi scan, GPIO results, and OLED status.
- Keep JSON exports for machine parsing and TXT/CSV for manual analysis.
- Use the `/api/memory-details` endpoint when reports warn about fragmentation and, on 3.8.0, double-check that BLE scan responses include either fresh results or a clear error message if the radio is busy.

## 7. Hardware buttons (optional)
- If enabled via `ENABLE_BUTTONS`, the following defaults apply without changing pin mapping:
  - BTN1: short press plays a brief buzzer tone.
  - BTN2: short press cycles RGB LED colors (red ? green ? blue ? white).
- Pins are target-specific (`PIN_BUTTON_1` / `PIN_BUTTON_2` defined in `config.h`). On ESP32 Classic, GPIO 34�39 have no internal pull-ups; use external pull-ups if required for BTN2=35.

## 8. Best practices
- Re-run Wi-Fi scans after moving the device to a new location to refresh RSSI values.
- Stop NeoPixel animations before powering down external LEDs to avoid ghosting.
- When changing OLED pins, run `/api/oled-config` followed by `/api/oled-test` to validate wiring.

## 9. Post-release debugging checklist (3.8.0)
- Power-cycle the board with an OLED connected and confirm the Wi-Fi splash advances through association, DHCP, and completion before the dashboard loads.
- Trigger `/api/wifi-scan` and `/api/ble-scan` (or the UI scan button) to ensure NimBLE builds return valid results and resume advertising without raising conversion errors.
- Download TXT/JSON/CSV exports and verify the filenames embed version 3.9.0 while the BLE and Wi-Fi sections reflect the refreshed compatibility guidance.











