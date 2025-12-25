# Pin Mapping – Quick Reference (v3.29.0)

> NEW in v3.29.0: Dynamic TFT controller selection (ILI9341 or ST7789) and resolution configuration in `config.h`. See FEATURE_MATRIX.md.

> WARNING: This document reflects the EXACT pin mapping from `include/board_config.h` as of v3.28.5. Ensure your wiring matches these GPIOs before powering on. Incorrect wiring may damage your ESP32 or peripherals. Read this entire document before flashing.

## Supported Environments

The firmware supports three distinct build environments, each with optimized pin configurations:

1. **`esp32s3_n16r8`** (default) – ESP32-S3 DevKitC-1 N16R8 (16MB Flash, 8MB PSRAM OPI/QSPI)
2. **`esp32s3_n8r8`** – ESP32-S3 DevKitC-1 N8R8 (8MB Flash, 8MB PSRAM)
3. **`esp32devkitc`** – ESP32 Classic DevKitC (4MB Flash, no PSRAM)

Pin mappings are defined in `include/board_config.h` using conditional compilation based on `TARGET_ESP32_S3` or `TARGET_ESP32_CLASSIC`.

---

## ESP32-S3 Pin Configuration (N16R8 / N8R8)

**Applies to:** `esp32s3_n16r8`, `esp32s3_n8r8`

### I2C Interface
- **SDA:** GPIO 15
- **SCL:** GPIO 16

### Buttons
- **BUTTON_BOOT:** GPIO 0 (built-in boot button, non-configurable)
- **BUTTON_1:** GPIO 38
- **BUTTON_2:** GPIO 39

### Rotary Encoder (HW-040)
- **CLK (A):** GPIO 47
- **DT (B):** GPIO 45
- **SW (Button):** GPIO 40

### RGB LED (Separate pins)
- **Red:** GPIO 21
- **Green:** GPIO 41 ⚠️ (was GPIO 45 in docs v3.22.1 - INCORRECT!)
- **Blue:** GPIO 42 ⚠️ (was GPIO 47 in docs v3.22.1 - INCORRECT!)

### Sensors
- **DHT (Temperature/Humidity):** GPIO 5
- **Light Sensor (ADC):** GPIO 4
- **HC-SR04 Distance Sensor:** TRIG GPIO 2 ⚠️ (was GPIO 3 - INCORRECT!) / ECHO GPIO 35 ⚠️ (was GPIO 6 - INCORRECT!)
- **PIR Motion Sensor:** GPIO 46
- **Buzzer:** GPIO 6 ⚠️ (was GPIO 14 in docs v3.22.1 - INCORRECT!)
- **PWM Test:** GPIO 20 ⚠️ (was GPIO 14 in docs v3.22.1 - INCORRECT!)

### TFT ST7789 Display (240x240)
- **MISO:** GPIO 13 ⚠️ (shared with SD card - see SD Card section)
- **MOSI:** GPIO 11
- **SCLK:** GPIO 12
- **CS:** GPIO 10
- **DC:** GPIO 9
- **RST:** GPIO 14 ⚠️ (was GPIO 13 in docs v3.22.1 - INCORRECT!)
- **Backlight (BL):** GPIO 7

### SD Card (SPI)
- **MISO:** GPIO 13 ⚠️ (shared with TFT MISO)
- **MOSI:** GPIO 11
- **SCLK:** GPIO 12
- **CS:** GPIO 1

### GPS Module (UART1)
- **RXD:** GPIO 18 (GPS TX → ESP RX)
- **TXD:** GPIO 17 (GPS RX → ESP TX)
- **PPS:** GPIO 8

### NeoPixel / WS2812B
- **Pin:** GPIO 48 (now enabled)
- **Count:** 1 LED
- **Brightness:** 50 (0-255)

### ESP32-S3 Important Notes
- **GPIO 35–44:** Reserved by the octal PSRAM/Flash interface – DO NOT USE
- **GPIO 48:** Dedicated to NeoPixel RGB LED
- **RGB LED:** Uses GPIO 21/41/42 (Red/Green/Blue) ⚠️ **NOT 21/45/47** (common documentation error!)
- **Rotary Encoder:** Uses GPIO 47/45/40 (CLK/DT/SW) – Added in v3.27.x
- **GPIO 13 Sharing:** TFT MISO and SD MISO both use GPIO 13 – requires proper SPI CS management
- **Strapping Pins:** GPIO 0, GPIO 45, GPIO 46 – keep stable during boot or boot may fail
- **USB CDC Enabled:** GPIO 19/20 reserved for USB D-/D+ (I2C moved to 15/16 to avoid conflict)
- **Boot Button:** GPIO 0 (BUTTON_BOOT) is read-only and non-configurable

---

## ESP32 Classic Pin Configuration (DevKitC)

**Applies to:** `esp32devkitc`

### I2C Interface
- **SDA:** GPIO 21
- **SCL:** GPIO 22 (different from S3!)

### Buttons
- **BUTTON_BOOT:** GPIO 0 (built-in boot button, non-configurable)
- **BUTTON_1:** GPIO 5 ⚠️ (was GPIO 2 in docs v3.22.1 - INCORRECT!)
- **BUTTON_2:** GPIO 12 ⚠️ (was GPIO 5 in docs v3.22.1 - INCORRECT!)

### Rotary Encoder (HW-040)
- **CLK (A):** GPIO 4
- **DT (B):** GPIO 13
- **SW (Button):** GPIO 26

### RGB LED (Separate pins)
- **Red:** GPIO 13
- **Green:** GPIO 26
- **Blue:** GPIO 33

### Sensors
- **DHT (Temperature/Humidity):** GPIO 15
- **Light Sensor (ADC):** GPIO 39
- **HC-SR04 Distance Sensor:** TRIG GPIO 1 ⚠️ (was GPIO 12 - INCORRECT!) / ECHO GPIO 35
- **Buzzer:** GPIO 19
- **PWM Test:** GPIO 4

### TFT ST7789 Display (240x240)
- **MISO:** GPIO 19 (if needed - shared with SD card on some configurations)
- **MOSI:** GPIO 23
- **SCLK:** GPIO 18
- **CS:** GPIO 27
- **DC:** GPIO 14
- **RST:** GPIO 25
- **Backlight (BL):** GPIO 32

### SD Card (SPI)
- **Note:** SD card support varies by board configuration
- Check `board_config.h` for your specific ESP32 Classic wiring

### GPS Module (UART2)
- **RXD:** GPIO 16 (RX2, GPS TX → ESP RX)
- **TXD:** GPIO 17 (TX2, GPS RX → ESP TX)
- **PPS:** GPIO 36

### NeoPixel / WS2812B
- **Pin:** -1 (disabled by default)
- **Count:** 8 LEDs
- **Brightness:** 50 (0-255)

### ESP32 Classic Important Notes
- **Input-Only Pins:** GPIO 34, 35, 36, 39 (VP/VN) – can only read, cannot output
- **Strapping Pins:** GPIO 0, GPIO 2, GPIO 4, GPIO 5, GPIO 12, GPIO 15 – keep stable during boot
  - **BUTTON_1 (GPIO 5)** and **BUTTON_2 (GPIO 12)** use internal pull-ups to ensure stable boot
  - ⚠️ Documentation previously listed buttons as GPIO 2/5 (v3.22.1) – this was INCORRECT
- **Rotary Encoder:** Uses GPIO 4/13/26 (CLK/DT/SW) – Added in v3.27.x
- **UART0 (USB-UART):** GPIO 1 (TX0) and GPIO 3 (RX0) reserved for flashing/debugging
  - **DISTANCE_TRIG (GPIO 1)** requires careful management to avoid UART conflicts
- **Flash Pins:** GPIO 6-11 connected to SPI Flash – DO NOT USE
- **ADC2 Limitation:** ADC2 (GPIO 0, 2, 4, 12–15, 25–27) cannot be used when WiFi is active
- **Current Limit:** Max 12 mA per GPIO; use transistor/driver for higher loads
- **Boot Button:** GPIO 0 (BUTTON_BOOT) is read-only and non-configurable

---

## Common Configuration (All Targets)

### Built-in LED
- **Default:** -1 (disabled, auto-detect at runtime)

### OLED Display (0.96" I2C)
- **Address:** 0x3C
- **Resolution:** 128x64 pixels
- **Rotation:** 0 (configurable: 0, 1, 2, 3)

### DHT Sensor Type
- **Default:** DHT22 (can be changed to DHT11 in config.h)

---

## Pin Conflict Resolution & Critical Warnings

### ESP32-S3 Conflicts
- **PSRAM/Flash Bus:** Keep GPIO 35–44 completely free (reserved by octal PSRAM/Flash)
- **RGB LED:** Uses GPIO 21/41/42 (Red/Green/Blue) ⚠️ **NOT 21/45/47** (common docs error!)
  - NeoPixel uses GPIO 48 (conflict resolved in v3.18.3)
- **Rotary Encoder:** Uses GPIO 47/45/40 (conflicts avoided by careful pin selection)
- **GPIO 13 Sharing:** TFT MISO and SD MISO both use GPIO 13 – proper CS management required
- **Distance Sensor:** TRIG GPIO 2 / ECHO GPIO 35 ⚠️ (v3.22.1 docs listed TRIG=3, ECHO=6 – WRONG!)
- **Buzzer/PWM:** Buzzer GPIO 6, PWM GPIO 20 ⚠️ (v3.22.1 docs listed both as GPIO 14 – WRONG!)
- **TFT Display:** Occupies GPIO 7–14 (RST=14, not 13); GPS uses GPIO 17–18
- **Motion Sensor:** GPIO 46; Light sensor GPIO 4

### ESP32 Classic Conflicts
- **Strapping Pins:** GPIO 0/2/4/5/12/15 must be stable during boot
  - BUTTON_1 (GPIO 5) and BUTTON_2 (GPIO 12) use internal pull-ups for safety
  - ⚠️ Previous docs listed buttons as GPIO 2/5 or GPIO 32/33 – ALL INCORRECT!
- **Rotary Encoder:** Uses GPIO 4/13/26 (CLK/DT/SW) – strapping pin GPIO 4 managed carefully
- **UART0 Conflict:** DISTANCE_TRIG uses GPIO 1 (TX0) – requires careful boot/flash management
  - Protect GPIO 1 (TX0) and GPIO 3 (RX0) during flashing
- **TFT Display:** Uses GPIO 18/23/27/14/25/32; manage CS when sharing SPI bus
- **ADC2 + WiFi:** ADC2 pins (GPIO 0, 2, 4, 12–15, 25–27) unusable when WiFi active
  - Prefer ADC1 (GPIO 32–39) for analog sensing
- **Input-Only:** GPIO 34–39 cannot drive outputs; used for GPS PPS (36) and light sensor (39)

---

## How to Change Pins

### Static Configuration (Compile-time)
1. Edit `include/board_config.h`
2. Modify pin definitions in the appropriate `#if defined(TARGET_ESP32_S3)` or `#elif defined(TARGET_ESP32_CLASSIC)` block
3. Rebuild firmware: `pio run -e <environment>`

### Dynamic Configuration (Runtime)
- **Web Interface:** Sensors section allows changing DHT, light sensor, HC-SR04, motion sensor, and buzzer pins without rebuilding
- **Serial Console:** Pin configuration commands available
- Changes persist in firmware memory until reset

---

## Selecting Build Environment

### Option 1: platformio.ini
Edit the `default_envs` line:
```ini
[platformio]
default_envs = esp32s3_n16r8  ; Change to esp32s3_n8r8 or esp32devkitc
```

### Option 2: Command Line
```bash
# Build specific environment
pio run -e esp32s3_n16r8
pio run -e esp32s3_n8r8
pio run -e esp32devkitc

# Upload to specific environment
pio run -e esp32devkitc --target upload
```

---

## Hardware Wiring Tips

### HC-SR04 Ultrasonic Sensor
- **Power:** 5V, GND
- **TRIG:** Direct connection (ESP32 output 3.3V is sufficient)
- **ECHO:** Use voltage divider (5V ? 3.3V) or level shifter
  - Resistor divider: 1kO (ECHO to ESP) + 2kO (ESP to GND)

### TFT ST7789 Display
- **Power:** 3.3V (check display module specifications)
- **Backlight:** Connect BL pin through 100O resistor
- **SPI Speed:** 40-80 MHz typical

### DHT Sensor (DHT11/DHT22)
- **Power:** 3.3V or 5V
- **Pull-up:** 4.7k-10kO resistor from DATA to VCC
- **Capacitor:** 100nF decoupling capacitor recommended

### NeoPixel / WS2812B LEDs
- **Power:** 5V for full brightness
- **Data:** 3.3V logic compatible, but 5V preferred for reliability
- **Current:** ~60mA per LED at full white

---

## Version History

- **v3.28.5 (2025-12-24):** ✅ **Input device monitoring fixes**
  - Fixed rotary encoder button stuck "Pressed" - now reads real GPIO state
  - Fixed button monitoring (BOOT/Button1/Button2) - direct constant access
  - No pin mapping changes from v3.28.2

- **v3.28.4 (2025-12-24):** Button monitoring API endpoint corrections
  - Added `/api/button-state` (singular) for individual button queries
  - No pin mapping changes

- **v3.28.3 (2025-12-24):** Rotary encoder initialization and button monitoring API
  - Rotary encoder now auto-initializes in setup()
  - Added `/api/button-states` endpoint
  - No pin mapping changes

- **v3.28.2 (2025-12-24):** ✅ **Documentation fully synchronized with `board_config.h`**
  - **CRITICAL FIXES:** Corrected 11+ GPIO assignment errors from v3.22.1 documentation:
    - ESP32-S3: RGB Green 45→41, Blue 47→42, Distance TRIG 3→2, ECHO 6→35, Buzzer 14→6, PWM 14→20, TFT RST 13→14
    - ESP32 Classic: Button1 2→5, Button2 5→12, Distance TRIG 12→1
  - **ADDED:** Rotary encoder documentation (ESP32-S3: 47/45/40, ESP32 Classic: 4/13/26)
  - **ADDED:** SD card pin mapping (ESP32-S3: MISO=13 shared with TFT, CS=1)
  - **ADDED:** BUTTON_BOOT (GPIO 0) for all boards – read-only, non-configurable
  - ⚠️ **WARNING:** Previous documentation (v3.22.1) contained DANGEROUS errors that could damage hardware

- **v3.27.x (2025-12-23):** Rotary encoder (HW-040) and button monitoring features added to firmware

- **v3.21.0:** Complete ESP32 Classic pin mapping revision – 11 changes to resolve boot issues (strapping pins GPIO 4/12/15), USB-UART communication (protect GPIO 1/3), and buttons (GPIO 32/33→5/12 with internal pull-up). Hardware migration required for ESP32 Classic users

- **v3.20.2:** Web UI pin references now dynamically sourced from `board_config.h`

- **v3.20.1:** ESP32-S3 I2C moved to 15/16 and RGB Red to 21 to free USB D-/D+ (19/20)

- **v3.18.3:** Resolved GPIO 48 conflict between NeoPixel and RGB LED on ESP32-S3

- **v3.17.1:** Refreshed ESP32-S3 and Classic pin mappings (GPS, TFT, RGB, sensors, buttons)

- **v3.15.0:** Multi-environment support with dedicated pin mappings











