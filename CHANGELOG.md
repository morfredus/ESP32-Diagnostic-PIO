### [CHANGE 1] NeoPixel Earthbeat: smooth fade

- Earthbeat effect now uses a smooth fade (brightness min/max) instead of blinking.
- No delay, no blocking: LED remains fluid, no performance impact.
- Documented and optimized code.

## [Version 3.33.4] - 2026-01-02

### 🐛 Bug Fix - UX Enhancement

**Removed Page Reload on Configuration Apply**

This patch release fixes an unpleasant UX issue where clicking "Apply Configuration" buttons caused a full page reload and redirected users back to the Overview page.

#### 🎯 Problem Fixed

When users clicked "Apply Configuration" in the **Display & Signal** page (Built-in LED, NeoPixel, TFT Display sections), the page performed a full `location.reload()` after 1.5-2 seconds, causing:
- ❌ Loss of current page context
- ❌ Forced redirect to Overview page
- ❌ Interruption of workflow
- ❌ Non-modern, jarring user experience

#### ✅ Solution Implemented

**Removed all page reloads** and replaced with **dynamic DOM updates** without navigation:
- ✅ Users stay on current page after configuration
- ✅ Smooth, modern web app experience
- ✅ Instant visual feedback
- ✅ No interruption to workflow

#### 📝 Technical Details

**Modified Functions in `web_src/app.js`:**

| Function | Line | Old Behavior | New Behavior |
|----------|------|--------------|--------------|
| `configBuiltinLED()` | 1052-1067 | `location.reload()` after 1500ms | Dynamic update of GPIO display |
| `configNeoPixel()` | 1097-1117 | `location.reload()` after 1500ms | Dynamic update of GPIO and count display |
| `configTFT()` | 1258-1299 | `location.reload()` after 2000ms | Dynamic update of pins and resolution display |

**Changes:**
```javascript
// BEFORE (v3.33.3)
if (d.success) {
    setTimeout(() => location.reload(), 1500);
}

// AFTER (v3.33.4)
if (d.success) {
    // Update GPIO display dynamically without page reload
    const gpioDisplay = document.getElementById('builtin-gpio-display');
    if (gpioDisplay) {
        gpioDisplay.textContent = gpio;
    }
}
```

**Good Practice Already Present:**
- `configOLED()` was already doing dynamic updates correctly (no reload)
- All `apply*()` functions (RGB, Buzzer, DHT, etc.) were already working correctly (no reload)

#### 🔧 Modified Files

- `web_src/app.js` - Removed 3 `location.reload()` calls, added dynamic DOM updates
- `platformio.ini` - Version bump to 3.33.4

#### ⚠️ Important Notes

- **No Backend Changes**: This is purely a frontend UX fix
- **Backward Compatible**: All existing API endpoints work unchanged
- **Minification Required**: Run `python tools/minify_web.py` after pulling changes
- **All Pages Reviewed**: Verified no other pages have this issue

### 🔄 Version Control

- **Version bumped**: `3.33.3` → `3.33.4` in `platformio.ini`
- This is a **PATCH** version increment per SEMVER (bug fix, UX enhancement, backward compatible)

---

## [Version 3.33.3] - 2026-01-02

### ✨ User Interface Enhancement

**TFT Brightness Control Web UI**

This patch release adds user-friendly web interface controls for the TFT brightness feature introduced in v3.33.2, completing the brightness control implementation.

#### 🎯 New Features

- **Interactive Brightness Slider**: Real-time brightness control (0-255) in TFT Display section
- **Live Value Display**: Current brightness level shown next to slider
- **Preset Buttons**: Quick access buttons (OFF, 25%, 50%, 75%, 100%)
- **Bilingual Support**: Full FR/EN translations for all UI elements
- **Responsive Design**: Styled slider with gradient background and hover effects

#### 📝 Technical Details

- **Modified Files**:
  - `web_src/app.js` - Brightness slider HTML and JavaScript functions
  - `web_src/styles.css` - Modern range input styling
  - `include/languages.h` - FR/EN translations (`tft_brightness`, `brightness_level`)
  - `platformio.ini` - Version bump to 3.33.3

- **New JavaScript Functions**:
  ```javascript
  updateBrightnessValue(value)      // Update display when slider moves
  setTFTBrightnessLevel(level)      // Set brightness via API
  getTFTBrightness()                // Fetch current brightness from device
  ```

- **Web Interface Location**: Display & Signal → TFT Display section (below driver selection)

#### 🎨 UI Components

- **Range Slider**: 0-255 with live value display
- **Preset Buttons**:
  - OFF (0)
  - 25% (64)
  - 50% (128)
  - 75% (192)
  - 100% (255)
- **Visual Styling**: Gradient slider track, cyan thumb with shadow effects

#### 🌐 Translations Added

| Key | English | Français |
|-----|---------|----------|
| `tft_brightness` | Brightness | Luminosité |
| `brightness_level` | Brightness level | Niveau de luminosité |

#### ⚠️ Important Notes

- **Backend Requirement**: Requires v3.33.2 backend (PWM brightness API)
- **Minification**: Run `python tools/minify_web.py` to embed web changes
- **app-lite.js**: No changes needed (simplified version doesn't include TFT features)
- **Live Update**: Slider changes apply immediately via `/api/tft-brightness` endpoint

### 🔄 Version Control

- **Version bumped**: `3.33.2` → `3.33.3` in `platformio.ini`
- This is a **PATCH** version increment per SEMVER (UI enhancement, backward compatible)

---

## [Version 3.33.2] - 2026-01-02

### ✨ New Features

**TFT Backlight PWM Brightness Control**

This minor release adds dynamic PWM-based brightness control for the TFT display backlight, allowing users to adjust screen brightness in real-time via web interface.

#### 🎯 New Features

- **PWM Brightness Control**: TFT backlight now uses PWM (0-255) instead of simple ON/OFF
- **Web API Endpoint**: `/api/tft-brightness` for real-time brightness adjustment
- **Dedicated PWM Channel**: Uses LEDC channel 1 (avoiding conflicts with other PWM uses)
- **Default Brightness**: 255 (maximum) from `config.h` (`TFT_BACKLIGHT_PWM`)
- **Smooth Transitions**: Hardware PWM at 5 kHz for flicker-free dimming

#### 📝 Technical Details

- **Modified Files**:
  - `include/tft_display.h` - PWM backlight control functions
  - `src/main.cpp` - New API endpoint `/api/tft-brightness`
  - `platformio.ini` - Version bump to 3.33.2

- **New Functions**:
  ```cpp
  void setTFTBrightness(uint8_t brightness)  // Set brightness 0-255
  uint8_t getTFTBrightness()                 // Get current brightness
  ```

- **API Endpoint**: `/api/tft-brightness`
  - **GET**: Returns current brightness level (JSON)
  - **POST**: Set brightness with `value` parameter (0-255)
  - **Response**: JSON with brightness, min, max values

#### 🔧 PWM Configuration

| Parameter | Value | Description |
|-----------|-------|-------------|
| PWM Channel | 1 | Dedicated to TFT backlight (avoids conflict with channel 0) |
| PWM Frequency | 5000 Hz | 5 kHz for flicker-free operation |
| PWM Resolution | 8-bit | 0-255 brightness levels |
| Default Brightness | 255 | Maximum brightness (configurable in `config.h`) |

#### 📡 API Usage Examples

**Get current brightness:**
```bash
curl http://esp32.local/api/tft-brightness
# Response: {"brightness":255,"min":0,"max":255}
```

**Set brightness to 50% (128):**
```bash
curl "http://esp32.local/api/tft-brightness?value=128"
# Response: {"success":true,"message":"TFT brightness set to 128/255","brightness":128}
```

**Turn off backlight (brightness 0):**
```bash
curl "http://esp32.local/api/tft-brightness?value=0"
```

#### ⚠️ Important Notes

- **ESP32-S3**: TFT_BL on GPIO 7
- **ESP32 Classic**: TFT_BL on GPIO 32
- **PWM Channel**: Channel 1 dedicated to backlight (channel 0 reserved for PWM tests)
- **Brightness Persistence**: Current brightness level maintained until changed or device reset

### 🔄 Version Control

- **Version bumped**: `3.33.1` → `3.33.2` in `platformio.ini`
- This is a **MINOR** version increment per SEMVER (new feature, backward compatible)

---

## [Version 3.33.1] - 2026-01-02

### 🐛 Bug Fixes

**ESP32 Classic Compilation Fix**

This patch release fixes a critical compilation error for the `esp32devkitc` environment caused by missing pin definitions.

#### 🎯 Fixed Issues

- **Missing Pin Definitions**: Added `MOTION_SENSOR` and `NEOPIXEL` constants for ESP32 Classic (DevKitC) in `board_config.h`
  - `MOTION_SENSOR` → GPIO 34 (input-only, ideal for PIR sensor)
  - `NEOPIXEL` → GPIO 2 (shared with LED_BUILTIN for simplicity)

#### 📝 Technical Details

- **Modified Files**:
  - `include/board_config.h` - Added missing pin definitions for TARGET_ESP32_CLASSIC section
  - `platformio.ini` - Version bump to 3.33.1

- **Compilation Errors Fixed**:
  ```
  src/main.cpp:216:25: error: 'MOTION_SENSOR' was not declared
  src/main.cpp:253:15: error: 'NEOPIXEL' was not declared
  include/web_interface.h:510:16: error: 'MOTION_SENSOR' was not declared
  ```

#### 🔧 Pin Assignments (ESP32 Classic)

| Constant | GPIO | Type | Notes |
|----------|------|------|-------|
| MOTION_SENSOR | 34 | Input | GPIO34 is input-only, ideal for PIR |
| NEOPIXEL | 2 | Output | Shared with LED_BUILTIN |

#### ⚠️ Important Notes

- **ESP32-S3**: No changes required (pins already defined)
- **ESP32 Classic**: Users can now compile and use MOTION_SENSOR and NEOPIXEL features
- **GPIO Sharing**: NEOPIXEL (GPIO 2) is shared with LED_BUILTIN - users should choose one or the other in their hardware setup

### 🔄 Version Control

- **Version bumped**: `3.33.0` → `3.33.1` in `platformio.ini`
- This is a **PATCH** version increment per SEMVER (bugfix, backward compatible)

---

## [Version 3.33.0] - 2025-12-29

### 🚀 TFT, NeoPixel & BOOT Improvements

- **Smooth, flicker-free TFT progress bar** on BOOT long press
- **Optimized partial redraw**: only the bar is updated, frame drawn once
- **NeoPixel purple (#8000FF) during progress**
- **Earthbeat NeoPixel restored** if BOOT released before 100%
- **Clean return to boot screen** after cancel
- **Robust state machine** for BOOT/Progress/Cancel/Final action
- **Refactored, readable, optimized code**
- **Detailed internal documentation**

#### 🛠️ Technical details
- State machine: NORMAL, PROGRESS, CANCEL, FINAL ACTION
- TFT rendering optimization (frameDrawn, lastProgress)
- Fine NeoPixel management (Earthbeat/purple)
- Code cleanup and comments

#### 📚 Documentation
- Updated: `CHANGELOG.md`, `CHANGELOG_FR.md`, `README.md`, `README_FR.md`
- SEMVER version: **3.33.0** (minor increment)

---

## [Version 3.31.1] - 2025-12-27

### 🎨 UI Enhancements

**Improved "Coming Soon" Indicator Visibility**

This patch release enhances the visual presentation of the "Coming Soon" indicator in the SD Card section of the Memory tab.

#### 🎯 Improvements

- **Enhanced Typography**: Increased font size to 1.3em for better readability
- **Visual Emphasis**: Added bold font weight for stronger visual presence
- **Centered Layout**: Text is now centered within its container for better visual balance
- **Improved Contrast**: Increased opacity to 1.0 for maximum visibility
- **Better Spacing**: Added 15px margin and 10px padding for improved visual separation

#### 📝 Technical Details

- **Modified Files**:
  - `web_src/styles.css` - Enhanced `.coming` class styling
  - `include/web_interface.h` - Updated with minified CSS

- **CSS Changes**:
  ```css
  .coming {
      color: #f5a623;
      font-size: 1.3em;        /* New: larger text */
      font-weight: bold;        /* New: emphasized */
      font-style: italic;
      text-align: center;       /* New: centered */
      opacity: 1;               /* Changed: from 0.85 */
      margin: 15px 0;           /* New: spacing */
      padding: 10px;            /* New: spacing */
  }
  ```

#### 🌍 Internationalization

- No changes to i18n keys
- Existing translation key `coming_soon` maintained:
  - English: "Coming Soon"
  - French: "Bientôt disponible"

### 🔄 Version Control

- **Version bumped**: `3.31.0` → `3.31.1` in `platformio.ini`
- This is a **PATCH** version increment per SEMVER (small UI enhancement, backward compatible)

---

## [Version 3.31.0] - 2025-12-27

### ✨ New Features

**Readable Web Interface Source Code with Automatic Minification**

This release introduces a complete restructuring of the web interface development workflow, making the HTML/CSS/JavaScript code maintainable while preserving optimal memory usage on the ESP32.

#### 🎯 Key Improvements

- **Readable Source Files**: All web interface code is now available in human-readable format in `web_src/`
  - `styles.css` - Beautified, commented CSS (~13 KB readable)
  - `app.js` - Beautified, documented JavaScript for full UI (~115 KB readable)
  - `app-lite.js` - Beautified JavaScript for ESP32 Classic lite UI (~3.8 KB readable)

- **Automatic Minification Workflow**: Two Python scripts automate the development cycle
  - `tools/extract_web_sources.py` - Extracts and beautifies code from `web_interface.h`
  - `tools/minify_web.py` - Minifies source files and updates `web_interface.h`

- **Memory Optimization Preserved**: Minified code remains embedded in firmware
  - CSS: ~26% size reduction (13 KB → 10 KB)
  - JavaScript Full: ~18% size reduction (115 KB → 94 KB)
  - JavaScript Lite: ~28% size reduction (3.8 KB → 2.8 KB)

- **Developer-Friendly**: Complete documentation in `web_src/README.md`
  - Detailed workflow instructions
  - Best practices
  - Troubleshooting guide
  - Tool usage examples

### 🔧 Technical Changes

**Project Structure**
- New directory: `web_src/` - Contains all readable web interface source files
- New directory: `tools/` - Contains Python scripts for minification workflow
- New files:
  - `web_src/README.md` - Complete workflow documentation
  - `web_src/styles.css` - Readable CSS source
  - `web_src/app.js` - Readable JavaScript source (full)
  - `web_src/app-lite.js` - Readable JavaScript source (lite)
  - `tools/extract_web_sources.py` - Extraction and beautification script
  - `tools/minify_web.py` - Minification script

**Dependencies**
- Python packages required for minification:
  - `rcssmin` - CSS minification
  - `rjsmin` - JavaScript minification
  - `jsbeautifier` - JavaScript beautification
  - `cssbeautifier` - CSS beautification

**Workflow**
1. Edit readable source files in `web_src/`
2. Run `python tools/minify_web.py` to update `include/web_interface.h`
3. Compile and upload firmware as usual

### 📝 Migration Notes

- **No breaking changes**: Existing `web_interface.h` continues to work exactly as before
- **New development workflow**: Developers should now edit source files in `web_src/` instead of `web_interface.h`
- **One-time setup**: Install Python dependencies: `pip install rcssmin rjsmin jsbeautifier cssbeautifier`

### 🎯 Benefits

- **Maintainability**: Code is now readable, properly formatted, and commented
- **Collaboration**: Multiple developers can easily understand and modify the web interface
- **Version Control**: Git diffs are meaningful and show actual code changes
- **No Performance Impact**: Minified code in firmware remains optimized
- **Flexibility**: Easy to customize the web interface without dealing with minified code
- **Documentation**: Comprehensive README guides developers through the new workflow

### 📚 Documentation Updates

- New `web_src/README.md` - Complete workflow and usage guide
- Updated project structure documentation to reflect new directories
- Added minification workflow examples and best practices

### 🔄 Version Control

- **Version bumped**: `3.30.0` → `3.31.0` in `platformio.ini`
- This is a **MINOR** version increment per SEMVER (new feature, backward compatible)

---

## [Version 3.30.0] - 2025-12-25

### ✨ New Features

**Dynamic TFT Driver Selection from Web UI (Runtime Switching)**

- **Runtime TFT Driver Switching**: Switch between ILI9341 and ST7789 TFT drivers dynamically from the Web UI without recompilation
- **No Reboot Required**: Change the active TFT driver instantly without rebooting the ESP32
- **Web UI Integration**: New driver selector dropdown in the TFT configuration section
- **Dual Driver Support**: Both ILI9341 and ST7789 drivers are now loaded simultaneously and can be switched on-the-fly
- **Backward Compatibility**: Default driver selection from `config.h` is preserved for initial boot

### 🔧 Technical Changes

**Backend Architecture**
- `include/tft_display.h`: Complete refactoring to support runtime driver switching
  - New enum `TFT_DriverType` for driver identification
  - Dynamic driver initialization with `initTFT(driverType, width, height, rotation)`
  - New `switchTFTDriver()` function for on-the-fly driver changes
  - Proper driver deinitialization with `deinitTFT()`
  - Generic `Adafruit_GFX*` pointer for unified driver access

**API Enhancements**
- `src/main.cpp`:
  - `handleTFTConfig()`: New `driver` parameter for dynamic driver switching
  - `handleScreensInfo()`: Now returns current active driver type
  - New global variable `tftDriver` (String) to track active driver

**Web Interface**
- `include/web_interface.h`:
  - New driver selector dropdown in TFT configuration section
  - `configTFT()` function updated to send driver parameter
  - Driver selection persisted and displayed in UI

**Configuration**
- `include/config.h`: Updated comments to reflect dynamic driver support
- `platformio.ini`: Version bumped to 3.30.0

### 📝 Migration Notes

- **No breaking changes**: Existing configurations continue to work as before
- **New capability**: Users can now test different TFT drivers without reflashing firmware
- **Default behavior**: The driver specified in `config.h` (`TFT_USE_ILI9341` or `TFT_USE_ST7789`) is used at boot

### 🎯 Use Cases

- **Hardware Testing**: Quickly test compatibility with different TFT controllers
- **Display Swapping**: Change TFT screens without recompiling and reflashing
- **Prototyping**: Evaluate different display controllers in real-time
- **Troubleshooting**: Switch drivers to identify hardware/software issues

---

## [Version 3.29.0] - 2025-12-25

### ✨ Nouveautés

**Support dynamique des écrans TFT ILI9341 et ST7789**

- Ajout de la sélection du contrôleur d'écran (ILI9341 ou ST7789) dans `config.h` via la macro `TFT_CONTROLLER`.
- La résolution de l'écran TFT est désormais configurable dans `config.h`.
- L'affichage est strictement identique quel que soit le contrôleur sélectionné.
- Documentation et guides mis à jour pour refléter ce changement.

**Breaking change :**
- Pour changer de contrôleur ou de résolution, modifiez simplement `TFT_CONTROLLER` et les macros associées dans `config.h`.

---
## [Version 3.28.5] - 2025-12-24

### 🐛 Bug Fixes

**Patch Release:** Fixed rotary encoder button stuck + button monitoring GPIO issues

### Fixed

#### 1. Bouton Encodeur Rotatif Reste "Pressed" ✅

**Problème :**
- Après avoir appuyé sur le bouton de l'encodeur rotatif, l'état restait "Pressed" même après relâchement
- L'interface affichait toujours "Pressed" en rouge, jamais "Released"
- Impossible de voir l'état réel du bouton en temps réel

**Cause Racine :**
- `handleRotaryPosition()` retournait `rotaryButtonPressed` (variable volatile ISR)
- Variable volatile mise à `true` par ISR lors de l'appui, mais jamais remise à `false` automatiquement
- Pour le monitoring temps réel, on doit lire l'état GPIO réel, pas la variable événementielle

**Solution :**
```cpp
// src/main.cpp:3199-3203 - Nouvelle fonction pour lire GPIO réel
int getRotaryButtonGPIOState() {
  if (rotary_sw_pin < 0 || rotary_sw_pin > 48) return -1;
  return digitalRead(rotary_sw_pin);
}

// src/main.cpp:4369-4379 - Utilisation dans handleRotaryPosition()
void handleRotaryPosition() {
  // v3.28.5 fix: Read REAL GPIO state for monitoring, not ISR variable
  int buttonGPIOState = getRotaryButtonGPIOState();
  bool buttonPressed = (buttonGPIOState == LOW && buttonGPIOState != -1);

  sendJsonResponse(200, {
    jsonNumberField("position", (int32_t)rotaryPosition),
    jsonBoolField("button_pressed", buttonPressed),  // Now reads real GPIO
    jsonBoolField("available", rotaryAvailable)
  });
}
```

**Impact :**
- ✅ Bouton encodeur affiche maintenant l'état correct en temps réel
- ✅ "Pressed" (rouge) quand bouton enfoncé
- ✅ "Released" (vert) quand bouton relâché
- ✅ Mise à jour immédiate (polling 100ms)

#### 2. Monitoring Boutons BOOT/1/2 Ne Fonctionne Pas ✅

**Problème :**
- Monitoring des boutons BOOT, Button 1, Button 2 ne fonctionnait toujours pas
- États ne se mettaient pas à jour malgré correction v3.28.4
- Boutons restaient bloqués sur "Released"

**Cause Racine :**
- Fonctions utilisaient variables `static` (`buttonBootPin`, `button1Pin`, `button2Pin`)
- Problème potentiel de visibilité ou d'initialisation des variables statiques
- GPIO peut-être pas correctement accessible via ces variables

**Solution :**
```cpp
// src/main.cpp:3182-3199 - Lecture directe des constantes
// v3.28.5: Use constants directly to ensure correct pin access
int getButtonBootState() {
  // Use constant directly instead of static variable
  if (BUTTON_BOOT < 0 || BUTTON_BOOT > 48) return -1;
  return digitalRead(BUTTON_BOOT);
}

int getButton1State() {
  if (BUTTON_1 < 0 || BUTTON_1 > 48) return -1;
  return digitalRead(BUTTON_1);
}

int getButton2State() {
  if (BUTTON_2 < 0 || BUTTON_2 > 48) return -1;
  return digitalRead(BUTTON_2);
}

// src/main.cpp:4420-4428 - handleButtonState() utilise constantes
if (buttonParam == "boot") {
  state = getButtonBootState();
  pin = BUTTON_BOOT;  // v3.28.5: Use constant directly
} else if (buttonParam == "1" || buttonParam == "button1") {
  state = getButton1State();
  pin = BUTTON_1;
} else if (buttonParam == "2" || buttonParam == "button2") {
  state = getButton2State();
  pin = BUTTON_2;
}
```

**Impact :**
- ✅ Monitoring BOOT (GPIO 0) fonctionne maintenant
- ✅ Monitoring Button 1 (GPIO 38/34) fonctionne
- ✅ Monitoring Button 2 (GPIO 39/35) fonctionne
- ✅ États se mettent à jour en temps réel
- ✅ "Pressed" (rouge gras) / "Released" (vert) correct

**Files Modified:**
- `src/main.cpp`:
  - Lines 3182-3203: Updated button state readers to use constants, added `getRotaryButtonGPIOState()`
  - Lines 4369-4379: `handleRotaryPosition()` now reads real GPIO state
  - Lines 4389-4407: `handleButtonStates()` uses constants for pin numbers
  - Lines 4420-4428: `handleButtonState()` uses constants for pins
- `platformio.ini`: Version 3.28.4 → 3.28.5

**Testing:**
1. **Encodeur Rotatif:**
   - Activer monitoring du bouton encodeur
   - Presser le bouton → "Pressed" (rouge) ✅
   - Relâcher → immédiatement "Released" (vert) ✅
   - Répéter plusieurs fois → états corrects ✅

2. **Boutons BOOT, 1, 2:**
   - Activer monitoring pour chaque bouton
   - Presser GPIO 0/38/39 → "Pressed" immédiat ✅
   - Relâcher → "Released" immédiat ✅
   - Pas de blocage sur un état ✅

---

## [Version 3.28.4] - 2025-12-24

### 🐛 Bug Fix

**Patch Release:** Fixed button monitoring not working - states stuck at "Released"

### Fixed

#### Button Monitoring Not Working ✅

**Problem:**
- Button monitoring buttons (BOOT, Button 1, Button 2) showed state always as "Released"
- Clicking "Monitor Button" had no effect - state never updated
- Frontend JavaScript was calling wrong API endpoint

**Root Cause:**
- Frontend calls `/api/button-state?button=boot` (singular) for individual button queries
- Backend only had `/api/button-states` (plural) endpoint that returns ALL buttons
- Endpoint mismatch: frontend expected individual button query, backend provided batch query
- No route handler registered for `/api/button-state` (singular)

**Solution:**
```cpp
// src/main.cpp:4395-4431 - Added individual button state handler
void handleButtonState() {
  if (!server.hasArg("button")) {
    sendActionResponse(400, false, "Missing 'button' parameter");
    return;
  }

  String buttonParam = server.arg("button");
  int state = -1;
  int pin = -1;

  if (buttonParam == "boot") {
    state = getButtonBootState();
    pin = buttonBootPin;
  } else if (buttonParam == "1" || buttonParam == "button1") {
    state = getButton1State();
    pin = button1Pin;
  } else if (buttonParam == "2" || buttonParam == "button2") {
    state = getButton2State();
    pin = button2Pin;
  } else {
    sendActionResponse(400, false, "Invalid button parameter");
    return;
  }

  // LOW = pressed (pull-up), HIGH = released
  bool pressed = (state == LOW && state != -1);
  bool available = (state != -1);

  sendJsonResponse(200, {
    jsonBoolField("pressed", pressed),
    jsonBoolField("released", !pressed && available),
    jsonBoolField("available", available),
    jsonNumberField("pin", pin),
    jsonNumberField("raw_state", state)
  });
}

// src/main.cpp:5798 - Registered route
server.on("/api/button-state", handleButtonState);
```

**API Response Format:**
```json
GET /api/button-state?button=boot
{
  "pressed": false,
  "released": true,
  "available": true,
  "pin": 0,
  "raw_state": 1
}
```

**Impact:**
- ✅ Button monitoring now works correctly
- ✅ State updates in real-time (100ms polling) when monitoring enabled
- ✅ "Pressed" shown in red bold when button pressed
- ✅ "Released" shown in green when button released
- ✅ Works for BOOT (GPIO 0), Button 1, and Button 2

**Files Modified:**
- `src/main.cpp`:
  - Lines 4395-4431: Added `handleButtonState()` handler
  - Line 5798: Registered `/api/button-state` route
- `platformio.ini`: Version 3.28.3 → 3.28.4

**Testing:**
1. Navigate to "Input Devices" page
2. Click "Monitor Button" for BOOT button
3. Press GPIO 0 (BOOT) button on ESP32 - state should change to "Pressed" (red) ✅
4. Release button - state should return to "Released" (green) ✅
5. Repeat for Button 1 and Button 2 ✅

---

## [Version 3.28.3] - 2025-12-24

### 🐛 Bug Fixes

**Patch Release:** Fixed rotary encoder initialization + Added button monitoring API

### Fixed

#### 1. Rotary Encoder Not Working Until Reset ✅

**Problem:**
- Rotary encoder did not respond to rotation or button presses after boot
- Only worked after navigating to "Input Devices" page and clicking "Test"
- Made the rotary encoder unusable for normal operation

**Root Cause:**
- `initRotaryEncoder()` was NEVER called during startup in `setup()`
- Function was only called inside `testRotaryEncoder()` which is triggered manually via web UI
- GPIO pins were not configured and interrupts were not attached at boot time

**Solution:**
```cpp
// src/main.cpp:5757-5765 - Added to setup()
// Initialize rotary encoder on startup (v3.28.3 fix)
Serial.println("Initialisation de l'encodeur rotatif...");
initRotaryEncoder();
if (rotaryAvailable) {
  Serial.printf("Encodeur rotatif OK: CLK=%d, DT=%d, SW=%d\r\n",
                rotary_clk_pin, rotary_dt_pin, rotary_sw_pin);
} else {
  Serial.println("Encodeur rotatif: non disponible ou configuration invalide");
}
```

**Impact:**
- ✅ Rotary encoder now initializes automatically on boot
- ✅ Rotation detection works immediately without manual test
- ✅ Button presses detected from power-on
- ✅ Real-time position tracking available via `/api/rotary-position`

#### 2. Button Monitoring Not Functional ✅

**Problem:**
- "Monitor Button" buttons in web UI did nothing
- No way to see real-time button state (pressed/released)
- JavaScript functions existed but backend API endpoints were missing

**Root Cause:**
- Frontend code referenced monitoring functions (`toggleBootButtonMonitoring()`, etc.)
- BUT no backend API endpoint existed to read button states in real-time
- Missing `/api/button-states` route

**Solution:**
```cpp
// src/main.cpp:3182-3196 - Added button state reader functions
int getButtonBootState() {
  if (buttonBootPin < 0 || buttonBootPin > 48) return -1;
  return digitalRead(buttonBootPin);
}

int getButton1State() {
  if (button1Pin < 0 || button1Pin > 48) return -1;
  return digitalRead(button1Pin);
}

int getButton2State() {
  if (button2Pin < 0 || button2Pin > 48) return -1;
  return digitalRead(button2Pin);
}

// src/main.cpp:4375-4393 - Added HTTP handler
void handleButtonStates() {
  int bootState = getButtonBootState();
  int button1State = getButton1State();
  int button2State = getButton2State();

  // LOW = pressed (pull-up), HIGH = released
  sendJsonResponse(200, {
    jsonBoolField("boot_pressed", bootState == LOW && bootState != -1),
    jsonBoolField("boot_available", bootState != -1),
    jsonBoolField("button1_pressed", button1State == LOW && button1State != -1),
    jsonBoolField("button1_available", button1State != -1),
    jsonBoolField("button2_pressed", button2State == LOW && button2State != -1),
    jsonBoolField("button2_available", button2State != -1),
    jsonNumberField("boot_pin", buttonBootPin),
    jsonNumberField("button1_pin", button1Pin),
    jsonNumberField("button2_pin", button2Pin)
  });
}

// src/main.cpp:5758-5759 - Registered route
server.on("/api/button-states", handleButtonStates);
```

**Impact:**
- ✅ New API endpoint `/api/button-states` returns real-time button states
- ✅ Returns JSON with pressed state for BOOT, Button 1, and Button 2
- ✅ Includes pin numbers and availability status
- ✅ Frontend monitoring can now poll this endpoint for live updates

**Files Modified:**
- `src/main.cpp`:
  - Lines 3182-3196: Added button state reader functions
  - Lines 4375-4393: Added `handleButtonStates()` HTTP handler
  - Lines 5757-5765: Initialize rotary encoder in `setup()`
  - Line 5759: Registered `/api/button-states` route
- `platformio.ini`: Version 3.28.2 → 3.28.3

**Testing:**
1. **Rotary Encoder:**
   - Power on ESP32
   - Rotate encoder immediately - position should change ✅
   - Press encoder button - should register ✅
   - Navigate to "Input Devices" - encoder already working ✅

2. **Button Monitoring:**
   - Navigate to "Input Devices" page
   - Press BOOT button (GPIO 0) - LED feedback should work ✅
   - Check `/api/button-states` endpoint - should return current states ✅

---

## [Version 3.28.2] - 2025-12-24

### 🐛 Critical Bug Fix

**Emergency Patch:** Fixed BUTTON_BOOT JavaScript error that was NOT actually fixed in 3.28.0/3.28.1

### Fixed

#### BUTTON_BOOT JavaScript ReferenceError ✅ (REALLY FIXED NOW)

**Problem:**
- `ReferenceError: BUTTON_BOOT is not defined` error still occurred on Input Devices page
- Despite attempts to fix in v3.28.0, the error persisted
- Root cause misidentified in previous versions

**Root Cause:**
- GPIO constants (BUTTON_BOOT, BUTTON_1, BUTTON_2, TFT_MISO_PIN) were injected in `web_interface.h` but NOT in `main.cpp:handleJavaScriptRoute()`
- The actual JavaScript served to the browser comes from `handleJavaScriptRoute()`, not from `web_interface.h:generateJavaScript()`
- `generateJavaScript()` is only used to calculate JavaScript size for statistics
- Therefore, the constants injected in `web_interface.h` were never actually sent to the browser

**Solution:**
```cpp
// src/main.cpp:5397-5405 - Added to handleJavaScriptRoute()
// Button pins
pinVars += ";const BUTTON_BOOT=";
pinVars += String(BUTTON_BOOT);
pinVars += ";const BUTTON_1=";
pinVars += String(BUTTON_1);
pinVars += ";const BUTTON_2=";
pinVars += String(BUTTON_2);
// TFT MISO pin
pinVars += ";const TFT_MISO_PIN=";
pinVars += String(TFT_MISO);
```

**Impact:**
- ✅ Input Devices page now loads WITHOUT JavaScript errors
- ✅ BUTTON_BOOT displays correctly as read-only GPIO 0
- ✅ BUTTON_1 and BUTTON_2 function correctly
- ✅ All GPIO constants now properly injected BEFORE JavaScript functions execute

**Files Modified:**
- `src/main.cpp` (lines 5397-5415): Added button and TFT MISO constants to pinVars
- `platformio.ini`: Version 3.28.1 → 3.28.2

**Testing:**
- Navigate to "Input Devices" page - should load without errors ✅
- BUTTON_BOOT should show "GPIO 0 (non configurable)" ✅
- Browser console should show no ReferenceError ✅

---

## [Version 3.28.1] - 2025-12-24

### 🐛 Critical Bug Fixes

**Patch Release:** Fixed MISO backend integration + Fixed SD card on ESP32-S3

**NOTE:** BUTTON_BOOT error was NOT fully fixed in this version despite documentation claiming it was. See v3.28.2 for actual fix.

### Fixed
- **TFT MISO Backend Integration**:
  - Fixed MISO field missing from `/api/screens-info` JSON response
  - Added `tftMISO` variable initialization from `TFT_MISO` constant
  - Backend now properly returns `tft.pins.miso` field (GPIO 13 for ESP32-S3)
  - Resolves "MISO: undefined" display issue in web UI

- **TFT MISO Configuration Sync**:
  - Fixed `configTFT()` JavaScript function not sending MISO value to backend
  - MISO parameter now properly included in `/api/tft-config` request
  - Backend `handleTFTConfig()` now accepts and validates MISO parameter
  - Completes full MISO configuration flow: UI ↔ API ↔ Firmware

- **SD Card Support on ESP32-S3**:
  - Fixed SD card initialization failing on ESP32-S3 with compilation/runtime errors
  - Root cause: `HSPI` constant only available on ESP32 classic, not ESP32-S2/S3
  - Implemented conditional SPI bus selection:
    - ESP32 classic: `HSPI` (Hardware SPI bus 2)
    - ESP32-S2/S3: `FSPI` (Flexible SPI bus, equivalent to SPI2)
  - SD card tests now fully functional on ESP32-S3 N16R8

### Technical Details
- **Backend Changes** (`src/main.cpp`):
  - Line 261: Added `int tftMISO = TFT_MISO;` variable declaration
  - Line 4568: Added `miso` field to TFT pins JSON in `handleScreensInfo()`
  - Lines 3814-3828: Updated `handleTFTConfig()` to accept and validate MISO parameter
  - Lines 2950-2954: Added conditional SPI bus selection for SD card initialization
  - Response JSON now includes: `"pins":{"miso":13,"mosi":11,...}`

- **Frontend Changes** (`include/web_interface.h`):
  - Line 119: Updated `configTFT()` to retrieve MISO value from input field
  - Added `const miso=document.getElementById('tftMISO').value;`
  - API call now includes: `?miso=${miso}&mosi=${mosi}&...`

### Compliance
- Maintains `board_config.h` immutability - all values sourced from constants
- No hardcoded GPIO values
- Proper SPI bus abstraction for ESP32 variant compatibility

### Files Modified
- `src/main.cpp`: MISO variable, JSON response, config handler, SD SPI bus
- `include/web_interface.h`: configTFT MISO parameter
- `platformio.ini`: Version 3.28.0 → 3.28.1

---

## [Version 3.28.0] - 2025-12-23

### 🚀 New Features & Bug Fixes

**Major Improvements:** Fixed BUTTON_BOOT JavaScript error + Added TFT MISO pin configuration + Added SD card test API endpoints + GPIO 13 sharing warning

### Fixed
- **BUTTON_BOOT JavaScript Error**:
  - Fixed `ReferenceError: BUTTON_BOOT is not defined`
  - Injected all missing pin constants from `board_config.h` into JavaScript
  - Added: `ROTARY_CLK_PIN`, `ROTARY_DT_PIN`, `ROTARY_SW_PIN`, `BUTTON_BOOT`, `BUTTON_1`, `BUTTON_2`
  - Added: `SD_MISO_PIN`, `SD_MOSI_PIN`, `SD_SCLK_PIN`, `SD_CS_PIN`, `TFT_MISO_PIN`
  - All GPIO definitions now properly sourced from `board_config.h` (immutable contract)

- **BUTTON_BOOT Configuration**:
  - Made BUTTON_BOOT non-configurable per specifications
  - Changed from editable input to read-only display
  - Marked as "(non configurable)" in UI to prevent accidental modification
  - Preserves native ESP32 boot button integrity

### Added
- **TFT MISO Pin Configuration**:
  - Added MISO pin to SPI pins display: `MISO`, `MOSI`, `SCLK`, `CS`, `DC`, `RST`
  - Added configurable MISO input field in TFT configuration section
  - Completes SPI pin management in web interface
  - Properly reflects GPIO 13 from `board_config.h` for ESP32-S3

- **SD Card Test API Endpoints**:
  - `/api/sd-test-read`: Test SD card read operations
  - `/api/sd-test-write`: Test SD card write operations with timestamp
  - `/api/sd-format`: Clean SD card test files (safe cleanup, not low-level format)
  - JSON response format consistent with existing endpoints
  - Automatic SD initialization if unavailable

- **GPIO 13 Sharing Warning**:
  - Added visible warning in SD Card section (yellow warning box)
  - Alerts users that GPIO 13 is shared between TFT and SD (MISO line)
  - Emphasizes need for strict SPI wiring and proper software management
  - New i18n keys: `gpio_shared_warning`, `gpio_13_shared_desc` (EN/FR)

### Technical Details
- **Web Interface** (`include/web_interface.h`):
  - Pin injection now includes all ROTARY, BUTTON, SD, and TFT pins
  - BOOT button displayed as read-only with visual indicator
  - GPIO 13 warning styled with Bootstrap-like alert styling
  - Full i18n support maintained for all new features

- **API Implementation** (`src/main.cpp`):
  - `handleSDTestRead()`: Creates test file if needed, tests read capability
  - `handleSDTestWrite()`: Tests write capability with timestamped data
  - `handleSDFormat()`: Removes all test files (`/test_*.txt`)
  - Proper error handling for unavailable SD cards

- **Translations** (`include/languages.h`):
  - `gpio_shared_warning`: "Shared GPIO 13 (TFT + SD – MISO)" / "GPIO 13 partagé (TFT + SD – MISO)"
  - `gpio_13_shared_desc`: Full explanation in EN/FR about SPI sharing requirements

### Compliance
- All GPIO definitions sourced from `board_config.h` (immutable contract)
- No hardcoded pin values in JavaScript
- Respects `board_config.h` as single source of truth
- No modifications to `board_config.h` itself (as required)

### Files Modified
- `include/web_interface.h`: Pin injection, BOOT button display, GPIO warning, MISO field
- `include/languages.h`: Added 2 new translation keys for GPIO warning
- `src/main.cpp`: Added 3 new SD card endpoint handlers + route registration
- `platformio.ini`: Version 3.27.2 → 3.28.0

---

## [Version 3.27.2] - 2025-12-23

### 🔧 Fixes & Enhancements

**Bug Fixes & New Features:** Fixed HW-040 rotary encoder button monitoring + Added 3 hardware button monitors (BOOT, BUTTON1, BUTTON2).

### Fixed
- **HW-040 Rotary Encoder Monitoring**:
  - Fixed button state not updating to "Released" after being "Pressed"
  - Replaced hardcoded text with i18n translations (`button_pressed`, `button_released`)
  - Monitoring now properly alternates between "Monitor" and "Stop" button labels
  - Real-time state updates now use proper translation functions

### Added
- **Hardware Button Monitoring** (3 new cartridges in Input Devices menu):
  - **BOOT Button** (GPIO 0): Built-in boot button monitoring with configurable pin
  - **User Button 1** (GPIO 38/5): Programmable button with internal pull-up
  - **User Button 2** (GPIO 39/12): Programmable button with internal pull-up
- **New i18n Keys** (12 additions):
  - `button_boot`, `button_boot_desc`, `button_1`, `button_1_desc`, `button_2`, `button_2_desc`
  - `button_pin`, `button_state`, `button_pressed`, `button_released`
  - `monitor_button`, `stop_monitoring`
- **JavaScript Functions**:
  - `toggleBootButtonMonitoring()`: Real-time BOOT button state monitoring
  - `toggleButton1Monitoring()`: Real-time Button 1 state monitoring
  - `toggleButton2Monitoring()`: Real-time Button 2 state monitoring
  - `applyButtonConfig(buttonId)`: Configure button GPIO pins via API

### Technical Details
- Each button cartridge includes:
  - Pin configuration with validation (min=0, max=48)
  - Real-time state display (color-coded: green=Released, red=Pressed)
  - Monitoring toggle button (100ms polling interval)
  - API integration: `/api/button-state?button=<boot|1|2>` and `/api/button-config`
- Button pins sourced from `board_config.h`: `BUTTON_BOOT`, `BUTTON_1`, `BUTTON_2`
- Fully bilingual (English/French) with proper i18n integration

### Files Modified
- `include/languages.h`: Added 12 new translation keys
- `include/web_interface.h`:
  - Fixed `toggleRotaryMonitoring()` to use `tr()` functions
  - Added 3 button monitoring functions
  - Updated `buildInputDevices()` with 3 button cartridges
- `platformio.ini`: Version 3.27.1 → 3.27.2

---

## [Version 3.27.0] - 2025-12-23

### ✨ UI Reorganization & Internationalization

**Major Refactoring:** Complete internationalization of SD Card and Rotary Encoder features + new menu structure.

### Added
- **New Navigation Menus**:
  - "Input Devices" menu for buttons, encoders, and user input controls
  - "Memory" menu for SD Card, Flash, SRAM, and PSRAM information
  - Improved menu organization: Overview → Display & Signal → Sensors → Input Devices → Memory → Hardware Tests → Wireless → Performance → Export
- **Complete Internationalization** (45+ new translation keys):
  - SD Card: `sd_card`, `sd_card_desc`, `sd_pins_spi`, `sd_pin_miso/mosi/sclk/cs`, `test_sd`, etc.
  - Rotary Encoder: `rotary_encoder`, `rotary_encoder_desc`, `rotary_pins`, `rotary_position`, `rotary_button`, etc.
  - Memory section: `memory_section`, `memory_intro`, `internal_sram`, `psram_external`, `flash_type/speed`
  - Input devices: `input_devices_section`, `input_devices_intro`
- **Language Support**: All new features now support English/French translations via `languages.h`

### Changed
- Replaced all hardcoded French text with i18n variables
- Menu buttons reorganized with new `nav_input_devices` and `nav_memory` entries
- Prepared infrastructure for logical hardware categorization

### Known Limitations
- Input Devices and Memory menus currently show placeholder content
- Full implementation of `buildMemory()` and `buildInputDevices()` pending
- SD Card and Rotary Encoder still appear in Sensors menu (migration pending)

---

## [Version 3.26.4] - 2025-12-23

### Fixed
- **UI Input Width**: Increased GPIO input field width from 50px to 70px for SD Card and Rotary Encoder
  - Fixes display issue where only 1 digit was visible
  - Allows proper input and display of 2-digit GPIO numbers (0-48)
  - Affects all SD pins (MISO, MOSI, SCLK, CS) and Rotary pins (CLK, DT, SW)

---

## [Version 3.26.3] - 2025-12-23

### Fixed
- **ESP32 Classic Support**: Added missing SD Card GPIO defines for `TARGET_ESP32_CLASSIC` in `board_config.h`
  - `SD_MISO = 19`, `SD_MOSI = 23`, `SD_SCLK = 18`, `SD_CS = 5`
  - Fixes incorrect GPIO values (1,1,1,1) displayed on ESP32 classic boards
  - Added `min="0" max="48"` attributes to all SD and Rotary Encoder GPIO inputs in web UI
- **GPIO Configuration**: Now supports full GPIO range (0-48) for all ESP32 variants

---

## [Version 3.26.2] - 2025-12-23

### Added
- **Debug Console Output**: Added browser console.log to verify GPIO pin injection
  - Displays all SD and Rotary Encoder GPIO values: `{SD_MISO: 14, SD_MOSI: 11, ...}`
  - Helps verify that `board_config.h` values are properly injected into JavaScript
  - Useful for troubleshooting GPIO configuration issues

---

## [Version 3.26.1] - 2025-12-23

### Fixed
- **Dynamic GPIO Injection**: Fixed SD Card and Rotary Encoder GPIO values not being dynamically injected
  - Added `SD_MISO_PIN`, `SD_MOSI_PIN`, `SD_SCLK_PIN`, `SD_CS_PIN` JavaScript variables in `/api/pin-vars`
  - Added `ROTARY_CLK_PIN`, `ROTARY_DT_PIN`, `ROTARY_SW_PIN` JavaScript variables
  - Replaced hardcoded values in web interface with dynamic variables
  - GPIO values now properly initialize from `board_config.h` defines

---

## [Version 3.26.0] - 2025-12-23

### ✨ New Features: SD Card and Rotary Encoder HW-040 Support

**Major Addition:** Full integration of SD Card reader and Rotary Encoder HW-040 with runtime GPIO configuration.

### Added - SD Card Support
- **SD Card Management**: Complete SPI-based SD card initialization and testing
  - Auto-detection of card type (MMC, SDSC, SDHC)
  - Card size detection and capacity reporting
  - Read/write verification tests
  - Support for totalBytes() and usedBytes() reporting
- **Runtime GPIO Configuration**:
  - `sd_miso_pin`, `sd_mosi_pin`, `sd_sclk_pin`, `sd_cs_pin` (modifiable via web UI)
  - Default values from `board_config.h`: `SD_MISO`, `SD_MOSI`, `SD_SCLK`, `SD_CS`
- **API Endpoints**:
  - `/api/sd-config` - Configure SD card pins
  - `/api/sd-test` - Run SD card test with read/write verification
  - `/api/sd-info` - Get SD card information (type, size, usage)
- **Test Functions**:
  - `initSD()` - Initialize SD card with SPI configuration
  - `testSD()` - Complete test including file write/read verification
  - `getSDInfo()` - Retrieve detailed SD card information
  - Async test support via `sdTestRunner`

### Added - Rotary Encoder HW-040 Support
- **Rotary Encoder Management**: Full interrupt-based encoder with button support
  - Hardware debouncing for rotation (5ms) and button (50ms)
  - Position tracking with increment/decrement detection
  - Button press detection with auto-reset
- **Runtime GPIO Configuration**:
  - `rotary_clk_pin`, `rotary_dt_pin`, `rotary_sw_pin` (modifiable via web UI)
  - Default values from `board_config.h`: `ROTARY_CLK`, `ROTARY_DT`, `ROTARY_SW`
- **ISR Implementation**:
  - `rotaryISR()` - IRAM interrupt handler for rotation detection
  - `rotaryButtonISR()` - IRAM interrupt handler for button presses
  - Quadrature encoding for accurate position tracking
- **API Endpoints**:
  - `/api/rotary-config` - Configure encoder pins
  - `/api/rotary-test` - Run 5-second interactive test
  - `/api/rotary-position` - Get current position and button state
  - `/api/rotary-reset` - Reset position counter to zero
- **Test Functions**:
  - `initRotaryEncoder()` - Initialize with interrupt attachment
  - `testRotaryEncoder()` - Interactive 5-second test
  - `getRotaryPosition()`, `getRotaryButtonState()`, `resetRotaryPosition()`
  - Async test support via `rotaryTestRunner`

### Changed - board_config.h
- **ESP32-S3 DevKitC-1 N16R8**:
  - SD Card pins: MISO=14, MOSI=11, SCLK=12, CS=1
  - Rotary Encoder pins: CLK=47, DT=45, SW=40
- **ESP32 Classic DevKitC**:
  - Rotary Encoder pins: CLK=4, DT=13, SW=26
  - (SD card shares pins with TFT on classic variant)

### Changed - Exports
- **TXT Export**: Added SD Card and Rotary Encoder test results
- **JSON Export**: Added `sd_card` and `rotary_encoder` fields in `hardware_tests`
- **CSV Export**: Added SD Card and Rotary Encoder test result rows

### Technical
- **Includes Added**: `<SPI.h>`, `<SD.h>`, `<FS.h>` for SD card support
- **Global Variables**:
  - SD: `sdTestResult`, `sdAvailable`, `sdCardSize`, `sdCardType`, `sdCardTypeStr`
  - Rotary: `rotaryPosition` (volatile), `rotaryButtonPressed` (volatile), interrupt timestamps
- **Stack Allocation**:
  - SD test: 6144 bytes (file I/O operations)
  - Rotary test: 4096 bytes (interactive test loop)
- **Hardware Notes**:
  - SD Card requires proper SPI wiring, shares bus with TFT on ESP32-S3
  - Rotary Encoder HW-040: 10nF capacitors recommended for hardware debouncing
  - All pins use 3.3V logic levels

### Architecture
- Follows existing runtime GPIO remapping pattern (lowercase variables)
- Async test runners for non-blocking web UI
- Consistent API response format with `success`, `result`, `available` fields
- ISR functions marked with `IRAM_ATTR` for interrupt safety

---

## [Version 3.25.1] - 2025-12-22

### Changed
- Patch version bump: updated version to 3.25.1 in platformio.ini and documentation files.
- Minor maintenance and documentation alignment for release process.

## [Version 3.25.0] - 2025-12-22

### ✅ Feature Restoration: Dynamic GPIO Pin Remapping via Web UI

**Major Change:** Reintroduced runtime pin remapping with improved architecture that avoids preprocessor conflicts.

### Added
- **Runtime Pin Variables with Lowercase Names**: New architecture uses lowercase variable names to avoid preprocessor conflicts
  - Example: `int i2c_sda = I2C_SDA;` (runtime variable) initialized from `#define I2C_SDA 15` (compile-time constant)
  - Lowercase names (`i2c_sda`, `rgb_led_pin_r`, etc.) prevent macro expansion conflicts
  - Variables declared in `src/main.cpp:201-216`
  - External declarations in `include/web_interface.h:24-35`

### Restored
- **Dynamic Pin Remapping**: Web UI can now modify GPIO pins at runtime (functionality restored from v3.23.x)
  - `handleOLEDConfig()` - I2C pin remapping for OLED/sensors
  - `handleRGBLedConfig()` - RGB LED pin remapping
  - `handleBuzzerConfig()` - Buzzer pin remapping
  - `handleDHTConfig()` - DHT sensor pin remapping
  - `handleLightSensorConfig()` - Light sensor pin remapping
  - `handleDistanceSensorConfig()` - Distance sensor pin remapping
  - `handleMotionSensorConfig()` - Motion sensor pin remapping

### Changed
- **All GPIO Pin References**: Systematically replaced UPPERCASE defines with lowercase runtime variables throughout codebase
  - `src/main.cpp`: ~100+ references updated in test functions, handlers, and JavaScript injection
  - `src/environmental_sensors.cpp`: I2C pin references updated
  - Maintains compile-time defines in `board_config.h` (UPPERCASE) as initial values

### Architecture
**Two-Layer GPIO System Restored (with improved naming)**:
1. **Compile-time defaults** (`board_config.h`): `#define I2C_SDA 15` (UPPERCASE)
2. **Runtime variables** (`main.cpp`): `int i2c_sda = I2C_SDA;` (lowercase)
3. **Key Improvement**: Different naming conventions prevent preprocessor conflicts

### Benefits
- ✅ **Dynamic remapping works**: Users can change pins via Web UI without recompilation
- ✅ **No preprocessor conflicts**: Lowercase runtime variables don't trigger macro expansion
- ✅ **Cleaner architecture**: Naming convention clearly distinguishes compile-time vs runtime
- ✅ **All functionality preserved**: No features lost compared to v3.23.x

### Technical
- **Backward Compatibility**: ⚠️ Requires firmware update from v3.24.0
- **Hardware**: No hardware changes required
- **Files Modified**:
  - `src/main.cpp`: Added runtime variables, restored handlers, updated all pin references
  - `include/web_interface.h`: Added extern declarations for runtime variables
  - `src/environmental_sensors.cpp`: Updated to use lowercase runtime variables
  - `platformio.ini`: Version bump to 3.25.0

---

## [Version 3.24.0] - 2025-12-22 (REVERTED)

### 🔄 Major Architectural Change: Simplified GPIO Pin System (REVERTED IN v3.25.0)

**Breaking Change:** GPIO pins were compile-time constants. Dynamic pin remapping via Web UI was removed.

### Changed
- **GPIO Architecture Simplified**: Removed two-layer pin system
  - Eliminated `DEFAULT_` prefix from all GPIO pin names in `board_config.h`
  - Removed runtime pin variables from `main.cpp` (lines 198-217)
  - GPIO pins are now accessed directly as `#define` constants
  - Example: `RGB_LED_PIN_R` instead of `DEFAULT_RGB_LED_PIN_R` + `int RGB_LED_PIN_R`

- **Web Interface Behavior**:
  - Pin configuration handlers now ignore changes (commented out assignments)
  - Web UI displays current pins for reference only
  - To change pins, users must edit `board_config.h` and recompile

- **Code Changes**:
  - `src/main.cpp`: Removed pin variable declarations, updated handlers
  - `include/web_interface.h`: Removed `extern` pin declarations
  - `src/environmental_sensors.cpp`: Removed `extern` declarations, uses defines directly

### Removed
- **Runtime Pin Remapping**: Web UI can no longer modify GPIO pins at runtime
- **`DEFAULT_` Prefix**: All GPIO pins now use direct names (e.g., `I2C_SDA` not `DEFAULT_I2C_SDA`)
- **Runtime Variables**: No more `int I2C_SDA = DEFAULT_I2C_SDA;` pattern

### Documentation
- **Updated `docs/PIN_POLICY.md`**: Reflects new compile-time constant architecture
- **Updated `docs/PIN_POLICY_FR.md`**: French documentation updated
- Removed references to runtime remapping and `DEFAULT_` prefix

### Benefits
- ✅ **Simpler codebase**: One-layer GPIO system is easier to understand
- ✅ **Better performance**: Compiler can optimize constant pin access
- ✅ **Clearer intent**: Pin assignments are fixed at compile time
- ✅ **No preprocessor conflicts**: Eliminates name collision issues

### Migration Guide
**For Users:**
- Pin changes now require editing `board_config.h` and recompiling
- No functional changes if using default pin assignments

**For Developers:**
- Replace `DEFAULT_GPIO_NAME` with `GPIO_NAME` in `board_config.h`
- Remove runtime variable declarations
- Access pins directly via defines

### Technical
- **Backward Compatibility**: ⚠️ Breaking change - requires firmware update
- **Hardware**: No hardware changes required
- **Files Modified**:
  - `src/main.cpp`: Pin variable removal, handler updates
  - `include/web_interface.h`: Extern declaration removal
  - `src/environmental_sensors.cpp`: Direct define usage
  - `include/board_config.h`: Removed `DEFAULT_` prefixes (already done by user)
  - `docs/PIN_POLICY.md`, `docs/PIN_POLICY_FR.md`: Documentation updates
  - `platformio.ini`: Version bump to 3.24.0

---

## [Version 3.23.2] - 2025-12-22 (DEPRECATED)

### Fixed
- **Environmental Sensors I2C Initialization**: Corrected I2C pin references in environmental sensors
  - Fixed `environmental_sensors.cpp:56-58` to use runtime variables `I2C_SDA` and `I2C_SCL`
  - Previously referenced `DEFAULT_I2C_SDA` and `DEFAULT_I2C_SCL` directly (compile-time defines)
  - Added `extern` declarations to access runtime variables from `main.cpp`
  - Now respects dynamic I2C pin configuration via Web UI

### Technical
- **File Changed**: `src/environmental_sensors.cpp:56-58`
- **Architecture Note**: Runtime variables (`int I2C_SDA`) and compile-time defines (`#define DEFAULT_I2C_SDA`)
  must coexist for Web UI dynamic remapping to work. Removing the `DEFAULT_` prefix causes preprocessor conflicts.
- **Impact**: Ensures environmental sensors (AHT20, BMP280) use correct I2C pins when remapped
- **Backward Compatibility**: ✅ Fully compatible with v3.23.1

---

## [Version 3.23.1] - 2025-12-22

### Fixed
- **Web UI Buzzer Pin Display**: Corrected buzzer pin input field initialization
  - Previously displayed `PWM_PIN` value instead of `BUZZER_PIN` in web interface
  - Affected function: `buildDisplaySignal()` in `web_interface.h:85`
  - Now correctly shows BUZZER_PIN value (ESP32-S3: GPIO 6, ESP32 Classic: GPIO 19)
  - PWM and Buzzer are distinct pins as defined in `board_config.h`

### Technical
- **File Changed**: `include/web_interface.h:85`
- **Impact**: Visual fix only - runtime behavior unchanged (backend already used correct pin)
- **Backward Compatibility**: ✅ Fully compatible with v3.23.0

---

## [Version 3.23.0] - 2025-12-22

### Added
- **PWM_PIN variable**: Added missing `PWM_PIN` runtime variable in `main.cpp`
  - Previously, `PWM_PIN` was declared as `extern` in `web_interface.h` but not defined
  - Now properly initialized from `DEFAULT_PWM_PIN` in `board_config.h`
  - ESP32-S3: PWM on GPIO 20, Buzzer on GPIO 6
  - ESP32 Classic: PWM on GPIO 4, Buzzer on GPIO 19
- **Pin Policy Documentation**: New comprehensive guides for GPIO management
  - `docs/PIN_POLICY.md` (English) - Complete pin mapping policy for developers
  - `docs/PIN_POLICY_FR.md` (French) - Guide détaillé de la politique de mapping GPIO
  - Explains the "single source of truth" principle (`board_config.h`)
  - Includes safety considerations, naming conventions, and practical examples

### Changed
- **JavaScript Injection**: Corrected PWM_PIN and BUZZER_PIN injection in Web UI
  - Both pins are now properly injected into JavaScript constants
  - Previously, `PWM_PIN` was incorrectly assigned the value of `BUZZER_PIN`
  - Affected files: `main.cpp:4812-4815`, `web_interface.h:456-459`
- **NEOPIXEL_PIN Unification**: Eliminated `DEFAULT_NEOPIXEL_PIN` redefinition
  - Removed duplicate definition from `config.h` and `config-example.h`
  - Now uses `NEOPIXEL_PIN` directly from `board_config.h` (GPIO 48 for ESP32-S3)
  - Comments added to clarify that `NEOPIXEL_PIN` is defined in `board_config.h`

### Fixed
- **Pin Mapping Consistency**: All GPIO references now exclusively use `board_config.h`
  - Eliminated ambiguity between `DEFAULT_NEOPIXEL_PIN` and `NEOPIXEL_PIN`
  - Separated `PWM_PIN` and `BUZZER_PIN` properly (they are distinct pins)
  - Improved runtime pin variable comments to reference `board_config.h` as source

### Technical
- **Backward Compatibility**: ✅ Fully compatible with v3.22.1
  - No hardware changes required
  - Web UI now correctly displays both PWM and Buzzer pins
  - All existing functionality preserved

### Documentation
- New developer guide explaining GPIO mapping architecture
- Clarifies the difference between `PIN_*` (fixed) and `DEFAULT_*` (runtime-configurable)
- Provides step-by-step examples for adding new sensors
- Available in English and French

---

## [Version 3.22.0] - 2025-01-11

### Added
- MQTT Publisher integration for real-time metrics
- Non-blocking MQTT client with auto-reconnect
- REST API endpoints for MQTT control (/api/mqtt/status, /api/mqtt/enable, /api/mqtt/test)
- Comprehensive MQTT documentation (EN/FR)
- MQTT configuration examples for popular brokers
- MQTT test script

### Changed
- Optimized MQTT loop to prevent UI freezing
- Reduced MQTT publish interval to 60 seconds
- Implemented rotating metric publication (2-3 metrics per cycle)

### Technical
- Added PubSubClient@2.8.0 library
- Memory footprint: 18KB (safe for 4MB ESP32 Classic)
- MQTT disabled by default (ENABLE_MQTT_BRIDGE=false)

## [Version 3.22.1] - 2025-12-12

### Fixed — Duplicate/obsolete pin mapping (ESP32 Classic)
1. Buttons corrected: `BTN1` 32 → 2, `BTN2` 33 → 5 (internal pull-up)
2. RGB LED aligned: `R=13`, `G=26` (was 14), `B=33` (was 25)
3. Sensors harmonized:
   - `DHT` 32 → 15
   - `HC-SR04` TRIG 27 → 12, ECHO 33 → 35
   - `PWM` on 4; `Buzzer` on 19

Reference: `include/board_config.h` is the single source of truth for pin mappings.

### Changed - ESP32-S3 Pin Mapping (v3.22.0)
- **RGB LED relocated**: Green 45 → 41, Blue 47 → 42
- **Rationale**: Free strapping pins to avoid boot conflicts
- **Red unchanged**: GPIO 21 remains
  - **Green heartbeat** (0, 50, 0) / (0, 10, 0): Successfully connected to Wi-Fi
  - **Red heartbeat** (50, 0, 0) / (10, 0, 0): Wi-Fi disconnected
  - **Heartbeat frequency**: 1 Hz (alternates every 1 second)
  - **Non-blocking operation**: Managed in main loop, no impact on responsiveness

- **BOOT Button Reboot Confirmation**: Violet LED flash on long press
  - **Violet flash (255, 0, 255)** displays immediately when holding BOOT for 2+ seconds
  - Provides clear visual confirmation of reboot request
  - Synchronized with TFT progress bar

- **Test Isolation**: NeoPixel Wi-Fi status pauses during hardware tests
  - Heartbeat automatically pauses when running `/api/neopixel-test`
  - Heartbeat automatically pauses when running `/api/neopixel-pattern`
  - Heartbeat automatically pauses during `/api/neopixel-color` changes
  - Status automatically resumes upon test/pattern completion

### Changed
- **main.cpp**: Added NeoPixel Wi-Fi state management functions
  - New functions: `updateNeoPixelWifiStatus()`, `neopixelSetWifiState()`, `neopixelShowConnecting()`, `neopixelPauseStatus()`, `neopixelResumeStatus()`, `neopixelRestoreWifiStatus()`, `neopixelShowRebootFlash()`
  - Modified `setup()`: Initialize NeoPixel before Wi-Fi connection
  - Modified `loop()`: Add heartbeat update call
  - Modified `onButtonBootLongPress()`: Add reboot flash confirmation
  - Modified NeoPixel test handlers: Add pause/resume around tests

### Technical
- **GPIO**: No changes - uses existing NeoPixel GPIO configuration (GPIO 48 ESP32-S3, GPIO 2 ESP32 Classic)
- **Timing**: Heartbeat 1 Hz update interval, non-blocking implementation
- **Memory**: Minimal overhead - 7 global state variables, no dynamic allocations in heartbeat path

### Backward Compatibility
- **? Fully compatible** with v3.21.0 - no hardware changes required
- **? No breaking changes** - purely additive feature
- **? No configuration changes** - NeoPixel GPIO unchanged from v3.21.0

---

## [Version 3.21.0] - 2025-12-09

### ?? BREAKING CHANGE - Hardware Migration Required for ESP32 Classic
- **Complete ESP32 Classic Pin Mapping Revision**: 11 changes to resolve boot issues and USB communication problems
  - **Boot Issues Resolved**: Eliminated LEDs and peripherals on strapping pins GPIO 4, 12, 15
  - **USB-UART Stability**: Protected GPIO 1 (TX0) and GPIO 3 (RX0) from interference during flashing
  - **Improved Buttons**: Migrated to GPIO 32/33 with internal pull-up (instead of 34/35 input-only without pull-up)

### Details of 11 Changes (ESP32 Classic Only)
1. **GPS PPS**: GPIO 4 ? GPIO 36 (GPIO4 = SDIO boot strapping)
2. **TFT CS**: GPIO 19 ? GPIO 27 (avoid USB-UART interference)
3. **TFT DC**: GPIO 27 ? GPIO 14 (wiring reorganization)
4. **TFT RST**: GPIO 26 ? GPIO 25 (better physical grouping)
5. **TFT BL**: GPIO 13 ? GPIO 32 (avoid internal LED conflict)
6. **RGB LED Red**: GPIO 12 ? GPIO 13 (GPIO12 = flash voltage strapping)
7. **RGB LED Blue**: GPIO 15 ? GPIO 25 (GPIO15 = JTAG debug strapping)
8. **Button 1**: GPIO 34 ? GPIO 32 (GPIO34 = input-only, no pull-up)
9. **Distance TRIG**: GPIO 32 ? GPIO 27 (GPIO32 reassignment)
10. **DHT**: GPIO 25 ? GPIO 32 (GPIO25 reassignment)
11. **Motion Sensor**: GPIO 36 removed (reassigned to GPS PPS)

### Added
- **Safety Section** in `board_config.h`: Detailed reminders on voltages (3.3V), strapping pins, input-only GPIOs (34/35/36/39), UART0 protection, current limits (=12 mA), I2C pull-up recommendations (4.7 kO)
- **Complete Documentation**: New file `docs/PIN_MAPPING_CHANGES_FR.md` detailing each change with numbering, technical rationale, and summary table

### Changed
- **All ESP32 Classic Pins** updated in `board_config.h`, `PIN_MAPPING_FR.md`, `PIN_MAPPING.md`
- **User Documentation**: CONFIG, FEATURE_MATRIX, README updated with new pins
- **Version History**: v3.21.0 entry added to all reference documents

### User Impact
- **ESP32-S3**: No changes, mapping unchanged
- **ESP32 Classic**: Hardware rewiring required according to new mapping (see table in `docs/PIN_MAPPING_CHANGES_FR.md`)
- **Dynamic Configuration**: Web interface still allows modifying certain sensor pins without recompiling

---

## [Version 3.20.4] - 2025-12-08

### Changed
- **Project Name Refactoring:** Eliminated hardcoded project name strings throughout codebase
- All occurrences of "ESP32 Diagnostic" replaced with `PROJECT_NAME` macro from platformio.ini
- Affected files: `main.cpp` (5 locations: TFT display, mDNS service registration, OLED display, HTML footer, JavaScript preamble), `languages.h` (title), `web_interface.h` (JavaScript console log)
- Project name now centrally defined in build configuration for improved maintainability
- Enables easy project customization through single build flag modification

### Technical
- No functionality changes - all features and behavior remain identical
- Improved code maintainability through centralized configuration
- Reduced technical debt from string literals scattered across multiple files

## [Version 3.20.3] - 2025-12-08

### Changed
- **Code Optimization:** Applied 9 systematic optimizations ([OPT-001] through [OPT-009]) for memory efficiency
- Eliminated 90+ string allocations through unified buffer-based approaches
- Version string formatting: snprintf-based (1 vs 11 allocations)
- Uptime formatting: buffer approach (1 vs 4-6 allocations)
- GPIO list building: optimized to O(1) allocations
- Chip features: eliminated substring operations
- Created reusable constants: DEFAULT_TEST_RESULT_STR, OK_STR, FAIL_STR
- Converted 13 debug/status messages to snprintf buffer formatting
- Optimized TextField usage in formatUptime (3 direct .str().c_str() calls)
- All test result assignments use pre-allocated constants (30+ locations)

### Technical
- No functionality changes - pins, tests, and features remain identical
- Successfully compiled on ESP32-S3 (esp32s3_n16r8) and ESP32 CLASSIC (esp32devkitc)
- Improved runtime memory efficiency through reduced heap allocations

## [Version 3.20.2] - 2025-12-07

### Changed
- **Web UI Pin References:** Hardcoded GPIO pin values in web interface replaced with dynamic variable references from `board_config.h`
   - RGB LED pins (R/G/B) now reference `RGB_LED_PIN_R`, `RGB_LED_PIN_G`, `RGB_LED_PIN_B` instead of hardcoded values
   - DHT sensor pin now references `DHT_PIN` instead of hardcoded value
   - Light sensor pin now references `LIGHT_SENSOR_PIN` instead of hardcoded value  
   - Distance sensor trigger/echo pins now reference `DISTANCE_TRIG_PIN` / `DISTANCE_ECHO_PIN` instead of hardcoded values
   - Motion sensor pin now references `MOTION_SENSOR_PIN` instead of hardcoded value
   - Buzzer/PWM pin now references `PWM_PIN` instead of hardcoded value
- Pin values are now injected as JavaScript constants at page load, ensuring UI always displays the correct compiled target pins
- **Version bump:** `PROJECT_VERSION` set to `3.20.2` in `platformio.ini`

### Fixed
- Web UI now correctly displays the actual GPIO pins based on compiled target (ESP32-S3 vs ESP32 CLASSIC)

## [Version 3.20.1] - 2025-12-07

### Fixed
- **Stabilit� USB/OTG (ESP32-S3)** : lib�ration des lignes D-/D+ (GPIO 19/20) en d�pla�ant l'I2C par d�faut sur SDA=15 / SCL=16 et la LED RGB Rouge sur GPIO 21; �limine les perturbations du bus USB provoqu�es par les anciens branchements I2C/RGB sur 19/20.

### Changed
- **Pin mapping ESP32-S3** :
   - I2C: SDA=15, SCL=16 (au lieu de 21/20)
   - RGB: Rouge=21, Vert=45, Bleu=47 (Rouge quitte 19 pour lib�rer l'USB)
   - Notes de strapping rappel�es sur GPIO45
- **Version bump** : `PROJECT_VERSION` mis � `3.20.1` dans `platformio.ini`.

### Documentation
- Guides Pin Mapping (EN/FR), README (EN/FR), et nouvelles release notes align�s sur la nouvelle attribution de broches et l'explication de la r�solution OTG.

## [Version 3.20.0] - 2025-12-07

### Added
- **Advanced Button Management:** Interactive button controls with visual feedback
  - BOOT Button (GPIO 0): Long press (2s) triggers system reboot with TFT progress bar; release before 2s clears screen
  - Button 1 (GPIO 38): Short press cycles RGB LED colors (Off ? Red ? Green ? Blue ? White)
  - Button 2 (GPIO 39): Short press triggers confirmation beep
  - Real-time progress visualization during long-press operations on TFT
  - Enhanced debouncing and long-press detection for reliable button interaction

### Changed
- **Version bump:** `PROJECT_VERSION` set to `3.20.0` in `platformio.ini`
- Button handling refactored with separate functions for long-press and short-press actions
- Improved visual feedback system using TFT display for user interactions

### Documentation
- Updated README/README_FR with button functionality descriptions and usage examples

## [Version 3.19.0] - 2025-12-07

### Changed
- **Pin mapping isolated:** Board GPIO definitions moved to `include/board_config.h`; `config.h` now keeps common/runtime options only. ESP32-S3 buttons remain on GPIO 38/39 to avoid USB upload/reset conflicts (no other pin value changes).
- **Secrets rename:** Wi-Fi credentials file renamed to `secrets.h` (with `secrets-example.h`); legacy `wifi-config` headers now emit compile-time errors.
- **Version bump:** `PROJECT_VERSION` set to `3.19.0` in `platformio.ini`; `.gitignore` explicitly protects `include/secrets.h`.

### Documentation
- Updated README/README_FR, CONFIG guides, pin mapping references, install/build checklists, FAQ, troubleshooting, security, architecture map, and usage docs to reflect `board_config.h`, new button pins, and `secrets.h`.

## [Version 3.18.3] - 2025-12-06

### Fixed
- **ESP32-S3 GPIO 48 Conflict**: Resolved hardware conflict between NeoPixel LED and RGB LED
  - NeoPixel now exclusively uses GPIO 48 (enabled, was previously disabled)
  - RGB LED Green relocated from GPIO 48 to GPIO 47
  - RGB LED Blue remains on GPIO 45 (no change)
  - RGB LED Red remains on GPIO 19 (no change)

### Changed
- **ESP32-S3 Pin Mapping Refactoring**: Complete reorganization of sensor pins to resolve conflicts
  - Motion Sensor (PIR): GPIO 6 ? GPIO 46
  - Light Sensor: GPIO 19 ? GPIO 4
  - HC-SR04 ECHO: GPIO 19 ? GPIO 6
  - NeoPixel: Enabled on GPIO 48, count changed from disabled to 1 LED
  - Unchanged: I2C (SDA=21, SCL=20), Buttons (BTN1=1, BTN2=2), GPS (RX=18, TX=17, PPS=8), TFT display pins, PWM/Buzzer (14), DHT (5), HC-SR04 TRIG (3)

### Documentation
- Updated `README.md` and `README_FR.md` with version 3.18.3 and new pin summary
- Updated `docs/PIN_MAPPING.md` and `docs/PIN_MAPPING_FR.md` with complete ESP32-S3 pin tables
- Synchronized `include/config-example.h` with definitive ESP32-S3 configuration
- Created `docs/RELEASE_NOTES_3.18.3.md` and `docs/RELEASE_NOTES_3.18.3_FR.md`

## [Version 3.18.2] - 2025-12-06

### Fixed
- **Missing Translation Keys**: Added 4 missing translation keys for GPS and environmental sensors display
  - `gps_status`: GPS status indicator in the UI
  - `temperature_avg`: Average temperature label for combined sensors
  - `pressure_hpa`: Pressure measurement label with unit
  - `altitude_calculated`: Calculated altitude from barometric pressure

## [Version 3.18.1] - 2025-12-06

### Fixed
- **AHT20 Sensor Data Reading**: Corrected bit extraction algorithm for humidity and temperature values (20-bit values were not properly extracted from 6-byte response)
- **Environmental Sensors API**: Fixed JSON structure to use flat format instead of nested objects for better web interface compatibility
- **Sensor Status Reporting**: Improved status messages to clearly indicate "OK", "Read error", or "Not detected" for each sensor

### Added
- **Missing Translation Keys**: Added missing French/English translation keys for GPS and environmental sensors UI elements
  - `refresh_gps`, `gps_module`, `gps_module_desc`
  - `refresh_env_sensors`, `test_env_sensors`

## [Version 3.18.0] - 2025-12-05

### New Features
1. **GPS Module Support**: Added full GPS receiver integration using NEO-6M/NEO-8M/NEO-M series modules.
   - NMEA sentence parsing (RMC, GGA, GSA, GSV)
   - Latitude, longitude, altitude, speed, course tracking
   - Satellite count and signal quality (HDOP, VDOP, PDOP)
   - PPS (Pulse Per Second) signal detection ready
   - Real-time data updates and diagnostic test mode
   - Uses UART1 with configurable pins in `config.h` (ESP32-S3: RX=18/TX=17/PPS=8)
   - Web API endpoint `/api/gps` for live data streaming
   - Web API endpoint `/api/gps-test` for diagnostic testing

2. **Environmental Sensors Support**: Added AHT20 (Temperature/Humidity) + BMP280 (Pressure) sensor integration.
   - AHT20: Temperature (�0.5�C) and Humidity (�3% RH) readings
   - BMP280: Atmospheric pressure (�1 hPa) with integrated temperature sensor
   - Altitude calculation from pressure measurements
   - Automatic sensor detection and dual-sensor averaging
   - Uses I2C interface with configurable pins in `config.h` (SDA/SCL pins)
   - Support for both AHT20 address (0x38) and BMP280 addresses (0x76/0x77)
   - Web API endpoint `/api/environmental-sensors` for live data streaming
   - Web API endpoint `/api/environmental-test` for diagnostic testing

3. **Web Interface Updates**:
   - GPS data card in wireless section displaying current coordinates, altitude, satellites, fix quality
   - Environmental sensors card under the existing DHT sensor section
   - Real-time data refresh with automatic status updates
   - Comprehensive error handling and sensor availability detection

### Improvements
4. Enhanced sensor module architecture for easy addition of future sensors
5. Comprehensive I2C and UART driver implementations with error handling
6. Added 24 new translation keys for GPS and environmental sensor UI elements (FR/EN)
7. Improved device detection and capability reporting

### Technical Details
8. New header files: `gps_module.h`, `environmental_sensors.h`
9. New implementation files: `gps_module.cpp`, `environmental_sensors.cpp`
10. New API endpoints in `main.cpp` for GPS and environmental sensor data
11. Extended translation dictionary in `languages.h` with GPS and environmental sensor labels

### Compatibility
- Fully compatible with ESP32-S3 DevKitC-1 N16R8 and ESP32 Classic boards
- No changes to existing pin mapping or configuration
- Backward compatible with existing diagnostics and features

## [Version 3.18.0] - 2025-12-06

### New Features
1. **GPS NEO-6M/NEO-8M Module**: Full support for GPS modules via UART1 with NMEA parsing (RMC, GGA, GSA, GSV).
   - Read latitude, longitude, altitude, speed, course
   - Signal quality (HDOP, PDOP, VDOP), satellite count
   - Optional PPS (Pulse Per Second) signal support
   - Pins configured in `config.h`: RX=18/TX=17/PPS=8 (ESP32-S3), RX=16/TX=17/PPS=4 (ESP32 Classic)
2. **Environmental Sensors AHT20 + BMP280**: I2C support for temperature, humidity, and atmospheric pressure.
   - AHT20 (address 0x38): temperature and humidity
   - BMP280 (address 0x76/0x77): temperature, pressure, and calculated altitude
   - Average temperature from both sensors for improved accuracy
   - API endpoints `/api/gps`, `/api/gps-test`, `/api/environmental-sensors`, `/api/environmental-test`
3. **Enhanced Web Interface**: GPS card in Wireless page and environmental sensors card in Sensors page.
4. **Translations**: Added 28 new FR/EN translation keys for GPS and environmental sensors.

### Technical
5. New files: `gps_module.h/.cpp`, `environmental_sensors.h/.cpp` in modular architecture
6. Automatic initialization of GPS and environmental modules at startup
7. Optimized NMEA parsing without external libraries
8. BMP280 calibration with temperature and pressure compensation

### Impact
- Minor release (3.17.1 ? 3.18.0); major new features added while maintaining backward compatibility.

## [Version 3.17.1] - 2025-12-05

### Changed
1. **ESP32-S3 pin mapping refresh:** GPS RX=18/TX=17/PPS=8; TFT MOSI=11/SCLK=12/CS=10/DC=9/RST=13/BL=7; RGB R=47/G=48/B=45; sensors updated (PWM/Buzzer=14, DHT=5, Motion=4, Light=19, HC-SR04 TRIG=3/ECHO=6); buttons stay BTN1=1/BTN2=2.
2. **ESP32 Classic pin mapping refresh:** GPS RX=16/TX=17/PPS=4; TFT MOSI=23/SCLK=18/CS=19/DC=27/RST=26/BL=13; RGB R=12/G=14/B=15; sensors updated (PWM/Buzzer=5, DHT=25, Motion=36, Light=2, HC-SR04 TRIG=32/ECHO=33); buttons BTN1=34/BTN2=35.
3. **Docs & build:** Updated README/README_FR, pin mapping guides, feature matrix, usage, build guides; bumped `PROJECT_VERSION` to `3.17.1` in `platformio.ini`.

### Impact
- Patch release (3.17.0 ? 3.17.1); functionality unchanged aside from new default pin assignments and documentation alignment.

## [Version 3.16.0] - 2025-11-28

### New Features
1. **Connected Client IP Logging**: Added automatic logging of connected client IP addresses in the Serial Monitor for better network monitoring and diagnostics.
2. **OLED Resolution Configuration**: Added ability to configure OLED screen resolution (width � height) dynamically through the web interface.
3. **TFT Configuration UI**: Added comprehensive TFT display configuration through web interface including:
   - Pin mapping configuration (MOSI, SCLK, CS, DC, RST, BL)
   - Display resolution configuration (width � height)
   - Rotation settings
4. **API Endpoint `/api/tft-config`**: New endpoint for TFT configuration with validation and real-time updates.
5. **Enhanced Screen Info API**: Updated `/api/screens-info` to include resolution and pin details for both OLED and TFT displays.

### Improvements
6. **Network Monitoring**: Client connections are now logged with format `[Client] <endpoint> connected from IP: <address>` for easier troubleshooting.
7. **Dynamic Configuration**: All display settings (OLED/TFT) can now be modified without code recompilation.

### Technical Details
8. Added global variables for runtime configuration: `oledWidth`, `oledHeight`, `tftMOSI`, `tftSCLK`, `tftCS`, `tftDC`, `tftRST`, `tftBL`, `tftWidth`, `tftHeight`, `tftRotation`.
9. Enhanced `handleOLEDConfig()` to support resolution parameters (width, height).
10. Implemented `logClientConnection()` helper function for consistent IP logging.
11. Version: Bumped from 3.15.1 to 3.16.0 following semantic versioning (new minor features).

### Web Interface Enhancements
12. **Wireless Network Monitor**: Added real-time connection status display in WiFi tab showing current IP address, SSID, gateway, DNS server, and signal strength in a dedicated info grid before the WiFi scanner.

## [Version 3.15.1] - 2025-11-27

### Bug Fixes
1. **Critical Memory Fix for ESP32 Classic**: Fixed web interface loading failure on `esp32devkitc` environment caused by heap exhaustion when serving large JavaScript files.
2. **PROGMEM Streaming**: Implemented chunked transfer (1KB blocks) for JavaScript content served from PROGMEM, eliminating large String allocations that caused crashes on boards without PSRAM.
3. **Universal Improvement**: Memory optimization benefits all environments (esp32s3_n16r8, esp32s3_n8r8, esp32devkitc) with reduced heap fragmentation during web page serving.

### Technical Details
4. Modified `handleJavaScriptRoute()` in `src/main.cpp` to stream `DIAGNOSTIC_JS_STATIC` content using `memcpy_P()` and `sendContent()` in small chunks.
5. Replaced single large `String(FPSTR(DIAGNOSTIC_JS_STATIC))` allocation with iterative chunked transfer.
6. No changes to UI functionality or user experience � purely internal optimization.

### Impact
7. **ESP32 Classic (esp32devkitc)**: Web UI now loads reliably on 4MB Flash / no PSRAM configurations.
8. **ESP32-S3 variants**: Improved memory efficiency with no regressions.
9. Version: Patch release following semantic versioning (3.15.0 ? 3.15.1).

## [Version 3.15.0] - 2025-11-27

### New Features
1. **Multi-Environment Support**: Added three distinct build environments in `platformio.ini`:
   - `esp32s3_n16r8` (default): ESP32-S3 with 16MB Flash + 8MB PSRAM (QSPI/OPI)
   - `esp32s3_n8r8`: ESP32-S3 with 8MB Flash + 8MB PSRAM
   - `esp32devkitc`: ESP32 Classic with 4MB Flash (no PSRAM)
2. **Hardware-Specific Pin Mapping**: Dedicated pin configurations in `config.h` for each target using conditional compilation (`TARGET_ESP32_S3` / `TARGET_ESP32_CLASSIC`).
3. **Shared Pin Mapping**: ESP32-S3 N8R8 and ESP32 Classic use common pin assignments where hardware allows.

### Configuration Changes
4. **ESP32-S3 Pin Mapping** (N16R8 / N8R8):
   - I2C: SDA=21, SCL=20
   - RGB LED: R=14, G=13, B=18
   - Sensors: DHT=19, Light=4, Distance TRIG=16/ECHO=17, Motion=39, Buzzer=3
   - TFT ST7789: MOSI=11, SCLK=12, CS=10, DC=9, RST=7, BL=15
   - GPS: RXD=8, TXD=5, PPS=38

5. **ESP32 Classic Pin Mapping** (DevKitC):
   - I2C: SDA=21, SCL=22
   - RGB LED: R=25, G=26, B=27
   - Sensors: DHT=4, Light=34, Distance TRIG=5/ECHO=18, Motion=36, Buzzer=13
   - TFT ST7789: MOSI=23, SCLK=18, CS=15, DC=2, RST=4, BL=32
   - GPS: RXD=16, TX=17, PPS=39
   - Buttons: BTN1=0 (BOOT), BTN2=35

### Documentation
6. Complete pin mapping reference documented in `config.h` with clear separation by target.
7. Build environment selection guide added to documentation.

### Technical
8. Version: Bumped from 3.14.1 to 3.15.0 (new minor feature: multi-environment support).
9. Compilation: Validated on all three environments with proper target-specific defines.

## [Version 3.14.0] - 2025-11-27

### New Features
1. **TFT Web Interface**: Added complete cartouche for testing TFT ST7789 display (240x240) via web interface.
2. **TFT Tests**: 8 individual tests available: boot splash, colors, shapes, text rendering, line patterns, animation, progress bar, final message.
3. **Boot Screen Button**: New button to restore startup display on both OLED and TFT.
4. **TFT REST API**: 3 new endpoints: `/api/tft-test` (full test), `/api/tft-step?step=<id>` (individual test), `/api/tft-boot` (boot screen restore).
5. **OLED REST API**: New endpoint `/api/oled-boot` to restore OLED boot screen.

### Improvements
6. Web Interface: TFT cartouche with similar structure to OLED for consistency.
7. Translations: 13 new bilingual keys (EN/FR) for TFT interface.
8. Architecture: TFT tests follow same pattern as OLED tests for maintainability.

### Technical
9. Version: Bumped from 3.13.1 to 3.14.0 (new minor feature).
10. Compilation: Validated on all three environments `esp32s3_n16r8`, `esp32s3_n8r8`, `esp32devkitc`.

## [Version 3.13.1] - 2025-11-26

### Updates
1. Dependencies: switched Adafruit libraries in `platformio.ini` to caret ranges (`^`) to allow safe minor/patch updates.
2. Build: verified clean compilation on all three environments: `esp32s3_n16r8`, `esp32s3_n8r8`, `esp32devkitc`.
3. Scope: documentation-only and build configuration maintenance; no functional firmware changes.

## [Version 3.13.0] - 2025-11-26

### Changed
- Synchronized documentation for multi-board environments: `esp32s3_n16r8`, `esp32s3_n8r8`, and `esp32devkitc`.
- Removed references to unsupported boards (ESP32-S2/C3/C6/H2) across guides and feature matrices.
- Corrected I2C default documentation (SCL=20) and kept HC-SR04 defaults (TRIG=16, ECHO=17).
- Updated `PROJECT_VERSION` to 3.13.0 in `platformio.ini`.

### Notes
- Builds validated for esp32s3_n16r8 and esp32s3_n8r8. `esp32devkitc` compilation configuration present but not hardware-tested.

# Changelog

## Version 3.17.0 - 2025-12-01
- Feature: Basic hardware button support (BTN1/BTN2) enabled via `ENABLE_BUTTONS` without changing pin mapping.
   - BTN1: short press plays a short buzzer tone for feedback.
   - BTN2: short press cycles RGB LED colors (red ? green ? blue ? white).
- Docs: Updated version references and described button behavior in FR/EN docs.
- Build: Bump `PROJECT_VERSION` to `3.17.0` in `platformio.ini`.
- No pin mapping changes; existing `PIN_BUTTON_1`/`PIN_BUTTON_2` respected per target.

All notable changes to ESP32 Diagnostic Suite are documented here. This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Version 3.12.3] - 2025-11-26
### Changed
- Default HC-SR04 pins set to `TRIG=16`, `ECHO=17` in `config.h` and aligned defaults in the web interface.

### Added
- New quick reference documents: `docs/PIN_MAPPING.md` and `docs/PIN_MAPPING_FR.md`.

### Technical Changes
- Bumped `PROJECT_VERSION` to 3.12.3 in `platformio.ini`.
- No other functional changes.

## [Version 3.12.2] - 2025-11-26
### Fixed
- **HC-SR04**: Hardened distance measurement by waiting for ECHO to settle LOW before triggering and using `pulseInLong()` with an extended timeout. Prevents false "No echo" results.

### Technical Changes
- No pin mapping changes. Only measurement sequence and timeout handling adjusted.

## [Version 3.12.1] - 2025-11-26
### Fixed
- **PSRAM Activation**: Ensured PSRAM is enabled on ESP32-S3 DevKitC-1 N16R8 in PlatformIO by adding `board_build.psram = enabled` and defining `BOARD_HAS_PSRAM`. This resolves memory tests failing to allocate in external PSRAM.

### Improved
- PlatformIO configuration refined for consistent PSRAM detection and usage across Arduino-ESP32 3.3.x.
- Updated FR/EN documentation and version references to 3.12.1.

### Technical Changes
- Bumped `PROJECT_VERSION` to 3.12.1 in `platformio.ini`.
- No pin-mapping changes (config.h remains unchanged).

## [Version 3.12.0] - 2025-11-26
### Fixed
- **CONFIG**: Validated and confirmed TFT backlight pin configuration (GPIO 15).
- Resolved duplicate TFT display declarations causing compilation warnings.
- Corrected pin mapping documentation in config.h for ESP32-S3 DevKitC-1 N16R8.

### Improved
- **DOCUMENTATION**: Removed obsolete development and debugging files for cleaner repository structure.
- Deleted temporary files: CORRECTIFS_APPLIQUES.md, CORRECTIF_v3.11.1.md, DEBUGGING_WEB_UI.md, PATCH_WEB_UI.cpp, RESUME_FINAL.md, RESUME_v3.11.1_FINAL.md.
- Consolidated pin mapping comments and organization in config.h.
- Enhanced PlatformIO configuration with improved PSRAM settings.

### Technical Changes
- Finalized TFT pin configuration for production use on ESP32-S3 N16R8.
- Cleaned repository of development artifacts and maintenance notes.
- Standardized configuration file structure and comments.
- Updated version to 3.12.0 in platformio.ini.

## [Version 3.11.4] - 2025-11-25
### Improved
- **MAINTENANCE**: Code quality improvements and cleanup.
- Removed obsolete development version history from source code headers.
- Removed unused `handleJavaScript()` function (dead code elimination).
- Simplified and standardized comment style throughout codebase.
- Fixed French typo: "defaut" ? "d�faut" in setup messages.

### Technical Changes
- Cleaned obsolete version comments (v3.8.x-dev through v3.10.3).
- Deleted `handleJavaScript()` function never referenced in routing.
- Normalized comment delimiters and removed redundant annotations.
- Updated version to 3.11.4 in platformio.ini.

## [Version 3.11.3] - 2025-11-25
### Fixed
- **CONFIG**: Corrected TFT backlight pin from GPIO 48 to GPIO 15 to resolve NeoPixel pin conflict.
- TFT display backlight now uses dedicated GPIO 15 instead of conflicting with GPIO 48 (NeoPixel).

### Technical Changes
- Updated `TFT_BL` definition in `config.h` from pin 48 to pin 15.
- Ensures proper TFT backlight operation without NeoPixel hardware conflicts.

## [Version 3.11.2] - 2025-11-25
### Fixed
- **BUILD**: Fixed FPSTR() type casting error preventing compilation.
- Corrected pointer type handling for PROGMEM strings in chunked transfer implementation.
- Changed `const char* staticJs = FPSTR(...)` to proper ESP32 PROGMEM access pattern.
- Resolved `cannot convert 'const __FlashStringHelper*' to 'const char*'` compilation error.

### Technical Changes
- Updated `handleJavaScriptRoute()` to use direct PROGMEM pointer access for verification.
- Maintained chunked transfer implementation while fixing type compatibility.
- Added comments explaining ESP32-specific PROGMEM memory mapping.

## [Version 3.11.1] - 2025-11-25
### Fixed
- **CRITICAL**: Fixed web interface JavaScript loading using chunked transfer encoding.
- Replaced monolithic JavaScript generation with memory-efficient chunked streaming.
- Enhanced debug logging to show JavaScript size breakdown (preamble, translations, static code).
- Fixed memory overflow issues when JavaScript exceeded allocated buffer size.

### Technical Changes
- Implemented chunked transfer encoding in `handleJavaScriptRoute()`.
- Separated JavaScript generation into three parts: preamble, translations, static code.
- Added PROGMEM verification to detect missing functions before sending.
- Reduced memory pressure by streaming JavaScript content instead of buffering.

## [Version 3.11.0] - 2025-11-25
### Added
- **NEW FEATURE**: TFT ST7789 display support with 240x240 resolution.
- Boot splash screen on TFT display showing system initialization.
- Real-time WiFi connection status visualization on TFT.
- IP address display on TFT once connected.
- Configurable TFT pins (MOSI, SCLK, CS, DC, RST, Backlight) in config.h.
- New tft_display.h header file for TFT display management.

### Fixed
- Web interface initialization and tab loading improvements.
- Enhanced JavaScript error handling for better UI responsiveness.

### Improved
- Better visual feedback during boot process with TFT display.
- Dual display support (OLED + TFT) for enhanced diagnostics.

## [Version 3.10.3] - 2025-11-25
### Added
- None.

### Fixed
- **CRITICAL**: Fixed `portGET_ARGUMENT_COUNT()` static assertion compilation error in FreeRTOS macros.
- Changed platform from unstable git version to stable `espressif32@6.5.0` release.
- Added `-DCONFIG_FREERTOS_ASSERT_ON_UNTESTED_FUNCTION=0` build flag to prevent FreeRTOS macro conflicts.

### Improved
- Enhanced build stability by using stable platform version instead of git repository.
- Improved compatibility with FreeRTOS and Arduino-ESP32 framework.

## [Version 3.10.2] - 2025-11-25
### Added
- None.

### Fixed
- **CRITICAL**: Fixed C++17 compilation flags causing runtime crash on startup.
- Added `build_unflags = -std=gnu++11` to properly override default C++ standard.
- Changed `-std=gnu++17` to `-std=c++17` for stricter C++17 compliance.
- Resolved inline variable initialization issues that prevented web interface and OLED display from functioning.

### Improved
- Cleaned up duplicate build flags in `platformio.ini`.
- Enhanced build system configuration for better C++17 support.

## [Version 3.9.0] - 2025-11-11
### Added
- Updated project documentation and repository references for PlatformIO-based deployment.
- Migrated from Arduino IDE to PlatformIO for improved build consistency and dependency management.

### Fixed
- None.

### Improved
- Updated all documentation to reflect PlatformIO toolchain and new repository URL.
- Standardized version references across entire project documentation.
- Enhanced repository structure for professional development workflow.

## [Version 3.8.14] - 2025-11-11
### Added
- None.

### Fixed
- **Critical**: Added missing `runtimeBLE` variable declaration that caused compilation errors on ESP32-S2/S3/C3/C6/H2 targets.
- Removed unused `DIAGNOSTIC_VERSION_HISTORY` array to reduce code clutter.

### Improved
- Cleaned up redundant `String` initializations (`String foo = ""` ? `String foo`) across 7 instances.
- Standardized `for` loop spacing (`for(` ? `for `) across 23 instances for better readability.
- Removed superfluous inline comments to improve code clarity.
- Updated version references to 3.8.14.

## [Version 3.8.0] - 2025-11-08
### Added
- Production release based on 3.7.30 with version bump for deployment readiness.

### Fixed
- None.

### Improved
- Standardized version references across all documentation and firmware constants.

## [Version 3.7.30] - 2025-11-08
### Added
- OLED-equipped boards now show Wi-Fi association, DHCP, and completion states during boot so the connection phase is visible without opening the serial monitor.

### Fixed
- Guarded optional NimBLE descriptors and aligned BLE callback signatures to eliminate missing header and `override` errors across ESP32 and ESP32-S3 targets.
- Prevented invalid NimBLE scan result conversions by validating the `start()` status before copying advertised device data.

### Improved
- Reused NimBLE scan buffers safely, resumed advertising after failures, and surfaced coherent HTTP errors whenever scans cannot start.
- Updated the FR/EN documentation set to capture the Wi-Fi splash workflow and the hardened BLE compatibility guidance for this release.

## [Version 3.5.1] - 2025-10-31
### Added
- Extended live translation bindings across diagnostics panels so wireless, pager, GPIO, and export widgets reuse the shared catalog without duplicates.

### Fixed
- Addressed untranslated board metadata (CPU cores, MAC addresses, reset reasons, memory metrics) to keep language toggles in sync without requiring a reload.

### Improved
- Refined the web client translation refresher to reapply prefixes, suffixes, and placeholders instantly when switching languages.

---

## [Version 3.4.0] - 2025-10-29
### Added
- Shared JSON helper prototypes to expose consistent utility signatures across diagnostics modules.

### Fixed
- Resolved build failures caused by mismatched JSON helper declarations during cross-module usage.
- Restored safe `String` conversions within the translation pipeline after the catalog optimisations.

### Improved
- Unified the HTTP JSON response builders and peripheral handlers to reduce duplication across exports.
- Harmonised the bilingual UI labels and retired the legacy touchpad diagnostic from the default run list.
- Updated the complete FR/EN documentation set to reflect the stabilised 3.3.x maintenance track.

---

## [Version 3.3.0] - 2025-10-27
### Added
- None.

### Fixed
- Removed redundant �NEW FEATURE� banner comments to avoid confusing maintenance efforts.

### Improved
- Consolidated the legacy "Version de dev" notes into a firmware constant so internal history remains accessible without banner duplication.
- Synced the firmware header comment and `DIAGNOSTIC_VERSION` macro at 3.3.0 and refreshed documentation to reflect the streamlined maintenance focus.

---

## [Version 3.2.0] - 2025-10-29
### Added
- Documented how to query the `/api/memory-details` endpoint and interpret fragmentation warnings from the latest diagnostics run.
- Captured the Bluetooth� and Wi-Fi debugging checklist from the post-release validation pass directly inside the usage and troubleshooting guides.

### Fixed
- Replaced remaining 3.1.19 references across READMEs and setup guides so the banner, guides, and bilingual changelog remain synchronized at 3.2.0.
- Clarified configuration guidance around language switching to avoid stale instructions when verifying `/api/set-language` responses.

### Improved
- Expanded installation, configuration, and usage manuals with concrete verification steps for the refreshed diagnostics (memory exports, BLE state widgets, Wi-Fi scan metadata).
- Strengthened the contributing guide with reminders to keep FR/EN documentation aligned during maintenance updates.

---

## [Version 3.1.19] - 2025-10-28
### Added
- None.

### Fixed
- Updated every guide to reference the bilingual changelog pair (`CHANGELOG.md` and `CHANGELOG_FR.md`) so both languages stay aligned.

### Improved
- Split the changelog into dedicated English and French files to make maintenance updates easier for each audience.
- Refreshed the version banner, `DIAGNOSTIC_VERSION`, and release highlights to reference 3.1.19 across the documentation set.

---

## [Version 3.1.18] - 2025-10-27
### Added
- None.

### Fixed
- Validated `/api/set-language` so French/English requests immediately return HTTP 200 while unsupported codes now receive a clear HTTP 400 response.
- Synchronized serial logs and exports with the new language-switch JSON responses.

### Improved
- Preallocated the `jsonEscape` buffer to reduce allocations while generating exports.
- Explicitly included `<cstring>` to guarantee C string helpers remain available on recent Arduino toolchains.
- Aligned the FR/EN documentation set (README, installation, configuration, usage, architecture, contributing, troubleshooting guides) with the 3.1.18 maintenance pass.

---

## [Version 3.1.18] - 2025-10-27
### Ajouts
- N�ant.

### Corrections
- Validation de `/api/set-language` pour renvoyer imm�diatement HTTP 200 sur `fr`/`en` et HTTP 400 sur les codes non pris en charge, supprimant les r�ponses ambigu�s.
- Synchronisation des journaux s�rie et exports avec les nouvelles r�ponses JSON du changement de langue.

### Am�liorations
- R�servation anticip�e du tampon de `jsonEscape` afin de r�duire les allocations pendant la g�n�ration d'exports.
- Inclusion explicite de `<cstring>` pour assurer la compatibilit� des fonctions de cha�ne C sur les toolchains Arduino r�centes.
- Documentation FR/EN align�e sur la maintenance 3.1.18 (README, guides d'installation, configuration, utilisation, architecture, contribution et d�pannage).

---

## [Version 3.1.16] - 2025-10-27
### Added
- Unified sticky banner showing version, Wi-Fi/Bluetooth status, and a quick-access link from the web dashboard.
- Bluetooth� commands (enable, rename, reset) exposed both in the interface and through dedicated REST endpoints.

### Fixed
- Repositioned tab navigation event delegation to restore the selection after dynamic reloads.
- Refreshed translations on tabs and dynamic elements after a language switch to remove inconsistent labels.
- Restored the �Not tested� label for additional diagnostics across the interface, API, and exports.

### Improved
- Enriched Wi-Fi scan responses (BSSID, band, channel width, PHY mode) to simplify RF analysis.
- Extended `htmlEscape` preallocation to limit reallocations during HTML streaming and export generation.
- Synchronized the header comment and `DIAGNOSTIC_VERSION` at 3.1.16 to align logs and generated files.

---

## [Version 3.1.15-maint] - 2025-10-26
### Added
- None.

### Fixed
- Standardized the �Not tested� label for additional diagnostics (ADC, touch, PWM, stress) in the interface, exports, and API.
- Synchronized the header comment and `DIAGNOSTIC_VERSION` at 3.1.15-maint so logs and exported files show the correct number.

### Improved
- Extended the `htmlEscape` preallocation to reduce reallocations while escaping generated HTML strings.
- Updated the FR/EN documentation (README, installation, configuration, usage, architecture, contributing, troubleshooting guides) to reflect the 3.1.15-maint maintenance and checks.

---

## [Version 3.1.14-maint] - 2025-10-26
### Added
- None.

### Fixed
- Restored �Not tested� status labels for the additional diagnostics to keep the French interface consistent.
- Aligned the version banner comment and `DIAGNOSTIC_VERSION` with revision 3.1.14-maint to reflect the active maintenance.

### Improved
- Increased the memory reserved by `htmlEscape` to limit reallocations while escaping generated strings.

---

## [3.1.1] - 2025-10-26
### Fixed
- Removed obsolete comments and internal markers to clarify firmware maintenance status.

### Documentation
- Updated the FR/EN guides, exports, and constants to reflect the move to version 3.1.1.

---

## [3.1.0] - 2025-10-24
### Highlights
- Automatic enablement of the Bluetooth� Low Energy service with native advertising on compatible targets (ESP32, S3, C3, C6, H2).
- Web dashboard enriched with a BLE card showing status, device name, and recent pairing logs.
- Fully rewritten FR/EN document sets covering installation, configuration, usage, architecture, troubleshooting, and contribution.

### Features
- Default GATT service with dynamic device renaming via the web interface or serial console.
- New contextual Wi-Fi status messages (association, DHCP, authentication, captive portal) displayed in the top banner and REST API.
- Relaxed initial support for the latest ESP32 SoC revisions (S3/C3/C6/H2) within the BLE and Wi-Fi modules.

### Interface & API
- Hardened tab navigation through JavaScript event delegation to avoid lockups after partial refresh.
- Responsive top menu grouping status indicators on a single line for screens below 768 px.
- TXT/JSON/CSV exports and print preview synchronized with the new BLE and Wi-Fi states.

### Documentation
- Consolidated guides inside `docs/` with cross-links to English and French versions.
- Added a detailed contributing guide (Git workflow, validation, formatting) and BLE/Wi-Fi troubleshooting procedures.

### Fixed
- Harmonized BLE messages across the serial console, `/api/status`, and the web interface.
- Cleaned the dynamic translation JSON to remove orphan keys and casing inconsistencies.
- Clarified active tab detection in the JavaScript bundle, removing duplicate listeners.

---

## [2.6.0] - 2025-10-15
### Features
- Manual buttons on the web interface plus REST endpoints to control each OLED diagnostic animation individually.
- Ability to start and stop display sequences directly from the serial console.

### Improved
- Simplified the OLED I�C reconfiguration flow: select SDA/SCL pins and speed directly from the interface.
- Refreshed the translation pack (FR/EN) for all new OLED labels and runtime states.

### Fixed
- Fully removed TFT support (firmware, Arduino dependencies, UI fragments), shrinking the binary and eliminating compilation warnings.

---

## [2.5.1] - 2025-10-10
### Fixed
- Clarified PSRAM statuses on the dashboard, exports, and printable reports (detected value, frequency, octal mode).
- Adjusted PSRAM detection order to avoid false negatives on ESP32-WROVER boards.

### Improved
- Highlighted PSRAM-compatible boards with reminders to enable the option in the Arduino IDE and installation docs.

---

## [2.5.0] - 2025-10-08
### Features
- Fully translated TXT/JSON/CSV exports (FR/EN) including ISO 8601 timestamps and build numbers.
- Printable preview aligned with the selected language and the new diagnostic blocks.

### Improved
- Enriched export filenames with version number, date, and detected board.
- Revised the REST export workflow to shorten generation times and harmonize column titles.

### Fixed
- Harmonized date formats (FR/EN localization) in exports and the printable view.
- Added missing translations on download buttons and section labels.

---

## [2.4.0] - 2025-10-07
### Features
- Full FR/EN multilingual interface with hot switching without page reload.
- `/api/set-language` and `/api/get-translations` endpoints enabling external clients to control the language.
- `languages.h` file centralizing more than 150 translated strings for the interface, exports, and serial console.

### Improved
- All texts (web, API, exports, logs) now rely on the centralized translation catalog.
- Added a visual indicator for the active language and persisted the preference in the browser's local storage.

---

## [2.3.0] - 2025-10-06
### Features
- Suite of 10 OLED 0.96" I�C tests (contrast toggles, inversion, scroll, custom frames) with explanatory messages.
- Dynamic SDA/SCL pin reconfiguration through the web interface and API to simplify rewiring.

### Improved
- Automatically detects the display at address 0x3C and reruns tests after connection.
- Added a contrast calibration module to optimise OLEDs based on supply voltage.

---








