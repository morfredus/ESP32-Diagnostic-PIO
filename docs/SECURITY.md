# ESP32 Diagnostic Suite – Security Guidelines (v3.29.0)

> NEW in v3.29.0: Dynamic TFT controller selection (ILI9341 or ST7789) and resolution configuration in `config.h`. See README and CONFIG.md.

> WARNING: v3.28.5 fixes ESP32 Classic pin mapping duplicates and retains ESP32-S3 GPIO remapping. Ensure your wiring and target match the documented pins. Read [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) and [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) before flashing.

Although the ESP32 Diagnostic Suite is typically used in controlled lab environments, following these security guidelines reduces
exposure when testing pre-production hardware across all supported build environments (ESP32-S3 N16R8, ESP32-S3 N8R8, ESP32 Classic).

## Network hardening
- Prefer a dedicated QA Wi-Fi network with WPA2-PSK or WPA2-Enterprise credentials.
- Change default hotspot SSID/password in `secrets.h` before field trials.
- Disable hotspot fallback entirely by setting `ENABLE_AP_FALLBACK` to `0` when not required.
- Restrict access to the REST API via firewall rules or reverse proxy authentication when deployed on shared networks.

## Credential management
- Store Wi-Fi credentials in `secrets.h` and exclude the file from version control (see `.gitignore`).
- Rotate passwords regularly and update all flashed units before decommissioning old credentials.
- For enterprise WPA2/EAP, use device-specific certificates and revoke them if a unit is lost.

## Firmware integrity
- Verify the SHA-256 hash of compiled binaries before flashing large batches.
- Keep the build environment patched and reproducible (documented in [BUILD_AND_DEPLOY.md](BUILD_AND_DEPLOY.md)).
- Tag releases with annotated Git tags (e.g., `v3.15.1`) to ensure traceability between code and deployed firmware.
- When deploying to multiple hardware targets (ESP32-S3 / ESP32 Classic), validate that the correct build environment binary is flashed to each device type to avoid pin mapping conflicts.

## Web interface best practices
- Avoid exposing the dashboard directly to the public internet.
- Use modern browsers with up-to-date TLS libraries when reverse proxying over HTTPS.
- Enable automatic logout in the proxy when diagnostics are run in shared labs.
- Review [WEB_INTERFACE.md](WEB_INTERFACE.md) for ARIA and accessibility considerations that also benefit kiosk deployments.

## Data retention
- Exported JSON/CSV/TXT reports can contain board identifiers and operator notes; treat them as confidential QA records.
- Store exports on secured storage with backup policies aligned with your organisation's compliance requirements.
- Purge device caches after retrieving reports by issuing `POST /api/stop` followed by a power cycle.

## Incident response
- If tampering is suspected, perform a factory reset: erase flash via `esptool.py erase_flash`, then reflash v3.15.1 with the correct build environment for your hardware.
- Revoke associated Wi-Fi credentials and update audit logs with report exports for the affected unit.
- File an incident ticket referencing the report `run_id` for traceability.

## Related resources
- [Configuration guide](CONFIG.md)
- [REST API reference](API_REFERENCE.md)
- [Build & deploy](BUILD_AND_DEPLOY.md)











