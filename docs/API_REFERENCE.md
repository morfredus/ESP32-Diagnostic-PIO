
# ESP32 Diagnostic Suite – REST API Reference (v3.33.2)

> NEW in v3.33.2: TFT backlight PWM brightness control via `/api/tft-brightness` endpoint. See [CHANGELOG.md](../CHANGELOG.md) for details.

> NEW in v3.33.0: Smooth TFT progress bar, purple NeoPixel, robust BOOT logic, and dynamic TFT controller selection (ILI9341 or ST7789). See [RELEASE_NOTES_3.33.0.md](RELEASE_NOTES_3.33.0.md).

> WARNING: This document reflects firmware v3.33.2 with EXACT pin mappings from `include/board_config.h`. All GPIO assignments have been verified and synchronized with the codebase. Read [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) before flashing.

The REST API mirrors the web dashboard controls and exposes diagnostics data for automation. All endpoints are served over HTTP on
the same port as the dashboard. Authentication is sessionless; secure deployments should restrict access at the network level.

## Base URL
- Station mode: `http://<device-ip>/api`
- Hotspot mode: `http://192.168.4.1/api`
- mDNS (when supported by the host OS): `http://esp32-diagnostic.local/api`

## Conventions
- Responses are JSON encoded with UTF-8.
- Timestamps follow ISO 8601 (`YYYY-MM-DDThh:mm:ssZ`).
- Errors include an `error` object with `code`, `message`, and optional `details` array.

## Endpoints
### `GET /api/version`
Returns firmware metadata and build hashes.
```json
{
  "version": "3.30.0",
  "git_commit": "abc1234",
  "build_time": "2024-05-10T09:15:00Z"
}
```

### `POST /api/run`
Triggers a diagnostic cycle.
- Payload: `{ "mode": "full" | "quick" }`
- Response: `{ "status": "queued", "run_id": "20240510-091500" }`

### `POST /api/stop`
Stops the currently running diagnostic.
- Response: `{ "status": "stopped" }`

### `GET /api/status`
Provides real-time progress.
- Response:
```json
{
  "run_id": "20240510-091500",
  "stage": "memory",
  "progress": 58,
  "modules": [
    { "id": "env", "status": "PASS", "duration_ms": 520 },
    { "id": "connectivity", "status": "WARN", "duration_ms": 2180 }
  ]
}
```

### `GET /api/report`
Returns the aggregated results of the last completed run.
- Query parameters:
  - `format` (optional): `json` (default), `csv`, `txt`.
  - `module` (optional): filter to a single module ID.
- Responses stream the requested format with appropriate `Content-Type` headers.

### `GET /api/report/<run_id>`
Fetches an archived run by identifier. Returns `404` if the cache has been rotated.

### `GET /api/modules`
Lists all modules with metadata and enablement flags.
- Response snippet:
```json
[
  {
    "id": "memory",
    "name": "Memory & Storage",
    "enabled": true,
    "requires": ["psram"],
    "export_keys": ["memory.heap_max", "storage.flash_crc"]
  }
]
```

### `POST /api/modules/<id>/toggle`
Enables or disables an optional module.
- Payload: `{ "enabled": true }`
- Response: `{ "id": "neopixel", "enabled": true }`

### `POST /api/notes`
Attaches operator notes to the latest report.
- Payload: `{ "author": "QA-Team", "message": "Replaced antenna, reran test." }`
- Response: `{ "status": "stored" }`

## Rate limiting
- The firmware processes one diagnostic run at a time.
- Concurrent API requests are queued; long polling on `/api/status` is limited to 1 request per second.
- Bulk exports should reuse cached data rather than re-triggering runs.

## Error codes
| Code | Meaning | Typical resolution |
|------|---------|--------------------|
| `400` | Invalid payload | Validate JSON schema and supported enum values. |
| `401` | Access denied | Restrict network access or add reverse proxy authentication. |
| `404` | Resource not found | Ensure the `run_id` exists or the module is enabled. |
| `409` | Run already active | Wait for the current run to finish before starting another. |
| `503` | Hardware busy | Module is initialising hardware; retry after a few seconds. |

## Automation tips
- Use `GET /api/version` during CI to verify that the deployed unit is running firmware 3.30.0.
- Combine `/api/run` with `/api/status` polling to monitor long test suites, then download the report in JSON for further parsing.
- Store operator notes via `/api/notes` after each run to keep audit trails within the device exports.

## Related resources
- [Usage guide](USAGE.md) � explains the same workflows through the web interface.
- [Diagnostic modules](DIAGNOSTIC_MODULES.md) � lists module IDs referenced by the API.
- [Security guidelines](SECURITY.md) � recommendations for securing the HTTP endpoints.











