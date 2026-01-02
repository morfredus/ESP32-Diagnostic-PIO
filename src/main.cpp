/*
 * ESP32 Diagnostic Suite
 * Compatible: ESP32 class targets with >=4MB Flash & >=8MB PSRAM (ESP32 / ESP32-S3)
 * Optimized for ESP32 Arduino Core 3.3.3 / PlatformIO
 * Tested board: ESP32-S3 DevKitC-1 N16R8 with PSRAM OPI (Core 3.3.3)
 * Author: morfredus
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>

#if defined(ARDUINO_ARCH_ESP32)
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
  #if defined(__has_include)
    #if __has_include(<ESPmDNS.h>)
      #include <ESPmDNS.h>
      #define DIAGNOSTIC_HAS_MDNS 1
      #define DIAGNOSTIC_USES_ESPMDNS 1
    #elif __has_include(<mdns.h>)
      #include <mdns.h>
      #define DIAGNOSTIC_HAS_MDNS 1
      #define DIAGNOSTIC_USES_IDF_MDNS 1
    #else
      #define DIAGNOSTIC_HAS_MDNS 0
    #endif
  #else
    #include <ESPmDNS.h>
    #define DIAGNOSTIC_HAS_MDNS 1
    #define DIAGNOSTIC_USES_ESPMDNS 1
  #endif
#else
  #define DIAGNOSTIC_HAS_MDNS 0
#endif

#if !defined(DIAGNOSTIC_HAS_MDNS)
  #define DIAGNOSTIC_HAS_MDNS 0
#endif

#if DIAGNOSTIC_HAS_MDNS && defined(DIAGNOSTIC_USES_ESPMDNS)
  #if defined(NO_GLOBAL_MDNS) || defined(NO_GLOBAL_INSTANCES)
    static MDNSResponder diagnosticMDNSResponder;
    #define DIAGNOSTIC_MDNS_SERVER diagnosticMDNSResponder
  #else
    #define DIAGNOSTIC_MDNS_SERVER MDNS
  #endif
#endif
#include <esp_chip_info.h>
#include <esp_err.h>
#include <esp_mac.h>
#include <esp_flash.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#if defined(__has_include)
  #if __has_include(<sdkconfig.h>)
    #include <sdkconfig.h>
    #define DIAGNOSTIC_HAS_SDKCONFIG 1
  #endif
  #if __has_include(<esp_netif.h>)
    #include <esp_netif.h>
    #define DIAGNOSTIC_HAS_ESP_NETIF 1
  #endif
#else
  #include <esp_netif.h>
  #define DIAGNOSTIC_HAS_ESP_NETIF 1
#endif
#if !defined(DIAGNOSTIC_HAS_SDKCONFIG)
  #define DIAGNOSTIC_HAS_SDKCONFIG 0
#endif
#include <soc/soc.h>
#include <soc/rtc.h>
#if defined(__has_include)
  #if __has_include(<soc/soc_caps.h>)
    #include <soc/soc_caps.h>
  #endif
#else
  #include <soc/soc_caps.h>
#endif
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <vector>
#include <cstring>
#include <string>
#include <initializer_list>
#include "json_helpers.h"

// Configuration file - customize your setup
// Copy include/config-example.h to include/config.h and customize your settings
#include "config.h"

// Secrets file - customize your network credentials
// For reference, see include/secrets-example.h
// secrets.h is excluded from version control via .gitignore
#include "secrets.h"

// Dual-language UI strings
#include "languages.h"

// TFT Display support
#include "tft_display.h"

// GPS module
#include "gps_module.h"

// Environmental sensors (AHT20 + BMP280)
#include "environmental_sensors.h"

// Set default language from config.h
Language currentLanguage = DEFAULT_LANGUAGE;

// --- Prototypes pour fonctions de réponse JSON/API ---
void sendJsonResponse(int statusCode, std::initializer_list<JsonFieldSpec> fields);
void sendOperationSuccess(const String& message, std::initializer_list<JsonFieldSpec> extraFields = {});
void sendOperationError(int statusCode, const String& message, std::initializer_list<JsonFieldSpec> extraFields = {});
void sendActionResponse(int statusCode, bool success, const String& message, std::initializer_list<JsonFieldSpec> extraFields = {});
void tftStepBoot();

static String buildActionResponseJson(bool success,
                                      const String& message,
                                      std::initializer_list<JsonFieldSpec> extraFields = {});
String htmlEscape(const String& raw);
String jsonEscape(const char* raw);
String buildTranslationsJSON();
String buildTranslationsJSON(Language lang);

#if defined(__has_include)
  #if __has_include(<esp_arduino_version.h>)
    #include <esp_arduino_version.h>
  #endif
#endif

#ifndef ESP_ARDUINO_VERSION_VAL
#define ESP_ARDUINO_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#endif

#ifndef ESP_ARDUINO_VERSION
#if defined(ESP_ARDUINO_VERSION_MAJOR) && defined(ESP_ARDUINO_VERSION_MINOR) && defined(ESP_ARDUINO_VERSION_PATCH)
#define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH)
#else
#define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(0, 0, 0)
#endif
#endif

// ========== CONFIGURATION (from config.h) ==========
// Version is now defined in platformio.ini as PROJECT_VERSION
const char* DIAGNOSTIC_VERSION_STR = PROJECT_VERSION;
const char* MDNS_HOSTNAME_STR = DIAGNOSTIC_HOSTNAME;

// HTTPS/HTTP scheme constants
const char* const DIAGNOSTIC_SECURE_SCHEME = "https://";
const char* const DIAGNOSTIC_LEGACY_SCHEME = "http://";

#ifndef DIAGNOSTIC_UNUSED
#define DIAGNOSTIC_UNUSED __attribute__((unused))
#endif

static inline String buildAccessUrl(const String& hostOrAddress,
                                    bool preferSecure = (DIAGNOSTIC_PREFER_SECURE != 0)) {
  if (hostOrAddress.length() == 0) {
    return String();
  }
  const String secureScheme = String(DIAGNOSTIC_SECURE_SCHEME);
  const String legacyScheme = String(DIAGNOSTIC_LEGACY_SCHEME);
  return (preferSecure ? secureScheme : legacyScheme) + hostOrAddress;
}

static inline String getStableAccessHost() {
  return String(DIAGNOSTIC_HOSTNAME) + ".local";
}

String getArduinoCoreVersionString() {
// [OPT-001] Optimize version string formatting: use snprintf instead of multiple String concatenations
#if defined(ESP_ARDUINO_VERSION_MAJOR) && defined(ESP_ARDUINO_VERSION_MINOR) && defined(ESP_ARDUINO_VERSION_PATCH)
  char versionBuf[16];
  snprintf(versionBuf, sizeof(versionBuf), "%u.%u.%u", 
    ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH);
  return String(versionBuf);
#else
  uint32_t value = ESP_ARDUINO_VERSION;
  int major = (value >> 16) & 0xFF;
  int minor = (value >> 8) & 0xFF;
  int patch = value & 0xFF;
  char versionBuf[16];
  snprintf(versionBuf, sizeof(versionBuf), "%d.%d.%d", major, minor, patch);
  return String(versionBuf);
#endif
}

// ========== HARDWARE PIN CONFIGURATION (from board_config.h) ==========
// Runtime pin variables (lowercase) initialized from board_config.h defines (UPPERCASE)
// Using lowercase names avoids preprocessor conflicts while enabling dynamic remapping

// I2C pins for OLED and environmental sensors (modifiable via web interface)
int i2c_sda = I2C_SDA;
int i2c_scl = I2C_SCL;

// RGB LED pins (modifiable via web interface)
int rgb_led_pin_r = LED_RED;
int rgb_led_pin_g = LED_GREEN;
int rgb_led_pin_b = LED_BLUE;

// Sensor and output pins (modifiable via web interface)
int pwm_pin = PWM;
int buzzer_pin = BUZZER;
int dht_pin = DHT;
int light_sensor_pin = LIGHT_SENSOR;
int distance_trig_pin = DISTANCE_TRIG;
int distance_echo_pin = DISTANCE_ECHO;
int motion_sensor_pin = MOTION_SENSOR;

// SD Card pins (modifiable via web interface)
int sd_miso_pin = SD_MISO;
int sd_mosi_pin = SD_MOSI;
int sd_sclk_pin = SD_SCLK;
int sd_cs_pin = SD_CS;

// Rotary Encoder pins (modifiable via web interface)
int rotary_clk_pin = ROTARY_CLK;
int rotary_dt_pin = ROTARY_DT;
int rotary_sw_pin = ROTARY_SW;

// OLED display settings (from config.h)
uint8_t oledRotation = DEFAULT_OLED_ROTATION;
int oledWidth = SCREEN_WIDTH;
int oledHeight = SCREEN_HEIGHT;

// DHT sensor type (configurable at runtime)
uint8_t DHT_SENSOR_TYPE = DEFAULT_DHT_SENSOR_TYPE;

// ========== OBJETS GLOBAUX ==========
WebServer server(WEB_SERVER_PORT);
WiFiMulti wifiMulti;
#if DIAGNOSTIC_HAS_MDNS
bool mdnsServiceActive = false;
bool wifiPreviouslyConnected = false;
unsigned long lastMDNSAttempt = 0;
bool mdnsLastAttemptFailed = false;
#if defined(DIAGNOSTIC_USES_IDF_MDNS)
bool mdnsResponderInitialized = false;
#endif
#endif
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);


// NeoPixel (from board_config.h via config.h)
int LED_PIN = NEOPIXEL;
int LED_COUNT = DEFAULT_NEOPIXEL_COUNT;

// TFT pins (modifiables via web)
#if ENABLE_TFT_DISPLAY
int tftMISO = TFT_MISO;
int tftMOSI = TFT_MOSI;
int tftSCLK = TFT_SCLK;
int tftCS = TFT_CS;
int tftDC = TFT_DC;
int tftRST = TFT_RST;
int tftBL = TFT_BL;
int tftWidth = TFT_WIDTH;
int tftHeight = TFT_HEIGHT;
int tftRotation = TFT_ROTATION;
// v3.30.0: Current TFT driver type (runtime switchable)
#if defined(TFT_USE_ILI9341)
String tftDriver = "ILI9341";
#elif defined(TFT_USE_ST7789)
String tftDriver = "ST7789";
#else
String tftDriver = "ILI9341";  // Default
#endif
#endif
Adafruit_NeoPixel *strip = nullptr;

// NeoPixel heartbeat state
volatile bool neopixelStatusPaused = false;
unsigned long neopixelHeartbeatPreviousMillis = 0;
const unsigned long NEOPIXEL_HEARTBEAT_INTERVAL_MS = 1000;
bool neopixelHeartbeatState = false;
bool neopixelStatusKnown = false;
bool neopixelLastWifiConnected = false;
bool neopixelConnecting = false;

// [OPT-004] Constant for repeated test result initialization
static const String DEFAULT_TEST_RESULT_STR = String(Texts::not_tested);
// [OPT-009] Constants for repeated test status strings
static const String OK_STR = String(Texts::ok);
static const String FAIL_STR = String(Texts::fail);

bool neopixelTested = false;
bool neopixelAvailable = false;
bool neopixelSupported = false;
String neopixelTestResult = DEFAULT_TEST_RESULT_STR;

// Forward declarations for NeoPixel functions
void neopixelShowRebootFlash();
void neopixelSetWifiState(bool connected);
void neopixelShowConnecting();
void neopixelPauseStatus();
void neopixelResumeStatus();
void neopixelRestoreWifiStatus();
void updateNeoPixelWifiStatus();

// Built-in LED (from config.h)
int BUILTIN_LED_PIN = LED_BUILTIN;
bool builtinLedTested = false;
bool builtinLedAvailable = false;
String builtinLedTestResult = DEFAULT_TEST_RESULT_STR;

bool oledTested = false;
bool oledAvailable = false;
String oledTestResult = DEFAULT_TEST_RESULT_STR;

#if ENABLE_TFT_DISPLAY
String tftTestResult = DEFAULT_TEST_RESULT_STR;
#endif

// Exécution asynchrone des tests matériels
typedef void (*TestRoutine)();

struct AsyncTestRunner {
  const char* taskName;
  TaskHandle_t taskHandle;
  volatile bool running;
};

struct AsyncTestTaskArgs {
  AsyncTestRunner* runner;
  TestRoutine routine;
};

static void asyncTestTask(void* parameters) {
  AsyncTestTaskArgs* args = static_cast<AsyncTestTaskArgs*>(parameters);
  if (args && args->routine) {
    args->routine();
  }
  if (args) {
    if (args->runner) {
      args->runner->running = false;
      args->runner->taskHandle = nullptr;
    }
    delete args;
  }
  vTaskDelete(nullptr);
}

static bool startAsyncTest(AsyncTestRunner& runner,
                           TestRoutine routine,
                           bool& alreadyRunning,
                           uint32_t stackSize = 4096,
                           UBaseType_t priority = 1) {
  alreadyRunning = runner.running;
  if (runner.running) {
    return false;
  }

  AsyncTestTaskArgs* args = new AsyncTestTaskArgs{&runner, routine};
  runner.running = true;

#if CONFIG_FREERTOS_UNICORE
  const BaseType_t targetCore = tskNO_AFFINITY;
#else
  const BaseType_t targetCore = 1;
#endif

  BaseType_t result = xTaskCreatePinnedToCore(asyncTestTask,
                                              runner.taskName,
                                              stackSize,
                                              args,
                                              priority,
                                              &runner.taskHandle,
                                              targetCore);
  if (result != pdPASS) {
    runner.running = false;
    runner.taskHandle = nullptr;
    delete args;
    alreadyRunning = false;
    return false;
  }

  alreadyRunning = false;
  return true;
}

// Orchestrateurs asynchrones des tests lents
static AsyncTestRunner builtinLedTestRunner = {"BuiltinLEDTest", nullptr, false};
static AsyncTestRunner neopixelTestRunner = {"NeoPixelTest", nullptr, false};
static AsyncTestRunner oledTestRunner = {"OLEDTest", nullptr, false};
static AsyncTestRunner rgbLedTestRunner = {"RgbLedTest", nullptr, false};
static AsyncTestRunner buzzerTestRunner = {"BuzzerTest", nullptr, false};
static AsyncTestRunner sdTestRunner = {"SDTest", nullptr, false};
static AsyncTestRunner rotaryTestRunner = {"RotaryTest", nullptr, false};

bool runtimeBLE = false;

String adcTestResult = DEFAULT_TEST_RESULT_STR;
String pwmTestResult = DEFAULT_TEST_RESULT_STR;
String partitionsInfo;
String spiInfo;
String stressTestResult = DEFAULT_TEST_RESULT_STR;
// Memory stress telemetry cache
size_t stressAllocationCount = 0;
unsigned long stressDurationMs = 0;

// Résultats de tests des nouveaux capteurs
String rgbLedTestResult = DEFAULT_TEST_RESULT_STR;
bool rgbLedAvailable = false;

String buzzerTestResult = DEFAULT_TEST_RESULT_STR;
bool buzzerAvailable = false;

String dhtTestResult = DEFAULT_TEST_RESULT_STR;
bool dhtAvailable = false;
float dhtTemperature = -999.0;
float dhtHumidity = -999.0;

String lightSensorTestResult = DEFAULT_TEST_RESULT_STR;
bool lightSensorAvailable = false;
int lightSensorValue = -1;

String distanceSensorTestResult = DEFAULT_TEST_RESULT_STR;
bool distanceSensorAvailable = false;
float distanceValue = -1.0;

String motionSensorTestResult = DEFAULT_TEST_RESULT_STR;
bool motionSensorAvailable = false;
bool motionDetected = false;

// ========== SD CARD ==========
String sdTestResult = DEFAULT_TEST_RESULT_STR;
bool sdAvailable = false;
bool sdTested = false;
SPIClass * sdSPI = nullptr;
uint64_t sdCardSize = 0;
uint8_t sdCardType = 0;
String sdCardTypeStr = "Unknown";

// ========== ENCODEUR ROTATIF ==========
String rotaryTestResult = DEFAULT_TEST_RESULT_STR;
volatile long rotaryPosition = 0;
volatile bool rotaryButtonPressed = false;
volatile unsigned long lastRotaryInterruptTime = 0;
volatile unsigned long lastButtonPressTime = 0;
const unsigned long rotaryDebounceDelay = 5;
const unsigned long buttonDebounceDelay = 50;
bool rotaryAvailable = false;
bool rotaryTested = false;
int lastRotaryState = 0;
int lastButtonState = HIGH;

// ========== BOUTONS ==========
// Gestion simple avec anti-rebond et actions utiles par défaut
#if ENABLE_BUTTONS
static int buttonBootPin = BUTTON_BOOT;
static int button1Pin = BUTTON_1;
static int button2Pin = BUTTON_2;
static int buttonBootLast = HIGH;
static int button1Last = HIGH;
static int button2Last = HIGH;
static unsigned long buttonBootPressStart = 0;
static unsigned long button1LastChange = 0;
static unsigned long button2LastChange = 0;
static const unsigned long debounceMs = 30;
static const unsigned long longPressMs = 2000;
static bool buttonBootLongPressTriggered = false;

static void initButtons() {
  // Bouton BOOT (GPIO 0)
  if (buttonBootPin >= 0 && buttonBootPin <= 48) {
    pinMode(buttonBootPin, INPUT_PULLUP);
  }
  
  // Bouton 1 (GPIO 38 ESP32-S3 / GPIO 34 ESP32 Classic)
  if (button1Pin >= 0 && button1Pin <= 48) {
#if defined(CONFIG_IDF_TARGET_ESP32)
    if (button1Pin >= 34 && button1Pin <= 39) {
      pinMode(button1Pin, INPUT);
    } else {
      pinMode(button1Pin, INPUT_PULLUP);
    }
#else
    pinMode(button1Pin, INPUT_PULLUP);
#endif
  }
  
  // Bouton 2 (GPIO 39 ESP32-S3 / GPIO 35 ESP32 Classic)
  if (button2Pin >= 0 && button2Pin <= 48) {
#if defined(CONFIG_IDF_TARGET_ESP32)
    if (button2Pin >= 34 && button2Pin <= 39) {
      pinMode(button2Pin, INPUT);
    } else {
      pinMode(button2Pin, INPUT_PULLUP);
    }
#else
    pinMode(button2Pin, INPUT_PULLUP);
#endif
  }
}

// Inline helper for NeoPixel reboot flash
inline void neopixelShowRebootFlash_Impl() {
  if (strip != nullptr) {
    strip->setBrightness(60);
    strip->setPixelColor(0, strip->Color(255, 0, 255)); // Violet
    strip->show();
  }
}

// Bouton BOOT: Appui long (2s) → reboot avec barre de progression TFT
static void onButtonBootLongPress() {
  // NeoPixel flash violet pour confirmer le reboot
  neopixelShowRebootFlash_Impl();
  
#if ENABLE_TFT_DISPLAY
  if (tftAvailable) {
    tft->fillScreen(TFT_BLACK);
    tft->setTextSize(2);
    tft->setTextColor(TFT_WHITE);
    tft->setCursor(10, 80);
    tft->println("Redemarrage...");
    
    // Barre de progression
    int barX = 20;
    int barY = 120;
    int barW = tftWidth - 40;
    int barH = 20;
    tft->drawRect(barX, barY, barW, barH, TFT_WHITE);
    
    for (int i = 0; i <= 100; i += 10) {
      int fillW = (barW - 4) * i / 100;
      tft->fillRect(barX + 2, barY + 2, fillW, barH - 4, TFT_GREEN);
      delay(50);
    }
    delay(300);
  }
#endif
  ESP.restart();
}

// Bouton 1: Cycle de couleurs RGB avec position éteinte
static void onButton1Pressed() {
  static int rgbStep = 0;
  int r = 0, g = 0, b = 0;
  
  switch (rgbStep % 5) {
    case 0: r = 0; g = 0; b = 0; break;       // Éteint
    case 1: r = 255; g = 0; b = 0; break;     // Rouge
    case 2: r = 0; g = 255; b = 0; break;     // Vert
    case 3: r = 0; g = 0; b = 255; break;     // Bleu
    case 4: r = 255; g = 255; b = 255; break; // Blanc
  }

  if (rgb_led_pin_r >= 0 && rgb_led_pin_g >= 0 && rgb_led_pin_b >= 0) {
    analogWrite(rgb_led_pin_r, r);
    analogWrite(rgb_led_pin_g, g);
    analogWrite(rgb_led_pin_b, b);
  }

  rgbStep++;
}

// Bouton 2: Bip à l'appui
static void onButton2Pressed() {
  if (buzzer_pin >= 0) {
    tone(buzzer_pin, 1000, 200);
  }
}

// --- Machine d'état pour gestion BOOT/TFT/NeoPixel ---
//
// BOOT_NORMAL   : État par défaut, aucun appui sur BOOT
// BOOT_PROGRESS : BOOT maintenu, barre de progression affichée, NeoPixel violet
// BOOT_CANCEL   : BOOT relâché avant 100%, retour écran boot, NeoPixel Earthbeat restauré
// BOOT_FINAL    : BOOT maintenu jusqu'à 100%, action finale (reset/config spéciale)
//
// La machine d'état permet une gestion robuste et fluide du comportement utilisateur.
//
// Rendu TFT :
// - Seule la zone de la barre de progression est redessinée (pas de fillScreen)
// - Le cadre n'est dessiné qu'une seule fois (frameDrawn)
// - La barre est mise à jour uniquement si la progression change (lastProgress)
// - Zéro scintillement, performance maximale
//
// Gestion NeoPixel :
// - Earthbeat (battement vert) est mis en pause pendant la progression
// - NeoPixel passe en violet (#8000FF) pendant toute la progression
// - Earthbeat est restauré si l'utilisateur relâche avant 100%
//
// Comportement BOOT :
// - Appui court : rien, retour à l'écran de boot
// - Appui long (100%) : action finale (reset/config)
//
enum BootState {
  BOOT_NORMAL,
  BOOT_PROGRESS,
  BOOT_CANCEL,
  BOOT_FINAL
};
static BootState bootState = BOOT_NORMAL;
static int lastProgress = -1;
static bool frameDrawn = false;
static bool earthbeatWasActive = false;

// Fonction centrale de gestion des boutons et de la machine d'état BOOT
static void maintainButtons() {
  unsigned long now = millis();

  // --- Gestion du bouton BOOT avec machine d'état ---
  if (buttonBootPin >= 0) {
    int s = digitalRead(buttonBootPin);
    unsigned long pressDuration = now - buttonBootPressStart;

    switch (bootState) {
      case BOOT_NORMAL:
        if (s == LOW && buttonBootLast == HIGH) {
          // Début d'appui
          buttonBootPressStart = now;
          buttonBootLongPressTriggered = false;
          bootState = BOOT_PROGRESS;
          frameDrawn = false;
          lastProgress = -1;
          // Désactiver Earthbeat, mémoriser l'état
          earthbeatWasActive = !neopixelStatusPaused;
          neopixelPauseStatus();
          if (strip) {
            strip->setBrightness(60);
            strip->setPixelColor(0, strip->Color(128, 0, 255)); // Violet
            strip->show();
          }
// Couleur violette NeoPixel : #8000FF = 0x801F
#if ENABLE_TFT_DISPLAY
          if (tftAvailable) {
            tft->fillScreen(TFT_BLACK);
            uint16_t violet = 0x801F;
            tft->setTextColor(violet);
            tft->setTextSize(3);
            int16_t x1, y1;
            uint16_t w, h;
            tft->getTextBounds("Reboot ?", 0, 0, &x1, &y1, &w, &h);
            int x = (tftWidth - w) / 2;
            int y = 60;
            tft->setCursor(x, y);
            tft->println("Reboot ?");
          }
#endif
        }
        break;
      case BOOT_PROGRESS:
        if (s == LOW) {
          // Appui maintenu
          int progress = (pressDuration * 100) / longPressMs;
          if (progress > 100) progress = 100;

// Couleur violette NeoPixel : #8000FF = 0x801F
#if ENABLE_TFT_DISPLAY
          if (tftAvailable) {
            uint16_t violet = 0x801F;
            int barX = 20;
            int barY = 130;
            int barW = tftWidth - 40;
            int barH = 20;
            // Dessiner le cadre une seule fois
            if (!frameDrawn) {
              // Nettoyage zone barre
              tft->fillRect(barX-2, barY-2, barW+4, barH+4, TFT_BLACK);
              tft->drawRect(barX, barY, barW, barH, violet);
              frameDrawn = true;
            }
            // Mise à jour de la barre uniquement si changement
            if (progress != lastProgress) {
              int fillW = (barW - 4) * progress / 100;
              // Efface uniquement la zone utile
              tft->fillRect(barX + 2, barY + 2, barW - 4, barH - 4, TFT_BLACK);
              if (fillW > 0) tft->fillRect(barX + 2, barY + 2, fillW, barH - 4, violet);
              lastProgress = progress;
            }
          }
#endif
          // NeoPixel reste violet pendant toute la progression
          if (strip) {
            strip->setBrightness(60);
            strip->setPixelColor(0, strip->Color(128, 0, 255));
            strip->show();
          }
          // Atteint 100% ?
          if (progress >= 100 && !buttonBootLongPressTriggered) {
            buttonBootLongPressTriggered = true;
            bootState = BOOT_FINAL;
          }
        } else {
          // Relâchement avant 100%
          bootState = BOOT_CANCEL;
        }
        break;
      case BOOT_CANCEL:
        // Restaure l'état NeoPixel
        if (earthbeatWasActive) neopixelResumeStatus();
        else neopixelPauseStatus();
        // Retour écran boot complet (splash + IP)
#if ENABLE_TFT_DISPLAY
        if (tftAvailable) {
          displayBootSplash();
          if (WiFi.status() == WL_CONNECTED) {
            displayWiFiConnected(WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
          } else {
            displayWiFiStatus("WiFi not connected", TFT_ORANGE);
          }
        }
#endif
        frameDrawn = false;
        lastProgress = -1;
        bootState = BOOT_NORMAL;
        break;
      case BOOT_FINAL:
        // Action finale (reset config, etc.)
        // NeoPixel reste violet ou logique projet
#if ENABLE_TFT_DISPLAY
        if (tftAvailable) {
          tft->fillRect(0, 0, tftWidth, tftHeight, TFT_BLACK);
          tft->setTextSize(2);
          tft->setTextColor(TFT_WHITE);
          tft->setCursor(10, 80);
          tft->println("Reboot...");
        }
#endif
        delay(300);
        ESP.restart();
        break;
    }
    buttonBootLast = s;
  }

  // --- Bouton 1: Gestion appui simple pour cycle RGB ---
  if (button1Pin >= 0) {
    int s = digitalRead(button1Pin);
    if (s != button1Last) {
      if (now - button1LastChange > debounceMs) {
        button1LastChange = now;
        button1Last = s;
        if (s == LOW) {
          onButton1Pressed();
        }
      }
    }
  }

  // --- Bouton 2: Gestion appui simple pour bip ---
  if (button2Pin >= 0) {
    int s = digitalRead(button2Pin);
    if (s != button2Last) {
      if (now - button2LastChange > debounceMs) {
        button2LastChange = now;
        button2Last = s;
        if (s == LOW) {
          onButton2Pressed();
        }
      }
    }
  }
}
#endif

// ========== STRUCTURES ==========
struct DiagnosticInfo {
  String chipModel;
  String chipRevision;
  int cpuCores;
  uint32_t cpuFreqMHz;
  uint32_t flashSize;
  uint32_t psramSize;
  String macAddress;
  
  uint32_t heapSize;
  uint32_t freeHeap;
  uint32_t minFreeHeap;
  uint32_t maxAllocHeap;
  
  bool hasWiFi;
  bool hasBT;
  bool hasBLE;
  String wifiSSID;
  int wifiRSSI;
  String ipAddress;
  bool mdnsAvailable;
  bool bluetoothEnabled;
  bool bluetoothAdvertising;
  String bluetoothName;
  String bluetoothAddress;

  String gpioList;
  int totalGPIO;
  
  String sdkVersion;
  String idfVersion;
  unsigned long uptime;
  float temperature;
  
  bool neopixelTested;
  bool neopixelAvailable;
  String neopixelResult;
  
  unsigned long cpuBenchmark;
  unsigned long memBenchmark;
  
  String i2cDevices;
  int i2cCount;
  
  bool oledTested;
  bool oledAvailable;
  String oledResult;
} diagnosticData;

// Include web interface after DiagnosticInfo definition
#include "web_interface.h"

struct DetailedMemoryInfo {
  uint32_t flashSizeReal;
  uint32_t flashSizeChip;
  
  uint32_t psramTotal;
  uint32_t psramFree;
  uint32_t psramUsed;
  uint32_t psramLargestBlock;
  bool psramAvailable;
  bool psramConfigured;
  bool psramBoardSupported;
  const char* psramType;

  uint32_t sramTotal;
  uint32_t sramFree;
  uint32_t sramUsed;
  uint32_t sramLargestBlock;
  
  bool sramTestPassed;
  bool psramTestPassed;
  float fragmentationPercent;
  String memoryStatus;
};

DetailedMemoryInfo detailedMemory;

struct GPIOTestResult {
  int pin;
  bool tested;
  bool working;
  String mode;
  String notes;
};

std::vector<GPIOTestResult> gpioResults;

struct WiFiNetwork {
  String ssid;
  int rssi;
  int channel;
  String encryption;
  String bssid;
  int freqMHz;
  String band;
  String bandwidth;
  String phyModes;
};

std::vector<WiFiNetwork> wifiNetworks;

struct ADCReading {
  int pin;
  int rawValue;
  float voltage;
};

std::vector<ADCReading> adcReadings;

#define HISTORY_SIZE 60
float heapHistory[HISTORY_SIZE];
float tempHistory[HISTORY_SIZE];
int historyIndex = 0;

String getStableAccessURL() {
  return buildAccessUrl(getStableAccessHost());
}

void configureNetworkHostname() {
  if (!WiFi.setHostname(DIAGNOSTIC_HOSTNAME)) {
    Serial.printf("[WiFi] Impossible de définir le nom d'hôte %s\r\n", DIAGNOSTIC_HOSTNAME);
  } else {
    Serial.printf("[WiFi] Nom d'hôte défini : %s.local\r\n", DIAGNOSTIC_HOSTNAME);
  }
#if defined(ARDUINO_ARCH_ESP32)
  #if defined(DIAGNOSTIC_HAS_ESP_NETIF)
    esp_netif_t* staNetif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (staNetif != nullptr) {
      esp_err_t netifStatus = esp_netif_set_hostname(staNetif, DIAGNOSTIC_HOSTNAME);
      if (netifStatus != ESP_OK) {
        Serial.printf("[WiFi] Échec assignation hostname netif (0x%X)\r\n", netifStatus);
      }
    } else {
      Serial.println("[WiFi] Interface STA introuvable pour l'assignation du hostname");
    }
  #elif defined(DIAGNOSTIC_HAS_TCPIP_ADAPTER)
    err_t adapterStatus = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, DIAGNOSTIC_HOSTNAME);
    if (adapterStatus != ERR_OK) {
      Serial.printf("[WiFi] Échec assignation hostname TCP/IP (%d)\r\n", static_cast<int>(adapterStatus));
    }
  #endif
#endif
}

void stopMDNSService(const char* reason) {
#if DIAGNOSTIC_HAS_MDNS
  if (!mdnsServiceActive) {
    return;
  }
#if defined(DIAGNOSTIC_USES_ESPMDNS)
  DIAGNOSTIC_MDNS_SERVER.end();
#elif defined(DIAGNOSTIC_USES_IDF_MDNS)
  if (mdnsResponderInitialized) {
    mdns_free();
    mdnsResponderInitialized = false;
  }
#endif
  mdnsServiceActive = false;
  if (reason != nullptr) {
    Serial.printf("[mDNS] Service arrêté (%s)\r\n", reason);
  } else {
    Serial.println("[mDNS] Service arrêté");
  }
#else
  (void) reason;
#endif
}

bool startMDNSService(bool verbose) {
#if DIAGNOSTIC_HAS_MDNS
  if (!WiFi.isConnected()) {
    return false;
  }
  if (mdnsServiceActive) {
    return true;
  }

  unsigned long now = millis();
  if (!verbose && (now - lastMDNSAttempt < 5000)) {
    return false;
  }

  lastMDNSAttempt = now;
#if defined(DIAGNOSTIC_USES_ESPMDNS)
  if (!DIAGNOSTIC_MDNS_SERVER.begin(DIAGNOSTIC_HOSTNAME)) {
    if (verbose || !mdnsLastAttemptFailed) {
      Serial.println("[mDNS] Échec du démarrage - nouvel essai dans 5s");
    }
    mdnsLastAttemptFailed = true;
    return false;
  }

  DIAGNOSTIC_MDNS_SERVER.addService("http", "tcp", 80);
#elif defined(DIAGNOSTIC_USES_IDF_MDNS)
  esp_err_t mdnsInitStatus = mdns_init();
  if (mdnsInitStatus != ESP_OK) {
    if (verbose || !mdnsLastAttemptFailed) {
      Serial.printf("[mDNS] Initialisation IDF échouée: 0x%02X\r\n", mdnsInitStatus);
    }
    mdnsLastAttemptFailed = true;
    return false;
  }

  mdnsResponderInitialized = true;

  esp_err_t hostStatus = mdns_hostname_set(DIAGNOSTIC_HOSTNAME);
  if (hostStatus != ESP_OK) {
    if (verbose || !mdnsLastAttemptFailed) {
      Serial.printf("[mDNS] Attribution du nom d'hôte échouée: 0x%02X\r\n", hostStatus);
    }
    mdns_free();
    mdnsResponderInitialized = false;
    mdnsLastAttemptFailed = true;
    return false;
  }

  mdns_instance_name_set(PROJECT_NAME);
  esp_err_t serviceStatus = mdns_service_add(PROJECT_NAME, "_http", "_tcp", 80, nullptr, 0);
  if (serviceStatus != ESP_OK) {
    if (verbose || !mdnsLastAttemptFailed) {
      Serial.printf("[mDNS] Publication du service échouée: 0x%02X\r\n", serviceStatus);
    }
    mdns_free();
    mdnsResponderInitialized = false;
    mdnsLastAttemptFailed = true;
    return false;
  }
  mdns_service_txt_item_set("_http", "_tcp", "path", "/");
#endif
  mdnsServiceActive = true;
  mdnsLastAttemptFailed = false;
  Serial.printf("[mDNS] Service actif sur %s\r\n", getStableAccessURL().c_str());
  return true;
#else
  (void) verbose;
  return false;
#endif
}

void maintainNetworkServices() {
#if DIAGNOSTIC_HAS_MDNS
  bool wifiConnectedNow = (WiFi.status() == WL_CONNECTED);
  if (wifiConnectedNow) {
    if (!wifiPreviouslyConnected) {
      startMDNSService(true);
    } else {
      startMDNSService(false);
    }
  } else if (mdnsServiceActive) {
    stopMDNSService("WiFi déconnecté");
  }
  wifiPreviouslyConnected = wifiConnectedNow;
#endif
}

// ========== DÉTECTION MODÈLE ==========
String detectChipModel() {
  #ifdef CONFIG_IDF_TARGET_ESP32
    return "ESP32";
  #elif defined(CONFIG_IDF_TARGET_ESP32S2)
    return "ESP32-S2";
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    return "ESP32-S3";
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    return "ESP32-C3";
  #elif defined(CONFIG_IDF_TARGET_ESP32C6)
    return "ESP32-C6";
  #elif defined(CONFIG_IDF_TARGET_ESP32H2)
    return "ESP32-H2";
  #else
    return "ESP32";
  #endif
}

// ========== FONCTIONS UTILITAIRES ==========
String getChipFeatures() {
  // [OPT-006] Optimize chip features string building: avoid substring operations
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  char featureBuf[128] = {0};
  char *p = featureBuf;
  size_t remaining = sizeof(featureBuf) - 1;
  bool first = true;

  if (chip_info.features & CHIP_FEATURE_WIFI_BGN) {
    if (!first && remaining > 2) { *p++ = ','; *p++ = ' '; remaining -= 2; }
    const char *wifi = "WiFi 2.4GHz";
    size_t len = strlen(wifi);
    if (len < remaining) { strcpy(p, wifi); p += len; remaining -= len; first = false; }
  }
  if (chip_info.features & CHIP_FEATURE_BT) {
    if (!first && remaining > 2) { *p++ = ','; *p++ = ' '; remaining -= 2; }
    const char *bt = "Bluetooth Classic";
    size_t len = strlen(bt);
    if (len < remaining) { strcpy(p, bt); p += len; remaining -= len; first = false; }
  }
  if (chip_info.features & CHIP_FEATURE_BLE) {
    if (!first && remaining > 2) { *p++ = ','; *p++ = ' '; remaining -= 2; }
    const char *ble = "Bluetooth LE";
    size_t len = strlen(ble);
    if (len < remaining) { strcpy(p, ble); p += len; remaining -= len; first = false; }
  }
  
  if (p == featureBuf) {
    return Texts::none.str();
  }
  return String(featureBuf);
}

String getFlashType() {
  #ifdef CONFIG_ESPTOOLPY_FLASHMODE_QIO
    return "QIO";
  #elif defined(CONFIG_ESPTOOLPY_FLASHMODE_QOUT)
    return "QOUT";
  #elif defined(CONFIG_ESPTOOLPY_FLASHMODE_DIO)
    return "DIO";
  #elif defined(CONFIG_ESPTOOLPY_FLASHMODE_DOUT)
    return "DOUT";
  #else
    return Texts::unknown.str();
  #endif
}

String getFlashSpeed() {
  #ifdef CONFIG_ESPTOOLPY_FLASHFREQ_80M
    return "80 MHz";
  #elif defined(CONFIG_ESPTOOLPY_FLASHFREQ_40M)
    return "40 MHz";
  #elif defined(CONFIG_ESPTOOLPY_FLASHFREQ_26M)
    return "26 MHz";
  #elif defined(CONFIG_ESPTOOLPY_FLASHFREQ_20M)
    return "20 MHz";
  #else
    return Texts::unknown.str();
  #endif
}

const char* getResetReasonKey() {
  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
    case ESP_RST_POWERON: return "poweron";
    case ESP_RST_SW: return "software_reset";
    case ESP_RST_DEEPSLEEP: return "deepsleep_exit";
    case ESP_RST_BROWNOUT: return "brownout";
    default: return "other";
  }
}

String getResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
    case ESP_RST_POWERON: return Texts::poweron.str();
    case ESP_RST_SW: return Texts::software_reset.str();
    case ESP_RST_DEEPSLEEP: return Texts::deepsleep_exit.str();
    case ESP_RST_BROWNOUT: return Texts::brownout.str();
    default: return Texts::other.str();
  }
}

String formatUptime(unsigned long days, unsigned long hours, unsigned long minutes) {
  // [OPT-002] Optimize uptime formatting: use direct buffer instead of String concatenations
  char uptimeBuf[64];
  char *p = uptimeBuf;
  size_t remaining = sizeof(uptimeBuf) - 1;
  
  if (days > 0) {
    // [OPT-008]: Direct .c_str() call avoids String() allocation
    int written = snprintf(p, remaining, "%lu %s", days, Texts::days.str().c_str());
    if (written > 0) {
      p += written;
      remaining -= written;
    }
  }
  
  if (hours > 0 || (days > 0 && remaining > 1)) {
    if (p != uptimeBuf && remaining > 1) {
      *p++ = ' ';
      remaining--;
    }
    // [OPT-008]: Direct .c_str() call avoids String() allocation
    int written = snprintf(p, remaining, "%lu %s", hours, Texts::hours.str().c_str());
    if (written > 0) {
      p += written;
      remaining -= written;
    }
  }
  
  if (minutes > 0 || uptimeBuf[0] == '\0') {
    if (p != uptimeBuf && remaining > 1) {
      *p++ = ' ';
      remaining--;
    }
    // [OPT-008]: Direct .c_str() call avoids String() allocation
    snprintf(p, remaining, "%lu %s", minutes, Texts::minutes.str().c_str());
  }
  
  return String(uptimeBuf);
}

String getMemoryStatus() {
  float heapUsagePercent = ((float)(diagnosticData.heapSize - diagnosticData.freeHeap) / diagnosticData.heapSize) * 100;
  if (heapUsagePercent < 50) return Texts::excellent.str();
  else if (heapUsagePercent < 70) return Texts::good.str();
  else if (heapUsagePercent < 85) return Texts::warning.str();
  else return Texts::critical.str();
}

const char* getWiFiSignalQualityKey() {
  if (diagnosticData.wifiRSSI >= -50) return "excellent";
  else if (diagnosticData.wifiRSSI >= -60) return "very_good";
  else if (diagnosticData.wifiRSSI >= -70) return "good";
  else if (diagnosticData.wifiRSSI >= -80) return "weak";
  else return "very_weak";
}

String getWiFiSignalQuality() {
  const char* key = getWiFiSignalQualityKey();
  if (key == nullptr) {
    return Texts::unknown.str();
  }
  if (strcmp(key, "excellent") == 0) return Texts::excellent.str();
  if (strcmp(key, "very_good") == 0) return Texts::very_good.str();
  if (strcmp(key, "good") == 0) return Texts::good.str();
  if (strcmp(key, "weak") == 0) return Texts::weak.str();
  if (strcmp(key, "very_weak") == 0) return Texts::very_weak.str();
  return Texts::unknown.str();
}

// Use translation keys for WiFi auth modes
String wifiAuthModeToString(wifi_auth_mode_t mode) {
  switch (mode) {
    case WIFI_AUTH_OPEN: return Texts::wifi_open_auth.str();
#ifdef WIFI_AUTH_WEP
    case WIFI_AUTH_WEP: return "WEP";
#endif
    case WIFI_AUTH_WPA_PSK: return "WPA-PSK";
    case WIFI_AUTH_WPA2_PSK: return "WPA2-PSK";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
#ifdef WIFI_AUTH_WPA2_ENTERPRISE
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-ENT";
#endif
#ifdef WIFI_AUTH_WPA3_PSK
    case WIFI_AUTH_WPA3_PSK: return "WPA3-PSK";
#endif
#ifdef WIFI_AUTH_WPA2_WPA3_PSK
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
#endif
#ifdef WIFI_AUTH_WAPI_PSK
    case WIFI_AUTH_WAPI_PSK: return "WAPI-PSK";
#endif
#ifdef WIFI_AUTH_OWE
    case WIFI_AUTH_OWE: return "OWE";
#endif
    default: return Texts::unknown.str();
  }
}

#if defined(WIFI_CIPHER_TYPE_NONE)
String wifiCipherToString(wifi_cipher_type_t cipher) {
  switch (cipher) {
    case WIFI_CIPHER_TYPE_NONE: return "None";
    case WIFI_CIPHER_TYPE_WEP40: return "WEP40";
    case WIFI_CIPHER_TYPE_WEP104: return "WEP104";
    case WIFI_CIPHER_TYPE_TKIP: return "TKIP";
    case WIFI_CIPHER_TYPE_CCMP: return "CCMP";
    case WIFI_CIPHER_TYPE_TKIP_CCMP: return "TKIP/CCMP";
#ifdef WIFI_CIPHER_TYPE_GCMP
    case WIFI_CIPHER_TYPE_GCMP: return "GCMP";
#endif
#ifdef WIFI_CIPHER_TYPE_GCMP256
    case WIFI_CIPHER_TYPE_GCMP256: return "GCMP256";
#endif
#ifdef WIFI_CIPHER_TYPE_AES_CMAC128
    case WIFI_CIPHER_TYPE_AES_CMAC128: return "CMAC128";
#endif
#ifdef WIFI_CIPHER_TYPE_SMS4
    case WIFI_CIPHER_TYPE_SMS4: return "SMS4";
#endif
    default: return "-";
  }
}
#endif

String wifiBandwidthToString(wifi_second_chan_t secondary) {
  switch (secondary) {
    case WIFI_SECOND_CHAN_NONE: return "20 MHz";
    case WIFI_SECOND_CHAN_ABOVE:
    case WIFI_SECOND_CHAN_BELOW: return "40 MHz";
    default: return "Auto";
  }
}

String wifiChannelToBand(int channel) {
  if (channel <= 0) return "?";
  if (channel <= 14) return "2.4 GHz";
  if (channel >= 180) return "6 GHz";
  return "5 GHz";
}

int wifiChannelToFrequency(int channel) {
  if (channel <= 0) return 0;
  if (channel <= 14) return 2407 + channel * 5;
  if (channel >= 180) return 5950 + channel * 5;
  return 5000 + channel * 5;
}

String wifiPhyModesToString(const wifi_ap_record_t& record) {
  String modes;
  if (record.phy_11b) {
    if (modes.length()) modes += "/";
    modes += "11b";
  }
  if (record.phy_11g) {
    if (modes.length()) modes += "/";
    modes += "11g";
  }
  if (record.phy_11n) {
    if (modes.length()) modes += "/";
    modes += "11n";
  }
#ifdef CONFIG_ESP_WIFI_80211AC_SUPPORT
  if (record.phy_11ac) {
    if (modes.length()) modes += "/";
    modes += "11ac";
  }
#endif
#ifdef CONFIG_ESP_WIFI_80211AX_SUPPORT
  if (record.phy_11ax) {
    if (modes.length()) modes += "/";
    modes += "11ax";
  }
#endif
#ifdef CONFIG_ESP_WIFI_80211BE_SUPPORT
  if (record.phy_11be) {
    if (modes.length()) modes += "/";
    modes += "11be";
  }
#endif
  if (record.phy_lr) {
    if (modes.length()) modes += "/";
    modes += "LR";
  }
  if (modes.length() == 0) {
    modes = "-";
  }
  return modes;
}

String getGPIOList() {
  // [OPT-005] Optimize GPIO list building: use buffer with snprintf instead of String concatenation
  char gpioListBuf[256] = {0};
  int gpios_array[22];
  int numGPIO = 0;

  #ifdef CONFIG_IDF_TARGET_ESP32
    int gpios[] = {0,2,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33};
    numGPIO = 20;
    memcpy(gpios_array, gpios, sizeof(gpios));
  #elif defined(CONFIG_IDF_TARGET_ESP32S2)
    int gpios[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
    numGPIO = 22;
    memcpy(gpios_array, gpios, sizeof(gpios));
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    int gpios[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
    numGPIO = 22;
    memcpy(gpios_array, gpios, sizeof(gpios));
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    int gpios[] = {0,1,2,3,4,5,6,7,8,9,10};
    numGPIO = 11;
    memcpy(gpios_array, gpios, sizeof(gpios));
  #else
    int gpios[] = {0,2,4,5};
    numGPIO = 4;
    memcpy(gpios_array, gpios, sizeof(gpios));
  #endif
  
  char *p = gpioListBuf;
  for (int i = 0; i < numGPIO && (size_t)(p - gpioListBuf) < sizeof(gpioListBuf) - 1; i++) {
    if (i > 0 && (size_t)(p - gpioListBuf) < sizeof(gpioListBuf) - 2) {
      *p++ = ',';
      *p++ = ' ';
    }
    p += snprintf(p, sizeof(gpioListBuf) - (p - gpioListBuf), "%d", gpios_array[i]);
  }
  
  return String(gpioListBuf);
}

int countGPIO() {
  #ifdef CONFIG_IDF_TARGET_ESP32
    return 26;
  #elif defined(CONFIG_IDF_TARGET_ESP32S2)
    return 37;
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    return 39;
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    return 15;
  #else
    return 20;
  #endif
}

// ========== MÉMOIRE DÉTAILLÉE ==========
bool testSRAMQuick() {
  const size_t testSize = 1024;
  uint8_t* testBuffer = (uint8_t*)malloc(testSize);
  if (!testBuffer) return false;
  
  memset(testBuffer, 0xAA, testSize);
  bool ok = (testBuffer[0] == 0xAA && testBuffer[testSize-1] == 0xAA);
  free(testBuffer);
  return ok;
}

bool testPSRAMQuick() {
  if (ESP.getPsramSize() == 0) return false;
  
  const size_t testSize = 1024;
  uint8_t* testBuffer = (uint8_t*)heap_caps_malloc(testSize, MALLOC_CAP_SPIRAM);
  if (!testBuffer) return false;
  
  memset(testBuffer, 0x55, testSize);
  bool ok = (testBuffer[0] == 0x55 && testBuffer[testSize-1] == 0x55);
  heap_caps_free(testBuffer);
  return ok;
}

void collectDetailedMemory() {
  uint32_t flashSizeReal;
  esp_flash_get_size(NULL, &flashSizeReal);
  
  detailedMemory.flashSizeChip = flashSizeReal;
  detailedMemory.flashSizeReal = ESP.getFlashChipSize();
  
  detailedMemory.psramTotal = ESP.getPsramSize();
  detailedMemory.psramAvailable = (detailedMemory.psramTotal > 0);

  detailedMemory.psramConfigured = false;
  detailedMemory.psramBoardSupported = false;
  detailedMemory.psramType = "PSRAM";

  #if defined(CONFIG_SPIRAM)
    detailedMemory.psramConfigured = true;
    detailedMemory.psramBoardSupported = true;
  #endif
  #if defined(CONFIG_SPIRAM_SUPPORT)
    detailedMemory.psramConfigured = true;
    detailedMemory.psramBoardSupported = true;
  #endif
  #if defined(BOARD_HAS_PSRAM)
    detailedMemory.psramConfigured = true;
    detailedMemory.psramBoardSupported = true;
  #endif
  #if defined(CONFIG_SPIRAM_MODE_OCT)
    detailedMemory.psramConfigured = true;
    detailedMemory.psramBoardSupported = true;
    detailedMemory.psramType = "OPI";
  #endif
  #if defined(CONFIG_SPIRAM_MODE_QUAD)
    detailedMemory.psramConfigured = true;
    detailedMemory.psramBoardSupported = true;
    detailedMemory.psramType = "QSPI";
  #endif

  if (detailedMemory.psramTotal == 0) {
    detailedMemory.psramConfigured = false;
  }
  
  if (detailedMemory.psramAvailable) {
    detailedMemory.psramFree = ESP.getFreePsram();
    detailedMemory.psramUsed = detailedMemory.psramTotal - detailedMemory.psramFree;
    
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    detailedMemory.psramLargestBlock = info.largest_free_block;
  } else {
    detailedMemory.psramFree = 0;
    detailedMemory.psramUsed = 0;
    detailedMemory.psramLargestBlock = 0;
  }
  
  multi_heap_info_t infoInternal;
  heap_caps_get_info(&infoInternal, MALLOC_CAP_INTERNAL);
  detailedMemory.sramTotal = infoInternal.total_free_bytes + infoInternal.total_allocated_bytes;
  detailedMemory.sramFree = infoInternal.total_free_bytes;
  detailedMemory.sramUsed = infoInternal.total_allocated_bytes;
  detailedMemory.sramLargestBlock = infoInternal.largest_free_block;
  
  size_t largestBlock = ESP.getMaxAllocHeap();
  size_t freeHeap = ESP.getFreeHeap();
  if (freeHeap > 0) {
    detailedMemory.fragmentationPercent = 100.0 - (100.0 * largestBlock / freeHeap);
  } else {
    detailedMemory.fragmentationPercent = 0;
  }
  
  detailedMemory.sramTestPassed = testSRAMQuick();
  detailedMemory.psramTestPassed = testPSRAMQuick();
  
  if (detailedMemory.fragmentationPercent < 20) {
    detailedMemory.memoryStatus = Texts::excellent.str();
  } else if (detailedMemory.fragmentationPercent < 40) {
    detailedMemory.memoryStatus = Texts::good.str();
  } else if (detailedMemory.fragmentationPercent < 60) {
    detailedMemory.memoryStatus = "Moyen"; // Pas traduit (statut technique)
  } else {
    detailedMemory.memoryStatus = Texts::critical.str();
  }
}

void printPSRAMDiagnostic() {
  Serial.println("\r\n=== DIAGNOSTIC PSRAM DETAILLE ===");
  Serial.printf("ESP.getPsramSize(): %u octets (%.2f MB)\r\n", 
                ESP.getPsramSize(), ESP.getPsramSize() / 1048576.0);
  
  Serial.println("\r\nFlags de compilation detectes (indication du type supporte par la carte):");
  bool anyFlag = false;
  String psramType = detailedMemory.psramType ? detailedMemory.psramType : Texts::unknown.str();
  
  #ifdef CONFIG_SPIRAM
    Serial.println("  ✓ CONFIG_SPIRAM");
    anyFlag = true;
  #endif
  #ifdef CONFIG_SPIRAM_SUPPORT
    Serial.println("  ✓ CONFIG_SPIRAM_SUPPORT");
    anyFlag = true;
  #endif
  #ifdef BOARD_HAS_PSRAM
    Serial.println("  ✓ BOARD_HAS_PSRAM");
    anyFlag = true;
  #endif
  #ifdef CONFIG_SPIRAM_MODE_OCT
    Serial.println("  ✓ CONFIG_SPIRAM_MODE_OCT (OPI) - Type supporte par la carte");
    psramType = "OPI";
    anyFlag = true;
  #endif
  #ifdef CONFIG_SPIRAM_MODE_QUAD
    Serial.println("  ✓ CONFIG_SPIRAM_MODE_QUAD (QSPI) - Type supporte par la carte");
    psramType = "QSPI";
    anyFlag = true;
  #endif
  
  if (!anyFlag) {
    Serial.println("  ✗ Aucun flag PSRAM detecte");
  }
  
  Serial.println("\r\nResultat logique:");
  Serial.printf("  psramTotal = %u octets\r\n", detailedMemory.psramTotal);
  Serial.printf("  psramConfigured = %s\r\n", detailedMemory.psramConfigured ? "TRUE" : "FALSE");
  Serial.printf("  psramAvailable = %s\r\n", detailedMemory.psramAvailable ? "TRUE" : "FALSE");
  
  Serial.println("\r\nConclusion:");
  if (ESP.getPsramSize() > 0) {
    Serial.printf("  → PSRAM fonctionnelle! Type: %s\r\n", psramType.c_str());
  } else if (anyFlag) {
    Serial.printf("  → Carte compatible PSRAM %s mais DESACTIVEE dans IDE\r\n", psramType.c_str());
    Serial.println("  → Pour activer: Tools → PSRAM → OPI PSRAM (ou QSPI)");
  } else {
    Serial.println("  → Carte sans support PSRAM");
  }
  Serial.println("=====================================\r\n");
}

// ========== BENCHMARKS ==========
unsigned long benchmarkCPU() {
  Serial.println("\r\n=== BENCHMARK CPU ===");
  unsigned long start = micros();
  
  volatile float result = 0;
  for (int i = 0; i < 100000; i++) {
    result += sqrt(i) * sin(i);
  }

  unsigned long duration = micros() - start;
  Serial.printf("CPU: %lu us (%.2f MFLOPS)\r\n", duration, 100000.0 / duration);
  return duration;
}

unsigned long benchmarkMemory() {
  Serial.println("=== BENCHMARK MEMOIRE ===");
  unsigned long start = micros();

  const int size = 10000;
  int* testArray = (int*)malloc(size * sizeof(int));
  if (!testArray) return 0;

  for (int i = 0; i < size; i++) testArray[i] = i;
  volatile int sum = 0;
  for (int i = 0; i < size; i++) sum += testArray[i];
  
  free(testArray);
  unsigned long duration = micros() - start;
  Serial.printf("Memory: %lu us\r\n", duration);
  return duration;
}

// ========== SCAN I2C ==========
void ensureI2CBusConfigured() {
  Wire.end();
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 3, 0)
  Wire.setPins(i2c_sda, i2c_scl);
  Wire.begin();
#else
  Wire.begin(i2c_sda, i2c_scl);
#endif
  Wire.setClock(400000);
}

void scanI2C() {
  if (!ENABLE_I2C_SCAN) return;

  Serial.println("\r\n=== SCAN I2C ===");
  ensureI2CBusConfigured();
  Serial.printf("I2C: SDA=%d, SCL=%d\r\n", i2c_sda, i2c_scl);
  
  diagnosticData.i2cDevices = "";
  diagnosticData.i2cCount = 0;
  
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      char addr[6];
      sprintf(addr, "0x%02X", address);
      if (diagnosticData.i2cCount > 0) diagnosticData.i2cDevices += ", ";
      diagnosticData.i2cDevices += String(addr);
      diagnosticData.i2cCount++;
    }
  }
  
  if (diagnosticData.i2cCount == 0) {
    diagnosticData.i2cDevices = "Aucun";
  }
  Serial.printf("I2C: %d peripherique(s)\r\n", diagnosticData.i2cCount);
}

// ========== SCAN WIFI ==========
void scanWiFiNetworks() {
  Serial.println("\r\n=== SCAN WIFI ===");
  wifiNetworks.clear();

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 3, 0)
  int n = WiFi.scanNetworks(false, true, true);
#else
  int n = WiFi.scanNetworks();
#endif

  if (n < 0) {
    Serial.println("WiFi: scan en echec");
    return;
  }

  for (int i = 0; i < n; i++) {
    WiFiNetwork net;

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 3, 0)
    wifi_ap_record_t* info = static_cast<wifi_ap_record_t*>(WiFi.getScanInfoByIndex(i));
    if (info != nullptr) {
      net.ssid = String(reinterpret_cast<const char*>(info->ssid));
      net.rssi = info->rssi;
      net.channel = info->primary;

      char bssidStr[18];
      sprintf(bssidStr, "%02X:%02X:%02X:%02X:%02X:%02X",
              info->bssid[0], info->bssid[1], info->bssid[2], info->bssid[3], info->bssid[4], info->bssid[5]);
      net.bssid = String(bssidStr);

      net.encryption = wifiAuthModeToString(info->authmode);
#if defined(WIFI_CIPHER_TYPE_NONE)
      String pairwise = wifiCipherToString(info->pairwise_cipher);
      String group = wifiCipherToString(info->group_cipher);
      if ((pairwise != "-" && pairwise != "None") || (group != "-" && group != "None")) {
        net.encryption += " (" + pairwise + "/" + group + ")";
      }
#endif
      net.bandwidth = wifiBandwidthToString(info->second);
      net.band = wifiChannelToBand(info->primary);
      net.freqMHz = wifiChannelToFrequency(info->primary);
      net.phyModes = wifiPhyModesToString(*info);
    } else
#endif
    {
      net.ssid = WiFi.SSID(i);
      net.rssi = WiFi.RSSI(i);
      net.channel = WiFi.channel(i);

      uint8_t* bssid = WiFi.BSSID(i);
      char bssidFallback[18];
      sprintf(bssidFallback, "%02X:%02X:%02X:%02X:%02X:%02X",
              bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
      net.bssid = String(bssidFallback);

      wifi_auth_mode_t auth = WiFi.encryptionType(i);
      net.encryption = wifiAuthModeToString(auth);
      net.band = wifiChannelToBand(net.channel);
      net.bandwidth = "-";
      net.freqMHz = wifiChannelToFrequency(net.channel);
      net.phyModes = "-";
    }

    if (net.bandwidth.length() == 0) net.bandwidth = "-";
    wifiNetworks.push_back(net);
  }

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 3, 0)
  WiFi.scanDelete();
#endif

  Serial.printf("WiFi: %d reseaux trouves\r\n", n);
}

// ========== ENCODEUR ROTATIF - ISR ==========
void IRAM_ATTR rotaryISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastRotaryInterruptTime < rotaryDebounceDelay) return;
  lastRotaryInterruptTime = currentTime;

  int clkState = digitalRead(rotary_clk_pin);
  int dtState = digitalRead(rotary_dt_pin);

  if (clkState != lastRotaryState) {
    if (dtState != clkState) {
      rotaryPosition++;
    } else {
      rotaryPosition--;
    }
  }
  lastRotaryState = clkState;
}

void IRAM_ATTR rotaryButtonISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonPressTime < buttonDebounceDelay) return;
  lastButtonPressTime = currentTime;

  if (digitalRead(rotary_sw_pin) == LOW) {
    rotaryButtonPressed = true;
  }
}

// ========== TEST GPIO ==========
bool testSingleGPIO(int pin) {
  if (pin == 1 || pin == 3) return false;
  if (pin >= 6 && pin <= 11) return false;
  
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  delayMicroseconds(250);
  bool highOk = digitalRead(pin) == HIGH;

  digitalWrite(pin, LOW);
  delayMicroseconds(250);
  bool lowOk = digitalRead(pin) == LOW;
  
  pinMode(pin, INPUT);
  return highOk && lowOk;
}

void testAllGPIOs() {
  Serial.println("\r\n=== TEST GPIO ===");
  gpioResults.clear();
  
  #ifdef CONFIG_IDF_TARGET_ESP32
    int gpios[] = {0,2,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33};
    int numGPIO = 20;
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    int gpios[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
    int numGPIO = 22;
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    int gpios[] = {0,1,2,3,4,5,6,7,8,9,10};
    int numGPIO = 11;
  #else
    int gpios[] = {0,2,4,5};
    int numGPIO = 4;
  #endif
  
  for (int i = 0; i < numGPIO; i++) {
    GPIOTestResult result;
    result.pin = gpios[i];
    result.tested = true;
    result.working = testSingleGPIO(gpios[i]);
    result.mode = "Digital I/O";
    // [OPT-009]: Use pre-allocated constants instead of String(Texts::ok/fail)
    result.notes = result.working ? OK_STR : FAIL_STR;
    gpioResults.push_back(result);
  }
  Serial.printf("GPIO: %d testes\r\n", numGPIO);
}

// ========== LED INTÉGRÉE ==========
void detectBuiltinLED() {
  String chipModel = detectChipModel();
  
  #ifdef LED_BUILTIN
    BUILTIN_LED_PIN = LED_BUILTIN;
  #else
    if (chipModel == "ESP32-S3") BUILTIN_LED_PIN = 2;
    else if (chipModel == "ESP32-C3") BUILTIN_LED_PIN = 8;
    else if (chipModel == "ESP32-S2") BUILTIN_LED_PIN = 15;
    else BUILTIN_LED_PIN = 2;
  #endif
  
  // [OPT-008]: Buffer-based test result initialization (1 vs 4 allocations)
  char ledBuf[96];
  snprintf(ledBuf, sizeof(ledBuf), "%s %d - %s", Texts::gpio.str().c_str(), BUILTIN_LED_PIN, Texts::not_tested.str().c_str());
  builtinLedTestResult = String(ledBuf);
  Serial.printf("LED integree: GPIO %d\r\n", BUILTIN_LED_PIN);
}

void testBuiltinLED() {
  if (BUILTIN_LED_PIN == -1) return;
  
  Serial.println("\r\n=== TEST LED ===");
  pinMode(BUILTIN_LED_PIN, OUTPUT);

  for (int i = 0; i < 5; i++) {
    digitalWrite(BUILTIN_LED_PIN, HIGH);
    delay(80);
    digitalWrite(BUILTIN_LED_PIN, LOW);
    delay(80);
  }

  for (int i = 0; i <= 255; i += 51) {
    analogWrite(BUILTIN_LED_PIN, i);
    delay(25);
    yield();
  }
  for (int i = 255; i >= 0; i -= 51) {
    analogWrite(BUILTIN_LED_PIN, i);
    delay(25);
    yield();
  }
  
  digitalWrite(BUILTIN_LED_PIN, LOW);
  builtinLedAvailable = true;
  // [OPT-009]: Buffer-based test result (1 vs 4 allocations)
  char ledBuf[96];
  snprintf(ledBuf, sizeof(ledBuf), "%s %s - GPIO %d", Texts::test.str().c_str(), Texts::ok.str().c_str(), BUILTIN_LED_PIN);
  builtinLedTestResult = String(ledBuf);
  builtinLedTested = true;
  Serial.println("LED: OK");
}

void resetBuiltinLEDTest() {
  builtinLedTested = false;
  builtinLedAvailable = false;
  if (BUILTIN_LED_PIN != -1) digitalWrite(BUILTIN_LED_PIN, LOW);
}

// ========== NEOPIXEL ==========
void detectNeoPixelSupport() {
  String chipModel = detectChipModel();
  
  if (CUSTOM_LED_PIN != -1) {
    LED_PIN = CUSTOM_LED_PIN;
    neopixelSupported = true;
  }
  else if (chipModel == "ESP32-S3") {
    LED_PIN = 48;
    neopixelSupported = true;
  } 
  else if (chipModel == "ESP32-C3") {
    LED_PIN = 8;
    neopixelSupported = true;
  }
  else {
    LED_PIN = 2;
    neopixelSupported = false;
  }
  
  if (strip != nullptr) delete strip;
  strip = new Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
  // [OPT-008]: Buffer-based test result initialization (1 vs 4 allocations)
  char neoBuf[96];
  snprintf(neoBuf, sizeof(neoBuf), "%s %d - %s", Texts::gpio.str().c_str(), LED_PIN, Texts::not_tested.str().c_str());
  neopixelTestResult = String(neoBuf);
  Serial.printf("NeoPixel: GPIO %d\r\n", LED_PIN);
}

void testNeoPixel() {
  if (!strip) return;

  Serial.println("\r\n=== TEST NEOPIXEL ===");
  strip->begin();
  strip->clear();
  strip->show();

  strip->setPixelColor(0, strip->Color(255, 0, 0));
  strip->show();
  delay(160);

  strip->setPixelColor(0, strip->Color(0, 255, 0));
  strip->show();
  delay(160);

  strip->setPixelColor(0, strip->Color(0, 0, 255));
  strip->show();
  delay(160);

  for (int i = 0; i < 256; i += 64) {
    strip->setPixelColor(0, strip->gamma32(strip->ColorHSV(i * 256)));
    strip->show();
    delay(28);
    yield();
  }

  strip->clear();
  strip->show();

  neopixelAvailable = true;
  // [OPT-009]: Buffer-based test result (1 vs 4 allocations)
  char neoBuf[96];
  snprintf(neoBuf, sizeof(neoBuf), "%s %s - GPIO %d", Texts::test.str().c_str(), Texts::ok.str().c_str(), LED_PIN);
  neopixelTestResult = String(neoBuf);
  neopixelTested = true;
  Serial.println("NeoPixel: OK");
}

void resetNeoPixelTest() {
  neopixelTested = false;
  neopixelAvailable = false;
  if (strip) {
    strip->clear();
    strip->show();
  }
}

static inline bool neopixelReady() {
  return strip != nullptr;
}

void neopixelPauseStatus() {
  neopixelStatusPaused = true;
}

void neopixelResumeStatus() {
  neopixelStatusPaused = false;
  neopixelStatusKnown = false;
  neopixelConnecting = false;
}

void neopixelShowConnecting() {
  if (!neopixelReady() || neopixelStatusPaused) return;
  neopixelConnecting = true;
  strip->setBrightness(60);
  strip->setPixelColor(0, strip->Color(50, 50, 0)); // Jaune fixe pendant la connexion
  strip->show();
}

void neopixelSetWifiState(bool connected) {
  if (!neopixelReady() || neopixelStatusPaused) return;
  neopixelConnecting = false;
  neopixelLastWifiConnected = connected;
  neopixelStatusKnown = true;
  neopixelHeartbeatPreviousMillis = millis();
  neopixelHeartbeatState = false;
  strip->setBrightness(60);
  strip->setPixelColor(0, connected ? strip->Color(0, 50, 0) : strip->Color(50, 0, 0));
  strip->show();
}

void updateNeoPixelWifiStatus() {
  if (!neopixelReady() || neopixelStatusPaused) return;
  if (neopixelConnecting) return; // Couleur fixe tant que la connexion est en cours

  bool connected = (WiFi.status() == WL_CONNECTED);

  if (!neopixelStatusKnown || connected != neopixelLastWifiConnected) {
    neopixelSetWifiState(connected);
    return;
  }

  unsigned long now = millis();
  if (now - neopixelHeartbeatPreviousMillis >= NEOPIXEL_HEARTBEAT_INTERVAL_MS) {
    neopixelHeartbeatPreviousMillis = now;
    neopixelHeartbeatState = !neopixelHeartbeatState;
    uint32_t color = connected
      ? (neopixelHeartbeatState ? strip->Color(0, 50, 0) : strip->Color(0, 10, 0))
      : (neopixelHeartbeatState ? strip->Color(50, 0, 0) : strip->Color(10, 0, 0));
    strip->setPixelColor(0, color);
    strip->show();
  }
}

void neopixelRestoreWifiStatus() {
  neopixelResumeStatus();
  updateNeoPixelWifiStatus();
}

void neopixelShowRebootFlash() {
  if (!neopixelReady()) return;
  neopixelStatusPaused = true;
  strip->setBrightness(60);
  strip->setPixelColor(0, strip->Color(255, 0, 255)); // Violet
  strip->show();
}

void neopixelRainbow() {
  if (!strip) return;
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < LED_COUNT; j++) {
      strip->setPixelColor(j, strip->gamma32(strip->ColorHSV(i * 256)));
    }
    strip->show();
    delay(10);
  }
}

void neopixelBlink(uint32_t color, int times) {
  if (!strip) return;
  for (int i = 0; i < times; i++) {
    strip->fill(color);
    strip->show();
    delay(120);
    strip->clear();
    strip->show();
    delay(120);
  }
}

void neopixelFade(uint32_t color) {
  if (!strip) return;
  for (int brightness = 0; brightness <= 255; brightness += 5) {
    strip->setBrightness(brightness);
    strip->fill(color);
    strip->show();
    delay(20);
  }
  for (int brightness = 255; brightness >= 0; brightness -= 5) {
    strip->setBrightness(brightness);
    strip->fill(color);
    strip->show();
    delay(20);
  }
  strip->setBrightness(255);
}

void neopixelChase() {
  if (!strip) return;
  uint32_t colors[] = {
    strip->Color(255, 0, 0),
    strip->Color(0, 255, 0),
    strip->Color(0, 0, 255),
    strip->Color(255, 255, 0),
    strip->Color(255, 0, 255),
    strip->Color(0, 255, 255)
  };
  int numColors = sizeof(colors) / sizeof(colors[0]);

  for (int cycle = 0; cycle < 3; cycle++) {
    for (int colorIndex = 0; colorIndex < numColors; colorIndex++) {
      for (int pos = 0; pos < LED_COUNT; pos++) {
        strip->clear();
        for (int i = 0; i < LED_COUNT; i++) {
          if ((i + pos) % 3 == 0) {
            strip->setPixelColor(i, colors[colorIndex]);
          }
        }
        strip->show();
        delay(100);
      }
    }
  }
  strip->clear();
  strip->show();
}

void applyOLEDOrientation() {
  const u8g2_cb_t *rotation = U8G2_R0;
  switch(oledRotation & 0x03) {
    case 0: rotation = U8G2_R0; break;
    case 1: rotation = U8G2_R1; break;
    case 2: rotation = U8G2_R2; break;
    case 3: rotation = U8G2_R3; break;
  }
  oled.setDisplayRotation(rotation);
}

void detectOLED() {
  Serial.println("\r\n=== DETECTION OLED ===");
  ensureI2CBusConfigured();
  Serial.printf("I2C: SDA=%d, SCL=%d\r\n", i2c_sda, i2c_scl);

  Wire.beginTransmission(SCREEN_ADDRESS);
  bool i2cDetected = (Wire.endTransmission() == 0);

  if(i2cDetected && oled.begin()) {
    oledAvailable = true;
    applyOLEDOrientation();
    oledTestResult = String(Texts::detected) + " @ 0x" + String(SCREEN_ADDRESS, HEX);
    Serial.println("OLED: Detecte!\r\n");
  } else {
    oledAvailable = false;
    if (i2cDetected) {
      // [OPT-009]: Buffer-based fail message (1 vs 2 allocations)
      char failBuf[96];
      snprintf(failBuf, sizeof(failBuf), "%s - %s", Texts::i2c_peripherals.str().c_str(), Texts::fail.str().c_str());
      oledTestResult = String(failBuf);
    } else {
      // [OPT-009]: Buffer-based not_detected message (1 vs 4 allocations)
      char notDetBuf[128];
      snprintf(notDetBuf, sizeof(notDetBuf), "%s (SDA:%d SCL:%d)", Texts::not_detected.str().c_str(), i2c_sda, i2c_scl);
      oledTestResult = String(notDetBuf);
    }
    Serial.println("OLED: Non detecte\r\n");
  }
}

void oledStepWelcome() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 10, "TEST OLED 0.96\"");
  oled.drawStr(0, 30, "128x64 pixels");
  char buf[32];
  snprintf(buf, sizeof(buf), "I2C: 0x%02X", SCREEN_ADDRESS);
  oled.drawStr(0, 45, buf);
  snprintf(buf, sizeof(buf), "SDA:%d SCL:%d", i2c_sda, i2c_scl);
  oled.drawStr(0, 60, buf);
  oled.sendBuffer();
  delay(700);
}

void oledStepBigText() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB14_tr);
  oled.drawStr(20, 35, "ESP32");
  oled.sendBuffer();
  delay(450);
}

void oledStepTextSizes() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 10, "Taille 1");
  oled.setFont(u8g2_font_ncenB14_tr);
  oled.drawStr(0, 30, "Taille 2");
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 50, "Retour taille 1");
  oled.sendBuffer();
  delay(550);
}

void oledStepShapes() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  oled.drawFrame(10, 10, 30, 20);
  oled.drawBox(50, 10, 30, 20);
  oled.drawCircle(25, 50, 10);
  oled.drawDisc(65, 50, 10);
  oled.drawTriangle(95, 30, 85, 10, 105, 10);
  oled.sendBuffer();
  delay(550);
}

void oledStepHorizontalLines() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  for (int i = 0; i < SCREEN_HEIGHT; i += 4) {
    oled.drawLine(0, i, SCREEN_WIDTH - 1, i);
  }
  oled.sendBuffer();
  delay(350);
}

void oledStepDiagonals() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  for (int i = 0; i < SCREEN_WIDTH; i += 8) {
    oled.drawLine(0, 0, i, SCREEN_HEIGHT - 1);
    oled.drawLine(SCREEN_WIDTH - 1, 0, i, SCREEN_HEIGHT - 1);
  }
  oled.sendBuffer();
  delay(350);
}

void oledStepMovingSquare() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  for (int x = 0; x < SCREEN_WIDTH - 20; x += 6) {
    oled.clearBuffer();
    oled.drawBox(x, 22, 20, 20);
    oled.sendBuffer();
    delay(12);
    yield();
  }
}

void oledStepProgressBar() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  String loadingText = String(Texts::loading);
  oled.drawStr(20, 15, loadingText.c_str());
  oled.sendBuffer();

  for (int i = 0; i <= 100; i += 10) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(20, 15, loadingText.c_str());
    oled.drawFrame(10, 30, 108, 15);
    oled.drawBox(12, 32, i, 11);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", i);
    oled.drawStr(45, 55, buf);
    oled.sendBuffer();
    delay(45);
    yield();
  }
  delay(300);
}

void oledStepScrollText() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  String scrollText = "  DIAGNOSTIC ESP32 COMPLET - OLED 0.96 pouces I2C  ";
  int textWidth = scrollText.length() * 6;
  for (int offset = 0; offset < textWidth; offset += 6) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    oled.setCursor(-offset, 35);
    oled.print(scrollText);
    oled.sendBuffer();
    delay(12);
    yield();
  }
}

void oledStepFinalMessage() {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(30, 30, "TEST OK!");
  oled.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  oled.sendBuffer();
  delay(600);
  oled.clearBuffer();
  oled.sendBuffer();
}

bool performOLEDStep(const String &stepId) {
  if (!oledAvailable) {
    return false;
  }

  if (stepId == "welcome") {
    oledStepWelcome();
  } else if (stepId == "big_text") {
    oledStepBigText();
  } else if (stepId == "text_sizes") {
    oledStepTextSizes();
  } else if (stepId == "shapes") {
    oledStepShapes();
  } else if (stepId == "horizontal_lines") {
    oledStepHorizontalLines();
  } else if (stepId == "diagonals") {
    oledStepDiagonals();
  } else if (stepId == "moving_square") {
    oledStepMovingSquare();
  } else if (stepId == "progress_bar") {
    oledStepProgressBar();
  } else if (stepId == "scroll_text") {
    oledStepScrollText();
  } else if (stepId == "final_message") {
    oledStepFinalMessage();
  } else {
    return false;
  }

  return true;
}

String getOLEDStepLabel(const String &stepId) {
  if (stepId == "welcome") return String(Texts::oled_step_welcome);
  if (stepId == "big_text") return String(Texts::oled_step_big_text);
  if (stepId == "text_sizes") return String(Texts::oled_step_text_sizes);
  if (stepId == "shapes") return String(Texts::oled_step_shapes);
  if (stepId == "horizontal_lines") return String(Texts::oled_step_horizontal_lines);
  if (stepId == "diagonals") return String(Texts::oled_step_diagonals);
  if (stepId == "moving_square") return String(Texts::oled_step_moving_square);
  if (stepId == "progress_bar") return String(Texts::oled_step_progress_bar);
  if (stepId == "scroll_text") return String(Texts::oled_step_scroll_text);
  if (stepId == "final_message") return String(Texts::oled_step_final_message);
  return stepId;
}

void testOLED() {
  if (!oledAvailable || oledTested) return;

  Serial.println("\r\n=== TEST OLED ===");

  oledStepWelcome();
  oledStepBigText();
  oledStepTextSizes();
  oledStepShapes();
  oledStepHorizontalLines();
  oledStepDiagonals();
  oledStepMovingSquare();
  oledStepProgressBar();
  oledStepScrollText();
  oledStepFinalMessage();

  oledTested = true;
  // [OPT-009]: Buffer-based test result (1 vs 3 allocations)
  char oledBuf[96];
  snprintf(oledBuf, sizeof(oledBuf), "%s %s - 128x64", Texts::test.str().c_str(), Texts::ok.str().c_str());
  oledTestResult = String(oledBuf);
  Serial.println("OLED: Tests complets OK\r\n");
}

void resetOLEDTest() {
  oledTested = false;
  if (oledAvailable) {
    oled.clearBuffer();
    oled.sendBuffer();
  }
}

void oledShowMessage(String message) {
  if (!oledAvailable) return;
  applyOLEDOrientation();
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 10, message.c_str());
  oled.sendBuffer();
}

// WiFi connection status banner on OLED
void oledShowWiFiStatus(const String& title,
                        const String& detail,
                        const String& footer,
                        int progressPercent) {
  if (!oledAvailable) {
    return;
  }

  int clampedProgress = progressPercent;
  if (clampedProgress < 0) {
    clampedProgress = -1;
  } else if (clampedProgress > 100) {
    clampedProgress = 100;
  }

  applyOLEDOrientation();
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(0, 12, title.c_str());

  if (detail.length() > 0) {
    oled.drawStr(0, 28, detail.c_str());
  }

  if (footer.length() > 0) {
    oled.drawStr(0, 44, footer.c_str());
  }

  if (clampedProgress >= 0) {
    const int barX = 8;
    const int barY = 52;
    const int barWidth = SCREEN_WIDTH - (barX * 2);
    const int barHeight = 10;
    oled.drawFrame(barX, barY, barWidth, barHeight);
    int fillWidth = (barWidth - 2) * clampedProgress / 100;
    if (fillWidth > 0) {
      oled.drawBox(barX + 1, barY + 1, fillWidth, barHeight - 2);
    }
  }

  oled.sendBuffer();
}

// ========== TEST TFT ==========
// Individual TFT test steps
void tftStepBoot() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;
  displayBootSplash();
  delay(1000);
#endif
}

void tftStepColors() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;
  
  uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_WHITE};
  const char* names[] = {"RED", "GREEN", "BLUE", "YELLOW", "CYAN", "MAGENTA", "WHITE"};
  
  for (int i = 0; i < 7; i++) {
    tft->fillScreen(colors[i]);
    tft->setTextColor(TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(60, 110);
    tft->print(names[i]);
    delay(500);
  }
#endif
}

void tftStepShapes() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;
  
  tft->fillScreen(TFT_BLACK);
  
  // Rectangles
  tft->drawRect(20, 20, 80, 60, TFT_RED);
  tft->fillRect(140, 20, 80, 60, TFT_GREEN);
  
  // Circles
  tft->drawCircle(60, 140, 30, TFT_BLUE);
  tft->fillCircle(180, 140, 30, TFT_YELLOW);
  
  // Triangles
  tft->drawTriangle(60, 180, 40, 220, 80, 220, TFT_CYAN);
  tft->fillTriangle(180, 180, 160, 220, 200, 220, TFT_MAGENTA);
  
  delay(1500);
#endif
}

void tftStepText() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;
  
  tft->fillScreen(TFT_BLACK);
  
  tft->setTextColor(TFT_WHITE);
  tft->setTextSize(1);
  tft->setCursor(10, 10);
  tft->println("Text Size 1");
  
  tft->setTextSize(2);
  tft->setCursor(10, 40);
  tft->println("Size 2");
  
  tft->setTextSize(3);
  tft->setCursor(10, 80);
  tft->println("Size 3");
  
  tft->setTextColor(TFT_CYAN);
  tft->setTextSize(2);
  tft->setCursor(10, 140);
  tft->println("Colored Text");
  
  delay(1500);
#endif
}

void tftStepLines() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;
  
  tft->fillScreen(TFT_BLACK);
  
  // Horizontal lines
  for (int y = 0; y < TFT_HEIGHT; y += 10) {
    tft->drawFastHLine(0, y, TFT_WIDTH, TFT_CYAN);
  }
  delay(700);
  
  tft->fillScreen(TFT_BLACK);
  
  // Vertical lines
  for (int x = 0; x < TFT_WIDTH; x += 10) {
    tft->drawFastVLine(x, 0, TFT_HEIGHT, TFT_MAGENTA);
  }
  delay(700);
  
  tft->fillScreen(TFT_BLACK);
  
  // Diagonal lines
  for (int i = 0; i < TFT_WIDTH; i += 20) {
    tft->drawLine(0, 0, i, TFT_HEIGHT - 1, TFT_YELLOW);
    tft->drawLine(TFT_WIDTH - 1, 0, i, TFT_HEIGHT - 1, TFT_GREEN);
  }
  delay(700);
#endif
}

void tftStepAnimation() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;
  
  tft->fillScreen(TFT_BLACK);
  
  // Moving square
  int squareSize = 30;
  for (int x = 0; x <= TFT_WIDTH - squareSize; x += 5) {
    tft->fillRect(x, (TFT_HEIGHT - squareSize) / 2, squareSize, squareSize, TFT_BLUE);
    delay(20);
    if (x + squareSize < TFT_WIDTH) {
      tft->fillRect(x, (TFT_HEIGHT - squareSize) / 2, squareSize, squareSize, TFT_BLACK);
    }
    yield();
  }
  
  delay(500);
#endif
}

void tftStepProgressBar() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;
  
  tft->fillScreen(TFT_BLACK);
  
  tft->setTextColor(TFT_WHITE);
  tft->setTextSize(2);
  tft->setCursor(60, 40);
  tft->print("Loading...");
  
  int barX = 30;
  int barY = 100;
  int barWidth = 180;
  int barHeight = 30;
  
  tft->drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
  
  for (int i = 0; i <= 100; i += 5) {
    int fillWidth = (barWidth - 4) * i / 100;
    tft->fillRect(barX + 2, barY + 2, fillWidth, barHeight - 4, TFT_GREEN);
    
    tft->fillRect(40, 150, 160, 20, TFT_BLACK);
    tft->setTextColor(TFT_CYAN);
    tft->setTextSize(2);
    tft->setCursor(90, 155);
    tft->print(i);
    tft->print("%");
    
    delay(30);
    yield();
  }
  
  delay(500);
#endif
}

void tftStepFinal() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;
  
  tft->fillScreen(TFT_BLACK);
  
  tft->setTextColor(TFT_GREEN);
  tft->setTextSize(3);
  tft->setCursor(40, 80);
  tft->println("TEST");
  tft->setCursor(20, 120);
  tft->println("COMPLETE!");
  
  // Success indicator
  tft->fillCircle(120, 180, 20, TFT_GREEN);
  tft->setTextColor(TFT_BLACK);
  tft->setTextSize(3);
  tft->setCursor(108, 172);
  tft->print("OK");
  
  delay(2000);
#endif
}

bool performTFTStep(const String &stepId) {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) {
    return false;
  }

  if (stepId == "boot") {
    tftStepBoot();
  } else if (stepId == "colors") {
    tftStepColors();
  } else if (stepId == "shapes") {
    tftStepShapes();
  } else if (stepId == "text") {
    tftStepText();
  } else if (stepId == "lines") {
    tftStepLines();
  } else if (stepId == "animation") {
    tftStepAnimation();
  } else if (stepId == "progress") {
    tftStepProgressBar();
  } else if (stepId == "final") {
    tftStepFinal();
  } else {
    return false;
  }

  return true;
#else
  return false;
#endif
}

String getTFTStepLabel(const String &stepId) {
  if (stepId == "boot") return "Boot Screen";
  if (stepId == "colors") return "Colors";
  if (stepId == "shapes") return "Shapes";
  if (stepId == "text") return "Text";
  if (stepId == "lines") return "Lines";
  if (stepId == "animation") return "Animation";
  if (stepId == "progress") return "Progress Bar";
  if (stepId == "final") return "Final Message";
  return stepId;
}

void testTFT() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) {
    tftTestResult = String(Texts::not_available);
    return;
  }

  Serial.println("\r\n=== TEST TFT ===");

  tftStepBoot();
  tftStepColors();
  tftStepShapes();
  tftStepText();
  tftStepLines();
  tftStepAnimation();
  tftStepProgressBar();
  tftStepFinal();

  tftTestResult = "OK - Tests complets";
  Serial.println("TFT: Tests complets OK\r\n");
#endif
}

// ========== TEST ADC ==========
void testADC() {
  Serial.println("\r\n=== TEST ADC ===");
  adcReadings.clear();
  
  #ifdef CONFIG_IDF_TARGET_ESP32
    int adcPins[] = {36, 39, 34, 35, 32, 33};
    int numADC = 6;
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    int adcPins[] = {1, 2, 3, 4, 5, 6};
    int numADC = 6;
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    int adcPins[] = {0, 1, 2, 3, 4};
    int numADC = 5;
  #else
    int adcPins[] = {36, 39};
    int numADC = 2;
  #endif

  for (int i = 0; i < numADC; i++) {
    ADCReading reading;
    reading.pin = adcPins[i];
    reading.rawValue = analogRead(adcPins[i]);
    reading.voltage = (reading.rawValue / 4095.0) * 3.3;
    adcReadings.push_back(reading);

    Serial.printf("GPIO%d: %d (%.2fV)\r\n", reading.pin, reading.rawValue, reading.voltage);
  }
  
  // [OPT-009]: Buffer-based test result (1 vs 4 allocations)
  char adcBuf[96];
  snprintf(adcBuf, sizeof(adcBuf), "%d %s - %s", numADC, Texts::channels.str().c_str(), Texts::ok.str().c_str());
  adcTestResult = String(adcBuf);
  Serial.printf("ADC: %d canaux testes\r\n", numADC);
}

// ========== TEST PWM ==========
void testPWM() {
  Serial.println("\r\n=== TEST PWM ===");
  int testPin = 2;

  #ifdef CONFIG_IDF_TARGET_ESP32S3
    testPin = 48;
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    testPin = 8;
  #endif

  Serial.printf("Test PWM sur GPIO%d\r\n", testPin);

  // --- [PLATFORMIO FIX] Use legacy LEDC API for PlatformIO compatibility ---
  uint8_t pwmChannel = 0;
  ledcSetup(pwmChannel, 5000, 8);       // channel, freq, resolution
  ledcAttachPin(testPin, pwmChannel);    // pin, channel

  for (int duty = 0; duty <= 255; duty += 51) {
    ledcWrite(pwmChannel, duty);         // channel, duty
    Serial.printf("PWM duty: %d/255\r\n", duty);
    delay(80);
    yield();
  }

  ledcWrite(pwmChannel, 0);
  ledcDetachPin(testPin);                // pin

  // [OPT-009]: Buffer-based test result (1 vs 4 allocations)
  char pwmBuf[96];
  snprintf(pwmBuf, sizeof(pwmBuf), "%s %s - GPIO %d", Texts::test.str().c_str(), Texts::ok.str().c_str(), testPin);
  pwmTestResult = String(pwmBuf);
  Serial.println("PWM: OK");
}

// ========== SCAN SPI ==========
void scanSPI() {
  Serial.println("\r\n=== SCAN SPI ===");
  spiInfo = "";
  
  #ifdef CONFIG_IDF_TARGET_ESP32
    spiInfo = "HSPI (12,13,14,15), VSPI (18,19,23,5)";
    Serial.println("SPI: HSPI + VSPI disponibles");
  #elif defined(CONFIG_IDF_TARGET_ESP32S2)
    spiInfo = "SPI2, SPI3 disponibles";
    Serial.println("SPI: SPI2, SPI3");
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    spiInfo = "SPI2, SPI3 disponibles";
    Serial.println("SPI: SPI2, SPI3");
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    spiInfo = "SPI2 disponible";
    Serial.println("SPI: SPI2");
  #else
    spiInfo = "Info SPI non disponible";
  #endif
}

// ========== LISTING PARTITIONS ==========
void listPartitions() {
  Serial.println("\r\n=== PARTITIONS FLASH ===");
  partitionsInfo = "";
  
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
  
  int count = 0;
  while (it != NULL) {
    const esp_partition_t *part = esp_partition_get(it);
    
    char line[128];
    sprintf(line, "%s - Type:0x%02x Addr:0x%08x Size:%dKB", 
            part->label, part->type, part->address, part->size / 1024);
    
    if (count > 0) partitionsInfo += "\r\n";
    partitionsInfo += String(line);
    
    Serial.println(line);
    
    it = esp_partition_next(it);
    count++;
  }
  
  esp_partition_iterator_release(it);
  Serial.printf("Total: %d partitions\r\n", count);
}

// TEST LED RGB
void testRGBLed() {
  Serial.println("\r\n=== TEST LED RGB ===");

  if (rgb_led_pin_r < 0 || rgb_led_pin_g < 0 || rgb_led_pin_b < 0) {
    rgbLedTestResult = String(Texts::configuration_invalid);
    rgbLedAvailable = false;
    Serial.println("LED RGB: Configuration invalide");
    return;
  }

  pinMode(rgb_led_pin_r, OUTPUT);
  pinMode(rgb_led_pin_g, OUTPUT);
  pinMode(rgb_led_pin_b, OUTPUT);

  Serial.printf("Test LED RGB - R:%d G:%d B:%d\r\n", rgb_led_pin_r, rgb_led_pin_g, rgb_led_pin_b);

  digitalWrite(rgb_led_pin_r, LOW);
  digitalWrite(rgb_led_pin_g, LOW);
  digitalWrite(rgb_led_pin_b, LOW);
  delay(120);

  digitalWrite(rgb_led_pin_r, HIGH);
  delay(150);
  digitalWrite(rgb_led_pin_r, LOW);

  digitalWrite(rgb_led_pin_g, HIGH);
  delay(150);
  digitalWrite(rgb_led_pin_g, LOW);

  digitalWrite(rgb_led_pin_b, HIGH);
  delay(150);
  digitalWrite(rgb_led_pin_b, LOW);

  rgbLedTestResult = OK_STR;
  rgbLedAvailable = true;
  Serial.println("LED RGB: OK");
}

void setRGBLedColor(int r, int g, int b) {
  if (rgb_led_pin_r >= 0 && rgb_led_pin_g >= 0 && rgb_led_pin_b >= 0) {
    analogWrite(rgb_led_pin_r, r);
    analogWrite(rgb_led_pin_g, g);
    analogWrite(rgb_led_pin_b, b);
  }
}

// TEST BUZZER
void testBuzzer() {
  Serial.println("\r\n=== TEST BUZZER ===");

  if (buzzer_pin < 0) {
    buzzerTestResult = String(Texts::configuration_invalid);
    buzzerAvailable = false;
    Serial.println("Buzzer: Configuration invalide");
    return;
  }

  pinMode(buzzer_pin, OUTPUT);
  Serial.printf("Test Buzzer - Pin:%d\r\n", buzzer_pin);

  tone(buzzer_pin, 1000, 160);
  delay(220);
  tone(buzzer_pin, 1500, 160);
  delay(220);
  tone(buzzer_pin, 2000, 160);
  delay(220);
  noTone(buzzer_pin);

  buzzerTestResult = OK_STR;
  buzzerAvailable = true;
  Serial.println("Buzzer: OK");
}

void playBuzzerTone(int frequency, int duration) {
  if (BUZZER >= 0) {
    tone(buzzer_pin, frequency, duration);
  }
}

// DHT sensor helpers
static inline const char* getDhtSensorName() {
  return (DHT_SENSOR_TYPE == 22) ? "DHT22" : "DHT11";
}

// TEST DHT SENSOR
void testDHTSensor() {
  const char* sensorName = getDhtSensorName();
  Serial.printf("\r\n=== TEST %s ===\r\n", sensorName);

  if (dht_pin < 0) {
    dhtTestResult = String(Texts::configuration_invalid);
    dhtAvailable = false;
    Serial.printf("%s: Configuration invalide\r\n", sensorName);
    return;
  }

  Serial.printf("Lecture %s - Pin:%d\r\n", sensorName, dht_pin);

  pinMode(dht_pin, OUTPUT);
  digitalWrite(dht_pin, LOW);
  delay(20);
  digitalWrite(dht_pin, HIGH);
  delayMicroseconds(40);
  pinMode(dht_pin, INPUT_PULLUP);

  uint8_t data[5] = {0};
  uint8_t bits[40] = {0};

  unsigned long timeout = millis();
  while (digitalRead(dht_pin) == HIGH) {
    if (millis() - timeout > 100) {
      dhtTestResult = String(Texts::error_label);
      dhtAvailable = false;
      Serial.printf("%s: Timeout\r\n", sensorName);
      return;
    }
  }

  timeout = millis();
  while (digitalRead(dht_pin) == LOW) {
    if (millis() - timeout > 100) {
      dhtTestResult = String(Texts::error_label);
      dhtAvailable = false;
      Serial.printf("%s: Timeout\r\n", sensorName);
      return;
    }
  }

  timeout = millis();
  while (digitalRead(dht_pin) == HIGH) {
    if (millis() - timeout > 100) {
      dhtTestResult = String(Texts::error_label);
      dhtAvailable = false;
      Serial.printf("%s: Timeout\r\n", sensorName);
      return;
    }
  }

  for (int i = 0; i < 40; i++) {
    timeout = micros();
    while (digitalRead(dht_pin) == LOW) {
      if (micros() - timeout > 100) break;
    }

    unsigned long t = micros();
    timeout = micros();
    while (digitalRead(dht_pin) == HIGH) {
      if (micros() - timeout > 100) break;
    }

    if ((micros() - t) > 40) {
      bits[i] = 1;
    }
  }

  for (int i = 0; i < 5; i++) {
    data[i] = 0;
    for (int j = 0; j < 8; j++) {
      data[i] <<= 1;
      data[i] |= bits[i * 8 + j];
    }
  }

  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    if (DHT_SENSOR_TYPE == 22) {
      uint16_t rawHumidity = (static_cast<uint16_t>(data[0]) << 8) | data[1];
      uint16_t rawTemperature = (static_cast<uint16_t>(data[2]) << 8) | data[3];
      bool negative = rawTemperature & 0x8000;
      if (negative) {
        rawTemperature &= 0x7FFF;
      }
      dhtHumidity = rawHumidity * 0.1f;
      dhtTemperature = rawTemperature * 0.1f;
      if (negative) {
        dhtTemperature = -dhtTemperature;
      }
    } else {
      dhtHumidity = static_cast<float>(data[0]) + static_cast<float>(data[1]) * 0.1f;
      dhtTemperature = static_cast<float>(data[2]) + static_cast<float>(data[3]) * 0.1f;
    }
    dhtTestResult = OK_STR;
    dhtAvailable = true;
    Serial.printf("%s: T=%.1f°C H=%.1f%%\r\n", sensorName, dhtTemperature, dhtHumidity);
  } else {
    dhtTestResult = String(Texts::error_label);
    dhtAvailable = false;
    Serial.printf("%s: Checksum error\r\n", sensorName);
  }
}

// TEST LIGHT SENSOR
void testLightSensor() {
  Serial.println("\r\n=== TEST LIGHT SENSOR ===");

  if (light_sensor_pin < 0) {
    lightSensorTestResult = String(Texts::configuration_invalid);
    lightSensorAvailable = false;
    Serial.println("Light Sensor: Configuration invalide");
    return;
  }

  pinMode(light_sensor_pin, INPUT);
  Serial.printf("Lecture Light Sensor - Pin:%d\r\n", light_sensor_pin);

  int sum = 0;
  const int samples = 6;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(light_sensor_pin);
    delay(8);
    yield();
  }
  lightSensorValue = sum / samples;

  lightSensorTestResult = OK_STR;
  lightSensorAvailable = true;
  Serial.printf("Light Sensor: %d\r\n", lightSensorValue);
}

// TEST DISTANCE SENSOR
void testDistanceSensor() {
  Serial.println("\r\n=== TEST DISTANCE SENSOR (HC-SR04) ===");

  if (distance_trig_pin < 0 || distance_echo_pin < 0) {
    distanceSensorTestResult = String(Texts::configuration_invalid);
    distanceSensorAvailable = false;
    Serial.println("Distance Sensor: Configuration invalide");
    return;
  }

  // ESP32-S3 (OPI Flash/PSRAM): GPIO 35..48 sont généralement réservées
  // Si utilisées pour TRIG/ECHO, la mesure échouera systématiquement
  if ((distance_trig_pin >= 35 && distance_trig_pin <= 48) ||
      (distance_echo_pin >= 35 && distance_echo_pin <= 48)) {
    distanceSensorTestResult = String(Texts::configuration_invalid);
    distanceSensorAvailable = false;
    Serial.printf("Distance Sensor: Pins invalides sur ESP32-S3 OPI (TRIG=%d, ECHO=%d). Evitez GPIO 35..48.\r\n",
                  distance_trig_pin, distance_echo_pin);
    Serial.println("Suggestion: TRIG=26 (sortie), ECHO=25 (entrée) si le bus I2C secondaire est inactif.");
    return;
  }

  pinMode(distance_trig_pin, OUTPUT);
  pinMode(distance_echo_pin, INPUT);

  Serial.printf("Distance Sensor - Trig:%d Echo:%d\r\n", distance_trig_pin, distance_echo_pin);

  // Assurer ECHO à LOW avant de déclencher (éviter faux négatifs)
  {
    unsigned long waitStart = micros();
    while (digitalRead(distance_echo_pin) == HIGH && (micros() - waitStart) < 200000UL) {
      yield();
    }
  }

  // Séquence TRIG: 10 µs HIGH après stabilisation LOW
  digitalWrite(distance_trig_pin, LOW);
  delayMicroseconds(4);
  digitalWrite(distance_trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(distance_trig_pin, LOW);

  // Mesure de l'impulsion ECHO (timeout élargi)
  unsigned long timeoutUs = 60000UL; // 60 ms
  unsigned long duration = 0UL;
#if defined(ARDUINO_ARCH_ESP32)
  duration = pulseInLong(distance_echo_pin, HIGH, timeoutUs);
#else
  duration = pulseIn(distance_echo_pin, HIGH, timeoutUs);
#endif

  if (duration > 0UL) {
    // Vitesse du son ~0.034 cm/µs ; aller-retour => /2
    distanceValue = (double)duration * 0.034 / 2.0;
    distanceSensorTestResult = OK_STR;
    distanceSensorAvailable = true;
    Serial.printf("Distance: %.2f cm\r\n", distanceValue);
  } else {
    distanceSensorTestResult = String(Texts::error_label);
    distanceSensorAvailable = false;
    Serial.println("Distance Sensor: No echo (check wiring/level shifting/target distance)");
  }
}

// TEST MOTION SENSOR
void testMotionSensor() {
  Serial.println("\r\n=== TEST MOTION SENSOR (PIR) ===");

  if (motion_sensor_pin < 0) {
    motionSensorTestResult = String(Texts::configuration_invalid);
    motionSensorAvailable = false;
    Serial.println("Motion Sensor: Configuration invalide");
    return;
  }

  pinMode(motion_sensor_pin, INPUT);
  Serial.printf("Motion Sensor - Pin:%d\r\n", motion_sensor_pin);

  delay(30);
  yield();
  motionDetected = digitalRead(motion_sensor_pin);

  motionSensorTestResult = OK_STR;
  motionSensorAvailable = true;
  Serial.printf("Motion: %s\r\n", motionDetected ? "Detected" : "None");
}

// ========== CARTE SD ==========
bool initSD() {
  Serial.println("\r\n=== INIT SD CARD ===");

  if (sd_miso_pin < 0 || sd_mosi_pin < 0 || sd_sclk_pin < 0 || sd_cs_pin < 0) {
    sdTestResult = String(Texts::configuration_invalid);
    sdAvailable = false;
    Serial.println("SD: Configuration invalide");
    return false;
  }

  // Configuration SPI
  if (sdSPI == nullptr) {
#if defined(CONFIG_IDF_TARGET_ESP32)
    sdSPI = new SPIClass(HSPI);  // ESP32 classic uses HSPI
#else
    sdSPI = new SPIClass(FSPI);  // ESP32-S2/S3 use FSPI (SPI2)
#endif
  }
  sdSPI->begin(sd_sclk_pin, sd_miso_pin, sd_mosi_pin, sd_cs_pin);

  if (!SD.begin(sd_cs_pin, *sdSPI)) {
    sdTestResult = String(Texts::not_detected);
    sdAvailable = false;
    Serial.println("SD: Carte non detectee");
    return false;
  }

  sdAvailable = true;

  // Détection type de carte
  sdCardType = SD.cardType();
  switch (sdCardType) {
    case CARD_MMC:
      sdCardTypeStr = "MMC";
      break;
    case CARD_SD:
      sdCardTypeStr = "SDSC";
      break;
    case CARD_SDHC:
      sdCardTypeStr = "SDHC";
      break;
    default:
      sdCardTypeStr = "UNKNOWN";
  }

  // Taille de la carte
  sdCardSize = SD.cardSize() / (1024 * 1024);  // MB

  Serial.printf("SD: Type=%s, Size=%llu MB\r\n", sdCardTypeStr.c_str(), sdCardSize);

  char sdBuf[128];
  snprintf(sdBuf, sizeof(sdBuf), "OK - %s, %llu MB", sdCardTypeStr.c_str(), sdCardSize);
  sdTestResult = String(sdBuf);

  return true;
}

void testSD() {
  Serial.println("\r\n=== TEST SD CARD ===");

  if (!initSD()) {
    return;
  }

  // Test écriture/lecture
  const char* testFile = "/test_esp32.txt";
  const char* testData = "ESP32 Diagnostic Test";

  // Écriture
  File file = SD.open(testFile, FILE_WRITE);
  if (!file) {
    sdTestResult = "Erreur: Ecriture impossible";
    Serial.println("SD: Erreur ecriture");
    return;
  }

  file.println(testData);
  file.close();
  Serial.println("SD: Ecriture OK");

  // Lecture
  file = SD.open(testFile);
  if (!file) {
    sdTestResult = "Erreur: Lecture impossible";
    Serial.println("SD: Erreur lecture");
    return;
  }

  String readData = file.readStringUntil('\n');
  file.close();

  if (readData == testData) {
    uint64_t totalBytes = SD.totalBytes() / (1024 * 1024);
    uint64_t usedBytes = SD.usedBytes() / (1024 * 1024);

    char sdBuf[200];
    snprintf(sdBuf, sizeof(sdBuf),
             "OK - %s, Total:%llu MB, Utilise:%llu MB, Libre:%llu MB",
             sdCardTypeStr.c_str(), totalBytes, usedBytes, totalBytes - usedBytes);
    sdTestResult = String(sdBuf);
    sdTested = true;
    Serial.println("SD: Test complet OK");
  } else {
    sdTestResult = "Erreur: Verification lecture";
    Serial.println("SD: Erreur verification");
  }

  // Nettoyage
  SD.remove(testFile);
}

String getSDInfo() {
  if (!sdAvailable) {
    return "Non disponible";
  }

  String info = "Type: " + sdCardTypeStr;
  info += ", Taille: " + String((uint32_t)sdCardSize) + " MB";
  info += ", Total: " + String((uint32_t)(SD.totalBytes() / (1024 * 1024))) + " MB";
  info += ", Utilise: " + String((uint32_t)(SD.usedBytes() / (1024 * 1024))) + " MB";

  return info;
}

void resetSDTest() {
  sdTested = false;
  sdAvailable = false;
  SD.end();
}

// ========== ENCODEUR ROTATIF ==========
void initRotaryEncoder() {
  Serial.println("\r\n=== INIT ROTARY ENCODER ===");

  if (rotary_clk_pin < 0 || rotary_dt_pin < 0 || rotary_sw_pin < 0) {
    rotaryTestResult = String(Texts::configuration_invalid);
    rotaryAvailable = false;
    Serial.println("Rotary: Configuration invalide");
    return;
  }

  pinMode(rotary_clk_pin, INPUT_PULLUP);
  pinMode(rotary_dt_pin, INPUT_PULLUP);
  pinMode(rotary_sw_pin, INPUT_PULLUP);

  lastRotaryState = digitalRead(rotary_clk_pin);

  // Attacher interruptions
  attachInterrupt(digitalPinToInterrupt(rotary_clk_pin), rotaryISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rotary_dt_pin), rotaryISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rotary_sw_pin), rotaryButtonISR, FALLING);

  rotaryAvailable = true;
  rotaryTestResult = "Initialise - Tournez l'encodeur";

  Serial.printf("Rotary: CLK=%d, DT=%d, SW=%d\r\n",
                rotary_clk_pin, rotary_dt_pin, rotary_sw_pin);
}

void testRotaryEncoder() {
  Serial.println("\r\n=== TEST ROTARY ENCODER ===");

  initRotaryEncoder();

  if (!rotaryAvailable) {
    return;
  }

  // Reset position
  rotaryPosition = 0;
  rotaryButtonPressed = false;

  Serial.println("Rotary: Tournez l'encodeur et appuyez sur le bouton...");

  unsigned long testStart = millis();
  bool rotationDetected = false;
  bool buttonDetected = false;

  // Test pendant 5 secondes
  while (millis() - testStart < 5000) {
    if (rotaryPosition != 0) {
      rotationDetected = true;
    }

    if (rotaryButtonPressed) {
      buttonDetected = true;
      rotaryButtonPressed = false;  // Reset
    }

    if (rotationDetected && buttonDetected) {
      break;
    }

    delay(10);
    yield();
  }

  char rotaryBuf[256];
  if (rotationDetected && buttonDetected) {
    snprintf(rotaryBuf, sizeof(rotaryBuf),
             "OK - Rotation: %ld, Bouton: detecte", rotaryPosition);
    rotaryTested = true;
  } else if (rotationDetected) {
    snprintf(rotaryBuf, sizeof(rotaryBuf),
             "Partiel - Rotation OK (%ld), Bouton non teste", rotaryPosition);
  } else if (buttonDetected) {
    snprintf(rotaryBuf, sizeof(rotaryBuf),
             "Partiel - Bouton OK, Rotation non testee");
  } else {
    snprintf(rotaryBuf, sizeof(rotaryBuf),
             "Erreur - Aucune activite detectee");
  }

  rotaryTestResult = String(rotaryBuf);
  Serial.println(rotaryTestResult);

  // Reset pour usage normal
  rotaryPosition = 0;
}

void resetRotaryTest() {
  rotaryTested = false;
  rotaryPosition = 0;
  rotaryButtonPressed = false;
  detachInterrupt(digitalPinToInterrupt(rotary_clk_pin));
  detachInterrupt(digitalPinToInterrupt(rotary_dt_pin));
  detachInterrupt(digitalPinToInterrupt(rotary_sw_pin));
  rotaryAvailable = false;
}

long getRotaryPosition() {
  return rotaryPosition;
}

bool getRotaryButtonState() {
  bool state = rotaryButtonPressed;
  rotaryButtonPressed = false;  // Auto-reset après lecture
  return state;
}

void resetRotaryPosition() {
  rotaryPosition = 0;
}

// ========== BUTTON STATE READERS (v3.28.3/v3.28.5) ==========
// Read REAL GPIO state for monitoring (not volatile ISR variables)
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

// v3.28.5 - Read REAL GPIO state of rotary button for monitoring
int getRotaryButtonGPIOState() {
  if (rotary_sw_pin < 0 || rotary_sw_pin > 48) return -1;
  return digitalRead(rotary_sw_pin);
}

// ========== TEST STRESS MÉMOIRE ==========
void memoryStressTest() {
  Serial.println("\r\n=== STRESS TEST MEMOIRE ===");
  
  unsigned long startMs = millis();
  stressAllocationCount = 0;
  stressDurationMs = 0;

  const int allocSize = 1024;
  int maxAllocs = 0;
  std::vector<void*> allocations;
  
  Serial.println("Allocation progressive...");
  while(true) {
    void* ptr = malloc(allocSize);
    if (!ptr) break;
    allocations.push_back(ptr);
    maxAllocs++;
    
    if (maxAllocs % 100 == 0) {
      Serial.printf("Alloue: %d KB\r\n", maxAllocs);
    }
  }
  
  Serial.printf("Max allocations: %d blocs (%d KB)\r\n", maxAllocs, maxAllocs);


  Serial.println("Liberation memoire...");
  for (auto ptr : allocations) {
    free(ptr);
  }

  stressAllocationCount = maxAllocs;
  stressDurationMs = millis() - startMs;

  stressTestResult = String(Texts::max_alloc) + ": " + String(maxAllocs) + " KB";
  Serial.println("Stress test: OK");
}

// ========== COLLECTE DONNÉES ==========
void collectDiagnosticInfo() {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  
  diagnosticData.chipModel = detectChipModel();
  diagnosticData.chipRevision = String(chip_info.revision);
  diagnosticData.cpuCores = chip_info.cores;
  diagnosticData.cpuFreqMHz = ESP.getCpuFreqMHz();
  
  uint32_t flash_size;
  esp_flash_get_size(NULL, &flash_size);
  diagnosticData.flashSize = flash_size;
  diagnosticData.psramSize = ESP.getPsramSize();
  
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  diagnosticData.macAddress = String(macStr);
  
  diagnosticData.heapSize = ESP.getHeapSize();
  diagnosticData.freeHeap = ESP.getFreeHeap();
  diagnosticData.minFreeHeap = ESP.getMinFreeHeap();
  diagnosticData.maxAllocHeap = ESP.getMaxAllocHeap();
  
  diagnosticData.hasWiFi = (chip_info.features & CHIP_FEATURE_WIFI_BGN);
  diagnosticData.hasBT = (chip_info.features & CHIP_FEATURE_BT);
  bool featureBLE = (chip_info.features & CHIP_FEATURE_BLE);

#ifdef CHIP_ESP32S2
  if (chip_info.model == CHIP_ESP32S2) {
    runtimeBLE = false;
  }
#endif
#ifdef CHIP_ESP32S3
  if (chip_info.model == CHIP_ESP32S3) {
    runtimeBLE = true;
  }
#endif
#ifdef CHIP_ESP32C3
  if (chip_info.model == CHIP_ESP32C3) {
    runtimeBLE = true;
  }
#endif
#ifdef CHIP_ESP32C6
  if (chip_info.model == CHIP_ESP32C6) {
    runtimeBLE = true;
  }
#endif
#ifdef CHIP_ESP32H2
  if (chip_info.model == CHIP_ESP32H2) {
    runtimeBLE = true;
  }
#endif

  if (WiFi.status() == WL_CONNECTED) {
    diagnosticData.wifiSSID = WiFi.SSID();
    diagnosticData.wifiRSSI = WiFi.RSSI();
    diagnosticData.ipAddress = WiFi.localIP().toString();
#if DIAGNOSTIC_HAS_MDNS
    diagnosticData.mdnsAvailable = mdnsServiceActive;
#else
    diagnosticData.mdnsAvailable = false;
#endif
  } else {
    diagnosticData.wifiSSID = "";
    diagnosticData.wifiRSSI = -127;
    diagnosticData.ipAddress = "";
    diagnosticData.mdnsAvailable = false;
  }
  
  diagnosticData.gpioList = getGPIOList();
  diagnosticData.totalGPIO = countGPIO();
  
  diagnosticData.sdkVersion = ESP.getSdkVersion();
  diagnosticData.idfVersion = esp_get_idf_version();
  diagnosticData.uptime = millis();
  
  #ifdef SOC_TEMP_SENSOR_SUPPORTED
    diagnosticData.temperature = temperatureRead();
  #else
    diagnosticData.temperature = -999;
  #endif
  
  diagnosticData.neopixelTested = neopixelTested;
  diagnosticData.neopixelAvailable = neopixelAvailable;
  diagnosticData.neopixelResult = neopixelTestResult;
  
  diagnosticData.oledTested = oledTested;
  diagnosticData.oledAvailable = oledAvailable;
  diagnosticData.oledResult = oledTestResult;

  heapHistory[historyIndex] = (float)diagnosticData.freeHeap / 1024.0;
  if (diagnosticData.temperature != -999) {
    tempHistory[historyIndex] = diagnosticData.temperature;
  }
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;
}

// Routines de tests en tâche de fond
static void runBuiltinLedTestTask() {
  resetBuiltinLEDTest();
  testBuiltinLED();
}

static void runNeopixelTestTask() {
  neopixelPauseStatus();
  resetNeoPixelTest();
  testNeoPixel();
  neopixelRestoreWifiStatus();
}

static void runOledTestTask() {
  resetOLEDTest();
  testOLED();
}

static void runRgbLedTestTask() {
  testRGBLed();
}

static void runBuzzerTestTask() {
  testBuzzer();
}

static void runSDTestTask() {
  resetSDTest();
  testSD();
}

static void runRotaryTestTask() {
  resetRotaryTest();
  testRotaryEncoder();
}

// ========== HANDLERS API ==========
void handleTestGPIO() {
  testAllGPIOs();
  String json;
  json.reserve(gpioResults.size() * 40 + 20);  // Estimate size to avoid reallocations
  json = "{\"results\":[";
  for (size_t i = 0; i < gpioResults.size(); i++) {
    if (i > 0) json += ",";
    json += "{\"pin\":" + String(gpioResults[i].pin) + ",\"working\":" + String(gpioResults[i].working ? "true" : "false") + "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleWiFiScan() {
  scanWiFiNetworks();
  String json;
  json.reserve(wifiNetworks.size() * 150 + 20);  // Estimate size to avoid reallocations
  json = "{\"networks\":[";
  for (size_t i = 0; i < wifiNetworks.size(); i++) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + wifiNetworks[i].ssid + "\",\"rssi\":" + String(wifiNetworks[i].rssi) +
            ",\"channel\":" + String(wifiNetworks[i].channel) + ",\"encryption\":\"" + wifiNetworks[i].encryption +
            "\",\"bssid\":\"" + wifiNetworks[i].bssid + "\",\"band\":\"" + wifiNetworks[i].band +
            "\",\"bandwidth\":\"" + wifiNetworks[i].bandwidth + "\",\"phy\":\"" + wifiNetworks[i].phyModes +
            "\",\"freq\":" + String(wifiNetworks[i].freqMHz) + "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleI2CScan() {
  scanI2C();
  sendJsonResponse(200, {
    jsonNumberField("count", diagnosticData.i2cCount),
    jsonStringField("devices", diagnosticData.i2cDevices)
  });
}

void handleBuiltinLEDConfig() {
  if (server.hasArg("gpio")) {
    int newGPIO = server.arg("gpio").toInt();
    if (newGPIO >= 0 && newGPIO <= 48) {
      BUILTIN_LED_PIN = newGPIO;
      resetBuiltinLEDTest();
      // [OPT-007]: Buffer-based message formatting (1 vs 4 allocations)
      char msgBuf[96];
      snprintf(msgBuf, sizeof(msgBuf), "%s %s %d", Texts::config.str().c_str(), Texts::gpio.str().c_str(), BUILTIN_LED_PIN);
      sendOperationSuccess(String(msgBuf), {});
      return;
    }
  }
  sendOperationError(400, Texts::gpio_invalid.str(), {});
}

void handleBuiltinLEDTest() {
  bool alreadyRunning = false;
  bool started = startAsyncTest(builtinLedTestRunner, runBuiltinLedTestTask, alreadyRunning);

  if (started) {
    sendActionResponse(202, true, String(Texts::test_in_progress), {
      jsonBoolField("running", true),
      jsonStringField("result", builtinLedTestResult)
    });
    return;
  }

  if (alreadyRunning) {
    sendActionResponse(200, true, String(Texts::test_in_progress), {
      jsonBoolField("running", true),
      jsonStringField("result", builtinLedTestResult)
    });
    return;
  }

  resetBuiltinLEDTest();
  testBuiltinLED();
  sendActionResponse(200, builtinLedAvailable, builtinLedTestResult, {
    jsonBoolField("running", false),
    jsonStringField("result", builtinLedTestResult)
  });
}

void handleBuiltinLEDControl() {
  if (!server.hasArg("action")) {
    sendOperationError(400, Texts::configuration_invalid.str(), {});
    return;
  }

  String action = server.arg("action");
  if (BUILTIN_LED_PIN == -1) {
    sendOperationError(400, Texts::gpio_invalid.str(), {});
    return;
  }

  pinMode(BUILTIN_LED_PIN, OUTPUT);
  String message;

  if (action == "blink") {
    for (int i = 0; i < 5; i++) {
      digitalWrite(BUILTIN_LED_PIN, HIGH);
      delay(200);
      digitalWrite(BUILTIN_LED_PIN, LOW);
      delay(200);
    }
    // [OPT-009]: Buffer-based message (1 vs 2 allocations)
    char msgBuf[96];
    snprintf(msgBuf, sizeof(msgBuf), "%s %s", Texts::blink.str().c_str(), Texts::ok.str().c_str());
    message = String(msgBuf);
  } else if (action == "fade") {
    for (int i = 0; i <= 255; i += 5) {
      analogWrite(BUILTIN_LED_PIN, i);
      delay(10);
    }
    for (int i = 255; i >= 0; i -= 5) {
      analogWrite(BUILTIN_LED_PIN, i);
      delay(10);
    }
    digitalWrite(BUILTIN_LED_PIN, LOW);
    // [OPT-009]: Buffer-based message (1 vs 2 allocations)
    char msgBuf[96];
    snprintf(msgBuf, sizeof(msgBuf), "%s %s", Texts::fade.str().c_str(), Texts::ok.str().c_str());
    message = String(msgBuf);
  } else if (action == "off") {
    digitalWrite(BUILTIN_LED_PIN, LOW);
    builtinLedTested = false;
    message = String(Texts::off);
  } else {
    sendOperationError(400, Texts::configuration_invalid.str(), {});
    return;
  }

  sendOperationSuccess(message, {});
}

void handleNeoPixelConfig() {
  if (server.hasArg("gpio") && server.hasArg("count")) {
    int newGPIO = server.arg("gpio").toInt();
    int newCount = server.arg("count").toInt();
    
    if (newGPIO >= 0 && newGPIO <= 48 && newCount > 0 && newCount <= 100) {
      LED_PIN = newGPIO;
      LED_COUNT = newCount;
      if (strip) delete strip;
      strip = new Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
      resetNeoPixelTest();
      // [OPT-007]: Buffer-based message formatting (1 vs 4 allocations)
      char msgBuf[96];
      snprintf(msgBuf, sizeof(msgBuf), "%s %s %d", Texts::config.str().c_str(), Texts::gpio.str().c_str(), LED_PIN);
      sendOperationSuccess(String(msgBuf), {});
      return;
    }
  }
  sendOperationError(400, Texts::configuration_invalid.str(), {});
}

void handleNeoPixelTest() {
  bool alreadyRunning = false;
  bool started = startAsyncTest(neopixelTestRunner, runNeopixelTestTask, alreadyRunning);

  if (started) {
    sendActionResponse(202, true, String(Texts::test_in_progress), {
      jsonBoolField("running", true),
      jsonBoolField("available", neopixelAvailable),
      jsonStringField("result", neopixelTestResult)
    });
    return;
  }

  if (alreadyRunning) {
    sendActionResponse(200, true, String(Texts::test_in_progress), {
      jsonBoolField("running", true),
      jsonBoolField("available", neopixelAvailable),
      jsonStringField("result", neopixelTestResult)
    });
    return;
  }

  neopixelPauseStatus();
  resetNeoPixelTest();
  testNeoPixel();
  neopixelRestoreWifiStatus();
  sendActionResponse(200, neopixelAvailable, neopixelTestResult, {
    jsonBoolField("running", false),
    jsonBoolField("available", neopixelAvailable),
    jsonStringField("result", neopixelTestResult)
  });
}

void handleNeoPixelPattern() {
  if (!server.hasArg("pattern")) {
    sendOperationError(400, Texts::configuration_invalid.str(), {});
    return;
  }

  String pattern = server.arg("pattern");
  if (!strip) {
    String message = String(Texts::neopixel) + " - " + String(Texts::not_detected);
    sendOperationError(400, message, {});
    return;
  }

  neopixelPauseStatus();
  String message;
  if (pattern == "rainbow") {
    neopixelRainbow();
    char msgBuf[96];
    snprintf(msgBuf, sizeof(msgBuf), "%s %s", Texts::rainbow.str().c_str(), Texts::ok.str().c_str());
    message = String(msgBuf);
  } else if (pattern == "blink") {
    neopixelBlink(strip->Color(255, 0, 0), 5);
    // [OPT-009]: Buffer-based message (1 vs 2 allocations)
    char msgBuf[96];
    snprintf(msgBuf, sizeof(msgBuf), "%s %s", Texts::blink.str().c_str(), Texts::ok.str().c_str());
    message = String(msgBuf);
  } else if (pattern == "fade") {
    neopixelFade(strip->Color(0, 0, 255));
    // [OPT-009]: Buffer-based message (1 vs 2 allocations)
    char msgBuf[96];
    snprintf(msgBuf, sizeof(msgBuf), "%s %s", Texts::fade.str().c_str(), Texts::ok.str().c_str());
    message = String(msgBuf);
  } else if (pattern == "chase") {
    neopixelChase();
    char msgBuf[96];
    snprintf(msgBuf, sizeof(msgBuf), "%s %s", Texts::chase.str().c_str(), Texts::ok.str().c_str());
    message = String(msgBuf);
  } else if (pattern == "off") {
    strip->clear();
    strip->show();
    neopixelTested = false;
    message = String(Texts::off);
  } else {
    sendOperationError(400, Texts::configuration_invalid.str(), {});
    neopixelRestoreWifiStatus();
    return;
  }

  neopixelRestoreWifiStatus();
  sendOperationSuccess(message, {});
}

void handleNeoPixelColor() {
  if (!server.hasArg("r") || !server.hasArg("g") || !server.hasArg("b") || !strip) {
    sendOperationError(400, Texts::configuration_invalid.str());
    return;
  }

  int r = server.arg("r").toInt();
  int g = server.arg("g").toInt();
  int b = server.arg("b").toInt();

  strip->fill(strip->Color(r, g, b));
  strip->show();
  neopixelTested = false;

  String message = "RGB(" + String(r) + "," + String(g) + "," + String(b) + ")";
  sendOperationSuccess(message);
}

void handleOLEDConfig() {
  if (server.hasArg("sda") && server.hasArg("scl") && server.hasArg("rotation")) {
    int newSDA = server.arg("sda").toInt();
    int newSCL = server.arg("scl").toInt();
    int newRotation = server.arg("rotation").toInt();
    
    // Optional resolution parameters
    int newWidth = server.hasArg("width") ? server.arg("width").toInt() : oledWidth;
    int newHeight = server.hasArg("height") ? server.arg("height").toInt() : oledHeight;

    if (newSDA >= 0 && newSDA <= 48 && newSCL >= 0 && newSCL <= 48 && newRotation >= 0 && newRotation <= 3) {
      bool pinsChanged = (i2c_sda != newSDA) || (i2c_scl != newSCL);
      bool rotationChanged = (oledRotation != static_cast<uint8_t>(newRotation));
      bool resolutionChanged = (oledWidth != newWidth) || (oledHeight != newHeight);

      i2c_sda = newSDA;
      i2c_scl = newSCL;
      oledRotation = static_cast<uint8_t>(newRotation);
      oledWidth = newWidth;
      oledHeight = newHeight;

      if (pinsChanged || rotationChanged || resolutionChanged) {
        resetOLEDTest();
        Wire.end();
        detectOLED();
      } else if (oledAvailable) {
        applyOLEDOrientation();
      }

      // [OPT-007]: Buffer-based message formatting to reduce allocations (1 vs 9)
      char messageBuffer[128];
      snprintf(messageBuffer, sizeof(messageBuffer), "I2C reconfigure: SDA:%d SCL:%d Rot:%d Res:%dx%d",
               i2c_sda, i2c_scl, oledRotation, oledWidth, oledHeight);
      sendOperationSuccess(String(messageBuffer), {
        jsonNumberField("sda", i2c_sda),
        jsonNumberField("scl", i2c_scl),
        jsonNumberField("rotation", oledRotation),
        jsonNumberField("width", oledWidth),
        jsonNumberField("height", oledHeight)
      });
      return;
    }
  }
  sendOperationError(400, Texts::configuration_invalid.str(), {});
}

void handleOLEDTest() {
  bool alreadyRunning = false;
  bool started = startAsyncTest(oledTestRunner, runOledTestTask, alreadyRunning, 6144, 1);

  if (started) {
    sendActionResponse(202, true, String(Texts::test_in_progress), {
      jsonBoolField("running", true),
      jsonBoolField("available", oledAvailable),
      jsonStringField("result", oledTestResult)
    });
    return;
  }

  if (alreadyRunning) {
    sendActionResponse(200, true, String(Texts::test_in_progress), {
      jsonBoolField("running", true),
      jsonBoolField("available", oledAvailable),
      jsonStringField("result", oledTestResult)
    });
    return;
  }

  resetOLEDTest();
  testOLED();
  sendActionResponse(200, oledAvailable, oledTestResult, {
    jsonBoolField("running", false),
    jsonBoolField("available", oledAvailable),
    jsonStringField("result", oledTestResult)
  });
}

void handleOLEDStep() {
  if (!server.hasArg("step")) {
    sendOperationError(400, Texts::oled_step_unknown.str(), {});
    return;
  }

  String stepId = server.arg("step");

  if (!oledAvailable) {
    sendActionResponse(200, false, Texts::oled_step_unavailable.str(), {});
    return;
  }

  bool ok = performOLEDStep(stepId);
  if (!ok) {
    sendOperationError(400, Texts::oled_step_unknown.str(), {});
    return;
  }

  String label = getOLEDStepLabel(stepId);
  // [OPT-007]: Buffer-based message formatting (1 vs 2 allocations)
  char msgBuf[256];
  snprintf(msgBuf, sizeof(msgBuf), "%s %s", Texts::oled_step_executed_prefix.str().c_str(), label.c_str());
  sendOperationSuccess(String(msgBuf), {});
}

void handleOLEDMessage() {
  if (!server.hasArg("message")) {
    sendOperationError(400, Texts::configuration_invalid.str(), {});
    return;
  }

  String message = server.arg("message");
  oledShowMessage(message);
  // Use translation key instead of hardcoded string
  sendOperationSuccess(Texts::message_displayed.str(), {});
}

void handleOLEDBoot() {
  if (!oledAvailable) {
    sendActionResponse(200, false, "OLED not available", {});
    return;
  }

  oledShowWiFiStatus(PROJECT_NAME, "System Ready", WiFi.localIP().toString(), 100);
  sendOperationSuccess("Boot screen displayed", {});
}

void handleTFTTest() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) {
    sendActionResponse(200, false, "TFT not available", {
      jsonBoolField("available", false)
    });
    return;
  }

  testTFT();
  sendActionResponse(200, true, "TFT test complete", {
    jsonBoolField("available", true)
  });
#else
  sendActionResponse(200, false, "TFT not enabled", {
    jsonBoolField("available", false)
  });
#endif
}

void handleTFTStep() {
#if ENABLE_TFT_DISPLAY
  if (!server.hasArg("step")) {
    sendOperationError(400, "Step parameter missing", {});
    return;
  }

  String stepId = server.arg("step");

  if (!tftAvailable) {
    sendActionResponse(200, false, "TFT not available", {});
    return;
  }

  bool ok = performTFTStep(stepId);
  if (!ok) {
    sendOperationError(400, "Unknown TFT step", {});
    return;
  }

  String label = getTFTStepLabel(stepId);
  String message = "Step executed: " + label;
  sendOperationSuccess(message, {});
#else
  sendActionResponse(200, false, "TFT not enabled");
#endif
}

void handleTFTBoot() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) {
    sendActionResponse(200, false, "TFT not available", {});
    return;
  }

  displayBootSplash();
  
  // Display WiFi info if connected
  if (WiFi.status() == WL_CONNECTED) {
    displayWiFiConnected(WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  } else {
    displayWiFiStatus("WiFi not connected", TFT_ORANGE);
  }
  
  sendOperationSuccess("Boot screen displayed", {});
#else
  sendActionResponse(200, false, "TFT not enabled");
#endif
}

void handleTFTConfig() {
#if ENABLE_TFT_DISPLAY
  // v3.30.0: Support for dynamic driver switching
  // Check if driver parameter is provided (driver switch request)
  if (server.hasArg("driver")) {
    String newDriver = server.arg("driver");
    newDriver.toUpperCase();

    // Validate driver type
    if (newDriver != "ILI9341" && newDriver != "ST7789") {
      sendOperationError(400, "Invalid driver type. Must be ILI9341 or ST7789", {});
      return;
    }

    // Get optional display parameters for reinit
    int newWidth = server.hasArg("width") ? server.arg("width").toInt() : tftWidth;
    int newHeight = server.hasArg("height") ? server.arg("height").toInt() : tftHeight;
    int newRotation = server.hasArg("rotation") ? server.arg("rotation").toInt() : tftRotation;

    // Determine driver type enum
    TFT_DriverType driverType = (newDriver == "ILI9341") ? TFT_DRIVER_ILI9341 : TFT_DRIVER_ST7789;

    // Switch to new driver
    bool success = switchTFTDriver(driverType, newWidth, newHeight, newRotation);

    if (success) {
      // Update global variables
      tftDriver = newDriver;
      tftWidth = newWidth;
      tftHeight = newHeight;
      tftRotation = newRotation;

      String message = "TFT driver switched to " + tftDriver + " successfully";
      sendOperationSuccess(message, {
        jsonStringField("driver", tftDriver),
        jsonNumberField("width", tftWidth),
        jsonNumberField("height", tftHeight),
        jsonNumberField("rotation", tftRotation)
      });
    } else {
      sendOperationError(500, "Failed to switch TFT driver", {});
    }
    return;
  }

  // Original pin configuration (no driver change)
  if (server.hasArg("mosi") && server.hasArg("sclk") && server.hasArg("cs") &&
      server.hasArg("dc") && server.hasArg("rst")) {

    int newMISO = server.hasArg("miso") ? server.arg("miso").toInt() : tftMISO;
    int newMOSI = server.arg("mosi").toInt();
    int newSCLK = server.arg("sclk").toInt();
    int newCS = server.arg("cs").toInt();
    int newDC = server.arg("dc").toInt();
    int newRST = server.arg("rst").toInt();

    // Optional parameters
    int newBL = server.hasArg("bl") ? server.arg("bl").toInt() : tftBL;
    int newWidth = server.hasArg("width") ? server.arg("width").toInt() : tftWidth;
    int newHeight = server.hasArg("height") ? server.arg("height").toInt() : tftHeight;
    int newRotation = server.hasArg("rotation") ? server.arg("rotation").toInt() : tftRotation;

    // Validate pin ranges
    if (newMISO >= -1 && newMISO <= 48 && newMOSI >= 0 && newMOSI <= 48 &&
        newSCLK >= 0 && newSCLK <= 48 && newCS >= 0 && newCS <= 48 &&
        newDC >= 0 && newDC <= 48 && newRST >= -1 && newRST <= 48 &&
        newBL >= -1 && newBL <= 48 && newRotation >= 0 && newRotation <= 3) {

      // Update configuration
      tftMISO = newMISO;
      tftMOSI = newMOSI;
      tftSCLK = newSCLK;
      tftCS = newCS;
      tftDC = newDC;
      tftRST = newRST;
      tftBL = newBL;
      tftWidth = newWidth;
      tftHeight = newHeight;
      tftRotation = newRotation;

      // Note: Pin configuration changes require reboot for hardware reconfiguration
      // Driver can be switched dynamically without reboot

      String message = "TFT config updated: MISO:" + String(tftMISO) + " MOSI:" + String(tftMOSI) +
                       " SCLK:" + String(tftSCLK) + " CS:" + String(tftCS) + " DC:" + String(tftDC) +
                       " RST:" + String(tftRST) + " Res:" + String(tftWidth) + "x" + String(tftHeight) +
                       " Rot:" + String(tftRotation);

      sendOperationSuccess(message, {
        jsonNumberField("miso", tftMISO),
        jsonNumberField("mosi", tftMOSI),
        jsonNumberField("sclk", tftSCLK),
        jsonNumberField("cs", tftCS),
        jsonNumberField("dc", tftDC),
        jsonNumberField("rst", tftRST),
        jsonNumberField("bl", tftBL),
        jsonNumberField("width", tftWidth),
        jsonNumberField("height", tftHeight),
        jsonNumberField("rotation", tftRotation),
        jsonStringField("driver", tftDriver)
      });
      return;
    }
  }
  sendOperationError(400, Texts::configuration_invalid.str(), {});
#else
  sendActionResponse(200, false, "TFT not enabled");
#endif
}

// ============================================================
// TFT BRIGHTNESS CONTROL API (v3.33.2)
// ============================================================
void handleTFTBrightness() {
#if ENABLE_TFT_DISPLAY
  // GET request: return current brightness
  if (server.method() == HTTP_GET) {
    uint8_t currentBrightness = getTFTBrightness();
    sendJsonResponse(200, {
      jsonNumberField("brightness", currentBrightness),
      jsonNumberField("min", 0),
      jsonNumberField("max", 255)
    });
    return;
  }

  // POST/PUT request: set new brightness
  if (!server.hasArg("value")) {
    sendOperationError(400, "Brightness value missing (0-255)", {});
    return;
  }

  int brightness = server.arg("value").toInt();

  // Validate brightness range
  if (brightness < 0 || brightness > 255) {
    sendOperationError(400, "Brightness must be between 0 and 255", {});
    return;
  }

  // Set new brightness
  setTFTBrightness(static_cast<uint8_t>(brightness));

  String message = "TFT brightness set to " + String(brightness) + "/255";
  sendOperationSuccess(message, {
    jsonNumberField("brightness", brightness)
  });
#else
  sendActionResponse(200, false, "TFT not enabled");
#endif
}

void handleADCTest() {
  testADC();
  String json;
  json.reserve(adcReadings.size() * 60 + 50);  // Estimate size to avoid reallocations
  json = "{\"readings\":[";
  for (size_t i = 0; i < adcReadings.size(); i++) {
    if (i > 0) json += ",";
    json += "{\"pin\":" + String(adcReadings[i].pin) + ",\"raw\":" + String(adcReadings[i].rawValue) +
            ",\"voltage\":" + String(adcReadings[i].voltage, 2) + "}";
  }
  json += "],\"result\":\"" + adcTestResult + "\"}";
  server.send(200, "application/json", json);
}

void handlePWMTest() {
  testPWM();
  sendJsonResponse(200, { jsonStringField("result", pwmTestResult) });
}

void handleSPIScan() {
  scanSPI();
  sendJsonResponse(200, { jsonStringField("info", spiInfo) });
}

void handlePartitionsList() {
  listPartitions();
  sendJsonResponse(200, { jsonStringField("partitions", partitionsInfo) });
}

void handleStressTest() {
  memoryStressTest();
  sendJsonResponse(200, {
    jsonStringField("result", stressTestResult),
    jsonNumberField("allocations", static_cast<unsigned long>(stressAllocationCount)),
    jsonNumberField("durationMs", stressDurationMs),
    jsonStringField("allocationsLabel", stressTestResult)
  });
}

// Handlers API pour les nouveaux capteurs
void handleRGBLedConfig() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    rgb_led_pin_r = server.arg("r").toInt();
    rgb_led_pin_g = server.arg("g").toInt();
    rgb_led_pin_b = server.arg("b").toInt();
    // [OPT-009]: Use OK_STR constant instead of String(Texts::ok)
    sendActionResponse(200, true, OK_STR, {});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleRGBLedTest() {
  bool alreadyRunning = false;
  bool started = startAsyncTest(rgbLedTestRunner, runRgbLedTestTask, alreadyRunning, 3072, 1);

  if (started) {
    sendJsonResponse(202, {
      jsonBoolField("running", true),
      jsonBoolField("success", rgbLedAvailable),
      jsonStringField("result", rgbLedTestResult)
    });
    return;
  }

  if (alreadyRunning) {
    sendJsonResponse(200, {
      jsonBoolField("running", true),
      jsonBoolField("success", rgbLedAvailable),
      jsonStringField("result", rgbLedTestResult)
    });
    return;
  }

  testRGBLed();
  sendJsonResponse(200, {
    jsonBoolField("running", false),
    jsonBoolField("success", rgbLedAvailable),
    jsonStringField("result", rgbLedTestResult)
  });
}

void handleRGBLedColor() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();
    setRGBLedColor(r, g, b);
    sendActionResponse(200, true, "RGB(" + String(r) + "," + String(g) + "," + String(b) + ")", {});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleBuzzerConfig() {
  if (server.hasArg("pin")) {
    buzzer_pin = server.arg("pin").toInt();
    // [OPT-009]: Use OK_STR constant instead of String(Texts::ok)
    sendActionResponse(200, true, OK_STR, {});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleBuzzerTest() {
  bool alreadyRunning = false;
  bool started = startAsyncTest(buzzerTestRunner, runBuzzerTestTask, alreadyRunning, 3072, 1);

  if (started) {
    sendJsonResponse(202, {
      jsonBoolField("running", true),
      jsonBoolField("success", buzzerAvailable),
      jsonStringField("result", buzzerTestResult)
    });
    return;
  }

  if (alreadyRunning) {
    sendJsonResponse(200, {
      jsonBoolField("running", true),
      jsonBoolField("success", buzzerAvailable),
      jsonStringField("result", buzzerTestResult)
    });
    return;
  }

  testBuzzer();
  sendJsonResponse(200, {
    jsonBoolField("running", false),
    jsonBoolField("success", buzzerAvailable),
    jsonStringField("result", buzzerTestResult)
  });
}

void handleBuzzerTone() {
  if (server.hasArg("freq") && server.hasArg("duration")) {
    int freq = server.arg("freq").toInt();
    int duration = server.arg("duration").toInt();
    playBuzzerTone(freq, duration);
    sendActionResponse(200, true, String(freq) + "Hz", {});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleDHTConfig() {
  bool updated = false;

  if (server.hasArg("pin")) {
    dht_pin = server.arg("pin").toInt();
    updated = true;
  }

  if (server.hasArg("type")) {
    String rawType = server.arg("type");
    rawType.trim();
    rawType.toUpperCase();

    uint8_t candidate = 0;
    if (rawType == "DHT22" || rawType == "22") {
      candidate = 22;
    } else if (rawType == "DHT11" || rawType == "11") {
      candidate = 11;
    }

    if (candidate == 11 || candidate == 22) {
      DHT_SENSOR_TYPE = candidate;
      updated = true;
    } else {
      sendActionResponse(400, false, String(Texts::configuration_invalid), {});
      return;
    }
  }

  if (updated) {
    sendActionResponse(200,
                       true,
                       String(Texts::ok),
                       {jsonNumberField("type", static_cast<int>(DHT_SENSOR_TYPE))});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleDHTTest() {
  testDHTSensor();
  sendJsonResponse(200, {
    jsonBoolField("success", dhtAvailable),
    jsonStringField("result", dhtTestResult),
    jsonFloatField("temperature", dhtTemperature, 1),
    jsonFloatField("humidity", dhtHumidity, 1),
    jsonNumberField("type", static_cast<int>(DHT_SENSOR_TYPE))
  });
}

void handleLightSensorConfig() {
  if (server.hasArg("pin")) {
    light_sensor_pin = server.arg("pin").toInt();
    // [OPT-009]: Use OK_STR constant instead of String(Texts::ok)
    sendActionResponse(200, true, OK_STR, {});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleLightSensorTest() {
  testLightSensor();
  sendJsonResponse(200, {
    jsonBoolField("success", lightSensorAvailable),
    jsonStringField("result", lightSensorTestResult),
    jsonNumberField("value", lightSensorValue)
  });
}

void handleDistanceSensorConfig() {
  if (server.hasArg("trig") && server.hasArg("echo")) {
    distance_trig_pin = server.arg("trig").toInt();
    distance_echo_pin = server.arg("echo").toInt();
    // [OPT-009]: Use OK_STR constant instead of String(Texts::ok)
    sendActionResponse(200, true, OK_STR, {});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleDistanceSensorTest() {
  testDistanceSensor();
  sendJsonResponse(200, {
    jsonBoolField("success", distanceSensorAvailable),
    jsonStringField("result", distanceSensorTestResult),
    jsonFloatField("distance", distanceValue, 2)
  });
}

void handleMotionSensorConfig() {
  if (server.hasArg("pin")) {
    motion_sensor_pin = server.arg("pin").toInt();
    // [OPT-009]: Use OK_STR constant instead of String(Texts::ok)
    sendActionResponse(200, true, OK_STR, {});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleMotionSensorTest() {
  testMotionSensor();
  sendJsonResponse(200, {
    jsonBoolField("success", motionSensorAvailable),
    jsonStringField("result", motionSensorTestResult),
    jsonBoolField("motion", motionDetected)
  });
}

// ========== SD CARD HANDLERS ==========
void handleSDConfig() {
  if (server.hasArg("miso") && server.hasArg("mosi") &&
      server.hasArg("sclk") && server.hasArg("cs")) {
    sd_miso_pin = server.arg("miso").toInt();
    sd_mosi_pin = server.arg("mosi").toInt();
    sd_sclk_pin = server.arg("sclk").toInt();
    sd_cs_pin = server.arg("cs").toInt();

    resetSDTest();
    sendActionResponse(200, true, OK_STR, {});
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid), {});
  }
}

void handleSDTest() {
  bool alreadyRunning = false;
  bool started = startAsyncTest(sdTestRunner, runSDTestTask, alreadyRunning, 6144, 1);

  if (started) {
    sendJsonResponse(202, {
      jsonBoolField("running", true),
      jsonBoolField("success", sdAvailable),
      jsonStringField("result", sdTestResult)
    });
    return;
  }

  if (alreadyRunning) {
    sendJsonResponse(200, {
      jsonBoolField("running", true),
      jsonBoolField("success", sdAvailable),
      jsonStringField("result", sdTestResult)
    });
    return;
  }

  testSD();
  sendJsonResponse(200, {
    jsonBoolField("running", false),
    jsonBoolField("success", sdAvailable),
    jsonStringField("result", sdTestResult)
  });
}

void handleSDInfo() {
  if (!sdAvailable) {
    initSD();
  }

  sendJsonResponse(200, {
    jsonBoolField("available", sdAvailable),
    jsonStringField("type", sdCardTypeStr),
    jsonNumberField("size_mb", (uint32_t)sdCardSize),
    jsonNumberField("total_mb", sdAvailable ? (uint32_t)(SD.totalBytes() / (1024 * 1024)) : 0),
    jsonNumberField("used_mb", sdAvailable ? (uint32_t)(SD.usedBytes() / (1024 * 1024)) : 0),
    jsonStringField("result", sdTestResult)
  });
}

void handleSDTestRead() {
  if (!sdAvailable) {
    initSD();
  }

  if (!sdAvailable) {
    sendJsonResponse(200, {
      jsonBoolField("success", false),
      jsonStringField("result", String(Texts::not_available))
    });
    return;
  }

  // Test lecture
  const char* testFile = "/test_read.txt";

  // Créer un fichier de test si nécessaire
  if (!SD.exists(testFile)) {
    File file = SD.open(testFile, FILE_WRITE);
    if (file) {
      file.println("ESP32 Read Test Data");
      file.close();
    }
  }

  // Lecture
  File file = SD.open(testFile);
  if (!file) {
    sendJsonResponse(200, {
      jsonBoolField("success", false),
      jsonStringField("result", "Read test failed: Cannot open file")
    });
    return;
  }

  String readData = file.readStringUntil('\n');
  file.close();

  sendJsonResponse(200, {
    jsonBoolField("success", true),
    jsonStringField("result", "Read test OK - " + String(readData.length()) + " bytes read")
  });
}

void handleSDTestWrite() {
  if (!sdAvailable) {
    initSD();
  }

  if (!sdAvailable) {
    sendJsonResponse(200, {
      jsonBoolField("success", false),
      jsonStringField("result", String(Texts::not_available))
    });
    return;
  }

  // Test écriture
  const char* testFile = "/test_write.txt";
  const char* testData = "ESP32 Write Test - ";

  File file = SD.open(testFile, FILE_WRITE);
  if (!file) {
    sendJsonResponse(200, {
      jsonBoolField("success", false),
      jsonStringField("result", "Write test failed: Cannot create file")
    });
    return;
  }

  file.print(testData);
  file.println(millis());
  file.close();

  sendJsonResponse(200, {
    jsonBoolField("success", true),
    jsonStringField("result", "Write test OK - File created successfully")
  });
}

void handleSDFormat() {
  if (!sdAvailable) {
    initSD();
  }

  if (!sdAvailable) {
    sendJsonResponse(200, {
      jsonBoolField("success", false),
      jsonStringField("result", String(Texts::not_available))
    });
    return;
  }

  // Nettoyer tous les fichiers de test
  SD.remove("/test_esp32.txt");
  SD.remove("/test_read.txt");
  SD.remove("/test_write.txt");

  // Note: Full SD card formatting requires low-level access
  // This is a basic cleanup of test files
  // For full format, would need SD_MMC.format() or similar

  sendJsonResponse(200, {
    jsonBoolField("success", true),
    jsonStringField("result", "Test files cleaned - SD card ready")
  });
}

// ========== ROTARY ENCODER HANDLERS ==========
void handleRotaryConfig() {
  if (server.hasArg("clk") && server.hasArg("dt") && server.hasArg("sw")) {
    rotary_clk_pin = server.arg("clk").toInt();
    rotary_dt_pin = server.arg("dt").toInt();
    rotary_sw_pin = server.arg("sw").toInt();

    resetRotaryTest();
    sendActionResponse(200, true, OK_STR);
  } else {
    sendActionResponse(400, false, String(Texts::configuration_invalid));
  }
}

void handleRotaryTest() {
  bool alreadyRunning = false;
  bool started = startAsyncTest(rotaryTestRunner, runRotaryTestTask, alreadyRunning, 4096, 1);

  if (started) {
    sendJsonResponse(202, {
      jsonBoolField("running", true),
      jsonBoolField("success", rotaryAvailable),
      jsonStringField("result", rotaryTestResult)
    });
    return;
  }

  if (alreadyRunning) {
    sendJsonResponse(200, {
      jsonBoolField("running", true),
      jsonBoolField("success", rotaryAvailable),
      jsonStringField("result", rotaryTestResult)
    });
    return;
  }

  testRotaryEncoder();
  sendJsonResponse(200, {
    jsonBoolField("running", false),
    jsonBoolField("success", rotaryAvailable),
    jsonStringField("result", rotaryTestResult)
  });
}

void handleRotaryPosition() {
  // v3.28.5 fix: Read REAL GPIO state for monitoring, not ISR variable
  int buttonGPIOState = getRotaryButtonGPIOState();
  bool buttonPressed = (buttonGPIOState == LOW && buttonGPIOState != -1);

  sendJsonResponse(200, {
    jsonNumberField("position", (int32_t)rotaryPosition),
    jsonBoolField("button_pressed", buttonPressed),
    jsonBoolField("available", rotaryAvailable)
  });
}

void handleRotaryReset() {
  resetRotaryPosition();
  sendActionResponse(200, true, "Position reset");
}

// ========== BUTTON STATE HANDLERS (v3.28.3) ==========
// v3.28.5: Use constants directly for pins
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
    jsonNumberField("boot_pin", BUTTON_BOOT),  // v3.28.5: Use constants
    jsonNumberField("button1_pin", BUTTON_1),
    jsonNumberField("button2_pin", BUTTON_2)
  });
}

// Individual button state (v3.28.4 fix - frontend expects this endpoint)
// v3.28.5: Use constants directly for pins
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
    pin = BUTTON_BOOT;  // v3.28.5: Use constant directly
  } else if (buttonParam == "1" || buttonParam == "button1") {
    state = getButton1State();
    pin = BUTTON_1;
  } else if (buttonParam == "2" || buttonParam == "button2") {
    state = getButton2State();
    pin = BUTTON_2;
  } else {
    sendActionResponse(400, false, "Invalid button parameter (must be 'boot', '1', or '2')");
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

// GPS Handlers
void handleGPSData() {
  updateGPS();
  String json;
  json.reserve(400);
  json = "{";
  json += "\"valid\":" + String(gpsData.valid ? "true" : "false") + ",";
  json += "\"hasFix\":" + String(gpsData.hasFix ? "true" : "false") + ",";
  json += "\"latitude\":" + String(gpsData.latitude, 6) + ",";
  json += "\"longitude\":" + String(gpsData.longitude, 6) + ",";
  json += "\"altitude\":" + String(gpsData.altitude, 2) + ",";
  json += "\"satellites\":" + String(gpsData.satellites) + ",";
  json += "\"satellites_used\":" + String(gpsData.satellites_used) + ",";
  json += "\"hdop\":" + String(gpsData.hdop, 2) + ",";
  json += "\"speed\":" + String(gpsData.speed, 2) + ",";
  json += "\"course\":" + String(gpsData.course, 2) + ",";
  json += "\"fix_type\":\"" + gpsData.fix_type + "\",";
  json += "\"status\":\"" + gpsData.status_str + "\",";
  json += "\"time\":\"" + String(gpsData.hour) + ":" + String(gpsData.minute) + ":" + String(gpsData.second) + "\",";
  json += "\"date\":\"" + String(gpsData.day) + "/" + String(gpsData.month) + "/" + String(gpsData.year) + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleGPSTest() {
  testGPS();
  String json;
  json.reserve(200);
  json = "{";
  json += "\"success\":" + String(gpsAvailable ? "true" : "false") + ",";
  json += "\"result\":\"" + gpsTestResult + "\",";
  json += "\"available\":" + String(gpsAvailable ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

// Environmental Sensors Handlers
void handleEnvironmentalSensors() {
  updateEnvironmentalSensors();
  String json;
  json.reserve(400);
  json = "{";
  json += "\"aht20_available\":" + String(envData.aht20_available ? "true" : "false") + ",";
  json += "\"bmp280_available\":" + String(envData.bmp280_available ? "true" : "false") + ",";
  json += "\"temperature_avg\":" + String(envData.temperature_avg, 1) + ",";
  json += "\"humidity\":" + String(envData.humidity, 1) + ",";
  json += "\"pressure\":" + String(envData.pressure, 2) + ",";
  json += "\"altitude\":" + String(envData.altitude, 1) + ",";
  json += "\"aht20_temp\":" + String(envData.temperature_aht20, 1) + ",";
  json += "\"bmp280_temp\":" + String(envData.temperature_bmp280, 1) + ",";
  json += "\"aht20_status\":\"" + envData.aht20_status + "\",";
  json += "\"bmp280_status\":\"" + envData.bmp280_status + "\",";
  json += "\"combined_status\":\"" + envData.combined_status + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleEnvironmentalTest() {
  testEnvironmentalSensors();
  String json;
  json.reserve(200);
  json = "{";
  json += "\"success\":" + String(envSensorAvailable ? "true" : "false") + ",";
  json += "\"result\":\"" + envSensorTestResult + "\",";
  json += "\"available\":" + String(envSensorAvailable ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleBenchmark() {
  unsigned long cpuTime = benchmarkCPU();
  unsigned long memTime = benchmarkMemory();

  diagnosticData.cpuBenchmark = cpuTime;
  diagnosticData.memBenchmark = memTime;

  // Provide derived benchmark metrics for richer telemetry
  // Combined memory stress metrics for the benchmark API
  memoryStressTest();

  double cpuPerf = 100000.0 / static_cast<double>(cpuTime);
  double memSpeed = (10000.0 * sizeof(int) * 2.0) / static_cast<double>(memTime);
  sendJsonResponse(200, {
    jsonNumberField("cpu", cpuTime),
    jsonNumberField("memory", memTime),
    jsonFloatField("cpuPerf", cpuPerf, 2),
    jsonFloatField("memSpeed", memSpeed, 2),
    jsonNumberField("allocations", static_cast<unsigned long>(stressAllocationCount)),
    jsonNumberField("stressDuration", stressDurationMs),
    jsonStringField("stress", stressTestResult),
    jsonStringField("allocationsLabel", stressTestResult)
  });
}

void handleStatus() {
  collectDiagnosticInfo();
  collectDetailedMemory();

  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
  unsigned long currentUptime = millis();

  String json;
  json.reserve(400);
  json = "{";
  json += "\"connected\":" + String(wifiConnected ? "true" : "false") + ",";
  json += "\"uptime\":" + String(currentUptime) + ",";
  json += "\"temperature\":" + String(diagnosticData.temperature) + ",";
  json += "\"sram\":{\"total\":" + String(detailedMemory.sramTotal) +
          ",\"free\":" + String(detailedMemory.sramFree) +
          ",\"used\":" + String(detailedMemory.sramUsed) + "},";
  json += "\"psram\":{\"total\":" + String(detailedMemory.psramTotal) +
          ",\"free\":" + String(detailedMemory.psramFree) +
          ",\"used\":" + String(detailedMemory.psramUsed) + "},";
  json += "\"fragmentation\":" + String(detailedMemory.fragmentationPercent, 1) + "";
  json += "}";

  server.send(200, "application/json", json);
}

void handleSystemInfo() {
  collectDiagnosticInfo();
  String json;
  json.reserve(600);
  json = "{";
  json += "\"chipModel\":\"" + diagnosticData.chipModel + "\",";
  json += "\"chipRevision\":\"" + diagnosticData.chipRevision + "\",";
  json += "\"cpuCores\":" + String(diagnosticData.cpuCores) + ",";
  json += "\"cpuFreq\":" + String(diagnosticData.cpuFreqMHz) + ",";
  json += "\"macAddress\":\"" + diagnosticData.macAddress + "\",";
  json += "\"ipAddress\":\"" + diagnosticData.ipAddress + "\",";
  json += "\"mdnsReady\":" + String(diagnosticData.mdnsAvailable ? "true" : "false") + ",";
  json += "\"uptime\":" + String(diagnosticData.uptime);
  if (diagnosticData.temperature != -999) {
    json += ",\"temperature\":" + String(diagnosticData.temperature, 1);
  }
  json += "}";
  server.send(200, "application/json", json);
}

void handleMemory() {
  collectDetailedMemory();
  String json;
  json.reserve(500);
  json = "{";
  json += "\"heap\":{\"total\":" + String(diagnosticData.heapSize) +
          ",\"free\":" + String(diagnosticData.freeHeap) +
          ",\"used\":" + String(diagnosticData.heapSize - diagnosticData.freeHeap) + "},";
  json += "\"psram\":{\"total\":" + String(detailedMemory.psramTotal) +
          ",\"free\":" + String(detailedMemory.psramFree) +
          ",\"used\":" + String(detailedMemory.psramUsed) + "},";
  json += "\"fragmentation\":" + String(detailedMemory.fragmentationPercent, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void handleWiFiInfo() {
  collectDiagnosticInfo();
  String json;
  json.reserve(400);
  json = "{";
  json += "\"connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  json += "\"ssid\":\"" + diagnosticData.wifiSSID + "\",";
  json += "\"rssi\":" + String(diagnosticData.wifiRSSI) + ",";
  json += "\"quality_key\":\"" + String(getWiFiSignalQualityKey()) + "\",";
  json += "\"quality\":\"" + getWiFiSignalQuality() + "\",";
  json += "\"ip\":\"" + diagnosticData.ipAddress + "\",";
  json += "\"gateway\":\"" + WiFi.gatewayIP().toString() + "\",";
  json += "\"dns\":\"" + WiFi.dnsIP().toString() + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handlePeripherals() {
  scanI2C();
  String json;
  json.reserve(300);
  json = "{";
  json += "\"i2c\":{\"count\":" + String(diagnosticData.i2cCount) +
          ",\"devices\":\"" + diagnosticData.i2cDevices + "\"},";
  json += "\"gpio\":{\"total\":" + String(diagnosticData.totalGPIO) +
          ",\"list\":\"" + diagnosticData.gpioList + "\"}";
  json += "}";
  server.send(200, "application/json", json);
}

void handleLedsInfo() {
  String json;
  json.reserve(400);
  json = "{";
  json += "\"builtin\":{\"pin\":" + String(BUILTIN_LED_PIN) +
          ",\"status\":\"" + builtinLedTestResult + "\"},";
  json += "\"neopixel\":{\"pin\":" + String(LED_PIN) +
          ",\"count\":" + String(LED_COUNT) +
          ",\"status\":\"" + neopixelTestResult + "\"}";
  json += "}";
  server.send(200, "application/json", json);
}

void handleScreensInfo() {
  String json;
  json.reserve(900);  // Increased for driver field
  json = "{";
  json += "\"oled\":{\"available\":" + String(oledAvailable ? "true" : "false") +
          ",\"status\":\"" + oledTestResult + "\",";
  json += "\"pins\":{\"sda\":" + String(i2c_sda) + ",\"scl\":" + String(i2c_scl) + "},";
  json += "\"rotation\":" + String(oledRotation) + ",";
  json += "\"width\":" + String(oledWidth) + ",\"height\":" + String(oledHeight) + "}";

  #if ENABLE_TFT_DISPLAY
  json += ",\"tft\":{\"available\":true,";
  json += "\"status\":\"" + String(tftTestResult.length() > 0 ? tftTestResult : "Ready") + "\",";
  json += "\"driver\":\"" + tftDriver + "\",";  // v3.30.0: Current TFT driver
  json += "\"width\":" + String(tftWidth) + ",\"height\":" + String(tftHeight) + ",";
  json += "\"rotation\":" + String(tftRotation) + ",";
  json += "\"pins\":{\"miso\":" + String(tftMISO) + ",\"mosi\":" + String(tftMOSI) + ",\"sclk\":" + String(tftSCLK) +
          ",\"cs\":" + String(tftCS) + ",\"dc\":" + String(tftDC) + ",\"rst\":" + String(tftRST) +
          ",\"bl\":" + String(tftBL) + "}}";
  #else
  json += ",\"tft\":{\"available\":false,\"status\":\"Not enabled\"}";
  #endif

  json += "}";
  server.send(200, "application/json", json);
}

void handleOverview() {
  collectDiagnosticInfo();
  collectDetailedMemory();
  scanI2C();

  String json;
  json.reserve(2500);  // Reserve memory for the complete overview response
  json = "{";

  // Chip info
  json += "\"chip\":{";
  json += "\"model\":\"" + diagnosticData.chipModel + "\",";
  json += "\"revision\":\"" + diagnosticData.chipRevision + "\",";
  json += "\"cores\":" + String(diagnosticData.cpuCores) + ",";
  json += "\"freq\":" + String(diagnosticData.cpuFreqMHz) + ",";
  json += "\"mac\":\"" + diagnosticData.macAddress + "\",";
  json += "\"uptime\":" + String(diagnosticData.uptime);
  if (diagnosticData.temperature != -999) {
    json += ",\"temperature\":" + String(diagnosticData.temperature, 1);
  } else {
    json += ",\"temperature\":-999";
  }
  json += "},";

  // Memory info
  json += "\"memory\":{";
  json += "\"flash\":{\"real\":" + String(detailedMemory.flashSizeReal) +
          ",\"type\":\"" + getFlashType() + "\",\"speed\":\"" + getFlashSpeed() + "\"},";
  json += "\"sram\":{\"total\":" + String(detailedMemory.sramTotal) +
          ",\"free\":" + String(detailedMemory.sramFree) +
          ",\"used\":" + String(detailedMemory.sramUsed) + "},";
  json += "\"psram\":{\"total\":" + String(detailedMemory.psramTotal) +
          ",\"free\":" + String(detailedMemory.psramFree) +
          ",\"used\":" + String(detailedMemory.psramUsed) + "},";
  json += "\"fragmentation\":" + String(detailedMemory.fragmentationPercent, 1);
  json += "},";

  // WiFi info - Use translation key instead of translated string
  json += "\"wifi\":{";
  json += "\"ssid\":\"" + diagnosticData.wifiSSID + "\",";
  json += "\"rssi\":" + String(diagnosticData.wifiRSSI) + ",";
  json += "\"quality_key\":\"" + String(getWiFiSignalQualityKey()) + "\",";  // Return key, not translated string
  json += "\"quality\":\"" + getWiFiSignalQuality() + "\",";  // Keep for backward compatibility
  json += "\"ip\":\"" + diagnosticData.ipAddress + "\"";
  json += "},";

  // GPIO info
  json += "\"gpio\":{";
  json += "\"total\":" + String(diagnosticData.totalGPIO) + ",";
  json += "\"i2c_count\":" + String(diagnosticData.i2cCount) + ",";
  json += "\"i2c_devices\":\"" + diagnosticData.i2cDevices + "\"";
  json += "}";

  json += "}";
  server.send(200, "application/json", json);
}

void handleMemoryDetails() {
  collectDetailedMemory();

  String json;
  json.reserve(450);  // Reserve memory to avoid reallocations
  json = "{\"flash\":{\"real\":" + String(detailedMemory.flashSizeReal) + ",\"chip\":" + String(detailedMemory.flashSizeChip) + "},";
  json += "\"psram\":{\"available\":" + String(detailedMemory.psramAvailable ? "true" : "false") +
          ",\"configured\":" + String(detailedMemory.psramConfigured ? "true" : "false") +
          ",\"supported\":" + String(detailedMemory.psramBoardSupported ? "true" : "false") +
          ",\"type\":\"" + String(detailedMemory.psramType ? detailedMemory.psramType : Texts::unknown.str()) + "\"" +
          ",\"total\":" + String(detailedMemory.psramTotal) + ",\"free\":" + String(detailedMemory.psramFree) + "},";
  json += "\"sram\":{\"total\":" + String(detailedMemory.sramTotal) + ",\"free\":" + String(detailedMemory.sramFree) + "},";
  json += "\"fragmentation\":" + String(detailedMemory.fragmentationPercent, 1) + ",\"status\":\"" + detailedMemory.memoryStatus + "\"}";

  server.send(200, "application/json", json);
}

// ========== EXPORTS ==========
void handleExportTXT() {
  collectDiagnosticInfo();
  collectDetailedMemory();

  String txt;
  txt.reserve(4500);  // Reserve memory to avoid reallocations during export
  txt = "========================================\r\n";
  txt += String(Texts::title) + " " + String(Texts::version) + String(PROJECT_VERSION) + "\r\n";
  txt += "========================================\r\n\r\n";
  
  txt += "=== CHIP ===\r\n";
  txt += String(Texts::model) + ": " + diagnosticData.chipModel + " " + String(Texts::revision) + diagnosticData.chipRevision + "\r\n";
  txt += "CPU: " + String(diagnosticData.cpuCores) + " " + String(Texts::cores) + " @ " + String(diagnosticData.cpuFreqMHz) + " MHz\r\n";
  txt += "MAC WiFi: " + diagnosticData.macAddress + "\r\n";
  txt += "SDK: " + diagnosticData.sdkVersion + "\r\n";
  txt += "ESP-IDF: " + diagnosticData.idfVersion + "\r\n";
  if (diagnosticData.temperature != -999) {
    txt += String(Texts::cpu_temp) + ": " + String(diagnosticData.temperature, 1) + " °C\r\n";
  }
  txt += "\r\n";
  
  txt += "=== " + String(Texts::memory_details) + " ===\r\n";
  txt += "Flash (" + String(Texts::board) + "): " + String(detailedMemory.flashSizeReal / 1048576.0, 2) + " MB\r\n";
  txt += "Flash (IDE): " + String(detailedMemory.flashSizeChip / 1048576.0, 2) + " MB\r\n";
  txt += String(Texts::flash_type) + ": " + getFlashType() + " @ " + getFlashSpeed() + "\r\n";
  txt += "PSRAM: " + String(detailedMemory.psramTotal / 1048576.0, 2) + " MB";
  if (detailedMemory.psramAvailable) {
    txt += " (" + String(Texts::free) + ": " + String(detailedMemory.psramFree / 1048576.0, 2) + " MB)\r\n";
  } else if (detailedMemory.psramBoardSupported) {
    String psramHint = String(Texts::enable_psram_hint);
    psramHint.replace("%TYPE%", detailedMemory.psramType ? detailedMemory.psramType : "PSRAM");
    txt += " (" + String(Texts::supported_not_enabled) + " - " + psramHint + ")\r\n";
  } else {
    txt += " (" + String(Texts::not_detected) + ")\r\n";
  }
  txt += "SRAM: " + String(detailedMemory.sramTotal / 1024.0, 2) + " KB";
  txt += " (" + String(Texts::free) + ": " + String(detailedMemory.sramFree / 1024.0, 2) + " KB)\r\n";
  txt += String(Texts::memory_fragmentation) + ": " + String(detailedMemory.fragmentationPercent, 1) + "%\r\n";
  txt += String(Texts::memory_status) + ": " + detailedMemory.memoryStatus + "\r\n";
  txt += "\r\n";
  
  txt += "=== WIFI ===\r\n";
  txt += "SSID: " + diagnosticData.wifiSSID + "\r\n";
  txt += "RSSI: " + String(diagnosticData.wifiRSSI) + " dBm (" + getWiFiSignalQuality() + ")\r\n";
  txt += "IP: " + diagnosticData.ipAddress + "\r\n";
  txt += "Lien constant: " + getStableAccessURL() + " (" + String(diagnosticData.mdnsAvailable ? "actif" : "en attente") + ")\r\n";
  txt += String(Texts::subnet_mask) + ": " + WiFi.subnetMask().toString() + "\r\n";
  txt += String(Texts::gateway) + ": " + WiFi.gatewayIP().toString() + "\r\n";
  txt += "DNS: " + WiFi.dnsIP().toString() + "\r\n";
  txt += "\r\n";

  txt += "=== GPIO ===\r\n";
  txt += String(Texts::total_gpio) + ": " + String(diagnosticData.totalGPIO) + " " + String(Texts::pins) + "\r\n";
  txt += String(Texts::gpio_list) + ": " + diagnosticData.gpioList + "\r\n";
  txt += "\r\n";
  
  txt += "=== " + String(Texts::i2c_peripherals) + " ===\r\n";
  txt += String(Texts::device_count) + ": " + String(diagnosticData.i2cCount) + " - " + diagnosticData.i2cDevices + "\r\n";
  txt += "SPI: " + spiInfo + "\r\n";
  txt += "\r\n";
  
  txt += "=== " + String(Texts::test) + " ===\r\n";
  txt += String(Texts::builtin_led) + ": " + builtinLedTestResult + "\r\n";
  txt += String(Texts::neopixel) + ": " + neopixelTestResult + "\r\n";
  txt += "OLED: " + oledTestResult + "\r\n";
  txt += "ADC: " + adcTestResult + "\r\n";
  txt += "PWM: " + pwmTestResult + "\r\n";
  txt += "SD Card: " + sdTestResult + "\r\n";
  txt += "Rotary Encoder: " + rotaryTestResult + "\r\n";
  txt += "\r\n";
  
  txt += "=== " + String(Texts::performance_bench) + " ===\r\n";
  if (diagnosticData.cpuBenchmark > 0) {
    txt += "CPU: " + String(diagnosticData.cpuBenchmark) + " us (" + String(100000.0 / diagnosticData.cpuBenchmark, 2) + " MFLOPS)\r\n";
    txt += String(Texts::memory_benchmark) + ": " + String(diagnosticData.memBenchmark) + " us\r\n";
  } else {
    txt += String(Texts::not_tested) + "\r\n";
  }
  txt += "Stress test: " + stressTestResult + "\r\n";
  txt += "\r\n";
  
  unsigned long seconds = diagnosticData.uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  // === ENVIRONNEMENT ===
  txt += "=== ENVIRONNEMENT ===\r\n";
  txt += "AHT20: ";
  txt += envData.aht20_available ? "OK" : "Non détecté";
  txt += "\r\n";
  txt += "  Température (AHT20): ";
  txt += (envData.temperature_aht20 != -999.0 ? String(envData.temperature_aht20, 1) + " °C" : "N/A");
  txt += "\r\n";
  txt += "  Humidité: ";
  txt += (envData.humidity != -999.0 ? String(envData.humidity, 1) + " %" : "N/A");
  txt += "\r\n";
  txt += "  Statut: " + envData.aht20_status + "\r\n";
  txt += "BMP280: ";
  txt += envData.bmp280_available ? "OK" : "Non détecté";
  txt += "\r\n";
  txt += "  Température (BMP280): ";
  txt += (envData.temperature_bmp280 != -999.0 ? String(envData.temperature_bmp280, 1) + " °C" : "N/A");
  txt += "\r\n";
  txt += "  Pression: ";
  txt += (envData.pressure != -999.0 ? String(envData.pressure, 1) + " hPa" : "N/A");
  txt += "\r\n";
  txt += "  Altitude: ";
  txt += (envData.altitude != -999.0 ? String(envData.altitude, 1) + " m" : "N/A");
  txt += "\r\n";
  txt += "  Statut: " + envData.bmp280_status + "\r\n";
  txt += "Moyenne température: ";
  txt += (envData.temperature_avg != -999.0 ? String(envData.temperature_avg, 1) + " °C" : "N/A");
  txt += "\r\n";
  txt += "Statut global: " + envData.combined_status + "\r\n";
  txt += "\r\n";

  // === GPS ===
  txt += "=== GPS ===\r\n";
  txt += "Module: ";
  txt += gpsAvailable ? "OK" : "Non détecté";
  txt += "\r\n";
  txt += "  Statut: " + gpsData.status_str + "\r\n";
  txt += "  Fix: ";
  txt += gpsData.hasFix ? "Oui" : "Non";
  txt += "\r\n";
  txt += "  Satellites: ";
  txt += String(gpsData.satellites);
  txt += "\r\n";
  txt += "  Latitude: ";
  txt += (gpsData.hasFix ? String(gpsData.latitude, 6) : "N/A");
  txt += "\r\n";
  txt += "  Longitude: ";
  txt += (gpsData.hasFix ? String(gpsData.longitude, 6) : "N/A");
  txt += "\r\n";
  txt += "  Altitude: ";
  txt += (gpsData.hasFix ? String(gpsData.altitude, 1) + " m" : "N/A");
  txt += "\r\n";
  txt += "  Vitesse: ";
  txt += (gpsData.hasFix ? String(gpsData.speed, 2) + " noeuds" : "N/A");
  txt += "\r\n";
  txt += "  HDOP: ";
  txt += (gpsData.hasFix ? String(gpsData.hdop, 2) : "N/A");
  txt += "\r\n";
  txt += "  Date/Heure: ";
  if (gpsData.hasTime && gpsData.hasDate) {
    txt += String(gpsData.day) + "/" + String(gpsData.month) + "/" + String(gpsData.year) + " ";
    txt += (gpsData.hour < 10 ? "0" : "") + String(gpsData.hour) + ":";
    txt += (gpsData.minute < 10 ? "0" : "") + String(gpsData.minute) + ":";
    txt += (gpsData.second < 10 ? "0" : "") + String(gpsData.second);
  } else {
    txt += "N/A";
  }
  txt += "\r\n\r\n";

  // === SYSTEME ===
  txt += "=== SYSTEM ===\r\n";
  txt += String(Texts::uptime) + ": " + String(days) + "d " + String(hours % 24) + "h " + String(minutes % 60) + "m\r\n";
  txt += String(Texts::last_reset) + ": " + getResetReason() + "\r\n";
  txt += "\r\n";
  txt += "========================================\r\n";
  txt += String(Texts::export_generated) + " " + String(millis()/1000) + "s " + String(Texts::export_after_boot) + "\r\n";
  txt += "========================================\r\n";

  server.sendHeader("Content-Disposition", "attachment; filename=esp32_diagnostic_v"+ String(PROJECT_VERSION) +".txt");
  server.send(200, "text/plain; charset=utf-8", txt);
}

void handleExportJSON() {
  collectDiagnosticInfo();
  collectDetailedMemory();
  String stableUrl = getStableAccessURL();

  String json;
  json.reserve(3500);  // Reserve memory to avoid reallocations during export
  json = "{";
  json += "\"chip\":{";
  json += "\"model\":\"" + diagnosticData.chipModel + "\",";
  json += "\"revision\":\"" + diagnosticData.chipRevision + "\",";
  json += "\"cores\":" + String(diagnosticData.cpuCores) + ",";
  json += "\"freq_mhz\":" + String(diagnosticData.cpuFreqMHz) + ",";
  json += "\"mac\":\"" + diagnosticData.macAddress + "\",";
  json += "\"sdk\":\"" + diagnosticData.sdkVersion + "\",";
  json += "\"idf\":\"" + diagnosticData.idfVersion + "\"";
  if (diagnosticData.temperature != -999) {
    json += ",\"temperature\":" + String(diagnosticData.temperature, 1);
  }
  json += "},";
  
  json += "\"memory\":{";
  json += "\"flash_real_mb\":" + String(detailedMemory.flashSizeReal / 1048576.0, 2) + ",";
  json += "\"flash_config_mb\":" + String(detailedMemory.flashSizeChip / 1048576.0, 2) + ",";
  json += "\"flash_type\":\"" + getFlashType() + "\",";
  json += "\"flash_speed\":\"" + getFlashSpeed() + "\",";
  json += "\"psram_mb\":" + String(detailedMemory.psramTotal / 1048576.0, 2) + ",";
  json += "\"psram_free_mb\":" + String(detailedMemory.psramFree / 1048576.0, 2) + ",";
  json += "\"psram_available\":" + String(detailedMemory.psramAvailable ? "true" : "false") + ",";
  json += "\"psram_supported\":" + String(detailedMemory.psramBoardSupported ? "true" : "false") + ",";
  json += "\"psram_type\":\"" + String(detailedMemory.psramType ? detailedMemory.psramType : Texts::unknown.str()) + "\",";
  json += "\"sram_kb\":" + String(detailedMemory.sramTotal / 1024.0, 2) + ",";
  json += "\"sram_free_kb\":" + String(detailedMemory.sramFree / 1024.0, 2) + ",";
  json += "\"fragmentation\":" + String(detailedMemory.fragmentationPercent, 1) + ",";
  json += "\"status\":\"" + detailedMemory.memoryStatus + "\"";
  json += "},";
  
  json += "\"wifi\":{";
  json += "\"ssid\":\"" + diagnosticData.wifiSSID + "\",";
  json += "\"rssi\":" + String(diagnosticData.wifiRSSI) + ",";
  json += "\"quality\":\"" + getWiFiSignalQuality() + "\",";
  json += "\"ip\":\"" + diagnosticData.ipAddress + "\",";
  json += "\"mdns_ready\":" + String(diagnosticData.mdnsAvailable ? "true" : "false") + ",";
  json += "\"stable_url\":\"" + jsonEscape(stableUrl.c_str()) + "\",";
  json += "\"subnet\":\"" + WiFi.subnetMask().toString() + "\",";
  json += "\"gateway\":\"" + WiFi.gatewayIP().toString() + "\",";
  json += "\"dns\":\"" + WiFi.dnsIP().toString() + "\"";
  json += "},";

  json += "\"gpio\":{";
  json += "\"total\":" + String(diagnosticData.totalGPIO) + ",";
  json += "\"list\":\"" + diagnosticData.gpioList + "\"";
  json += "},";
  
  json += "\"peripherals\":{";
  json += "\"i2c_count\":" + String(diagnosticData.i2cCount) + ",";
  json += "\"i2c_devices\":\"" + diagnosticData.i2cDevices + "\",";
  json += "\"spi\":\"" + spiInfo + "\"";
  json += "},";
  
  json += "\"hardware_tests\":{";
  json += "\"builtin_led\":\"" + builtinLedTestResult + "\",";
  json += "\"neopixel\":\"" + neopixelTestResult + "\",";
  json += "\"oled\":\"" + oledTestResult + "\",";
  json += "\"adc\":\"" + adcTestResult + "\",";
  json += "\"pwm\":\"" + pwmTestResult + "\",";
  json += "\"sd_card\":\"" + sdTestResult + "\",";
  json += "\"rotary_encoder\":\"" + rotaryTestResult + "\"";
  json += "},";
  
  json += "\"performance\":{";
  if (diagnosticData.cpuBenchmark > 0) {
    json += "\"cpu_us\":" + String(diagnosticData.cpuBenchmark) + ",";
    json += "\"cpu_mflops\":" + String(100000.0 / diagnosticData.cpuBenchmark, 2) + ",";
    json += "\"memory_us\":" + String(diagnosticData.memBenchmark);
  } else {
    json += "\"benchmarks\":\"not_run\"";
  }
  json += ",\"stress_test\":\"" + stressTestResult + "\"";
  json += "},";
  
  // === ENVIRONNEMENT ===
  json += ",\"environment\":{";
  json += "\"aht20_available\":" + String(envData.aht20_available ? "true" : "false") + ",";
  json += "\"temperature_aht20\":" + (envData.temperature_aht20 != -999.0 ? String(envData.temperature_aht20, 1) : "null") + ",";
  json += "\"humidity\":" + (envData.humidity != -999.0 ? String(envData.humidity, 1) : "null") + ",";
  json += "\"aht20_status\":\"" + jsonEscape(envData.aht20_status.c_str()) + "\",";
  json += "\"bmp280_available\":" + String(envData.bmp280_available ? "true" : "false") + ",";
  json += "\"temperature_bmp280\":" + (envData.temperature_bmp280 != -999.0 ? String(envData.temperature_bmp280, 1) : "null") + ",";
  json += "\"pressure\":" + (envData.pressure != -999.0 ? String(envData.pressure, 1) : "null") + ",";
  json += "\"altitude\":" + (envData.altitude != -999.0 ? String(envData.altitude, 1) : "null") + ",";
  json += "\"bmp280_status\":\"" + jsonEscape(envData.bmp280_status.c_str()) + "\",";
  json += "\"temperature_avg\":" + (envData.temperature_avg != -999.0 ? String(envData.temperature_avg, 1) : "null") + ",";
  json += "\"combined_status\":\"" + jsonEscape(envData.combined_status.c_str()) + "\"";
  json += "},";

  // === GPS ===
  json += "\"gps\":{";
  json += "\"available\":" + String(gpsAvailable ? "true" : "false") + ",";
  json += "\"status\":\"" + jsonEscape(gpsData.status_str.c_str()) + "\",";
  json += "\"has_fix\":" + String(gpsData.hasFix ? "true" : "false") + ",";
  json += "\"satellites\":" + String(gpsData.satellites) + ",";
  json += "\"latitude\":" + (gpsData.hasFix ? String(gpsData.latitude, 6) : "null") + ",";
  json += "\"longitude\":" + (gpsData.hasFix ? String(gpsData.longitude, 6) : "null") + ",";
  json += "\"altitude\":" + (gpsData.hasFix ? String(gpsData.altitude, 1) : "null") + ",";
  json += "\"speed\":" + (gpsData.hasFix ? String(gpsData.speed, 2) : "null") + ",";
  json += "\"hdop\":" + (gpsData.hasFix ? String(gpsData.hdop, 2) : "null") + ",";
  json += "\"date_time\":\"";
  if (gpsData.hasTime && gpsData.hasDate) {
    json += String(gpsData.day) + "/" + String(gpsData.month) + "/" + String(gpsData.year) + " ";
    if (gpsData.hour < 10) json += "0";
    json += String(gpsData.hour) + ":";
    if (gpsData.minute < 10) json += "0";
    json += String(gpsData.minute) + ":";
    if (gpsData.second < 10) json += "0";
    json += String(gpsData.second);
  } else {
    json += "N/A";
  }
  json += "\"";
  json += "},";

  // === SYSTEME ===
  json += "\"system\":{";
  json += "\"uptime_ms\":" + String(diagnosticData.uptime) + ",";
  json += "\"reset_reason\":\"" + getResetReason() + "\",";
  json += "\"language\":\"en\"";
  json += "}";

  json += "}";

  server.sendHeader("Content-Disposition", "attachment; filename=esp32_diagnostic_v" + String(PROJECT_VERSION) + ".json");
  server.send(200, "application/json", json);
}

void handleExportCSV() {
  collectDiagnosticInfo();
  collectDetailedMemory();

  String csv;
  csv.reserve(4000);  // Reserve memory to avoid reallocations during export
  csv = String(Texts::category) + "," + String(Texts::parameter) + "," + String(Texts::value) + "\r\n";
  
  csv += "Chip," + String(Texts::model) + "," + diagnosticData.chipModel + "\r\n";
  csv += "Chip," + String(Texts::revision) + "," + diagnosticData.chipRevision + "\r\n";
  csv += "Chip,CPU " + String(Texts::cores) + "," + String(diagnosticData.cpuCores) + "\r\n";
  csv += "Chip," + String(Texts::frequency) + " MHz," + String(diagnosticData.cpuFreqMHz) + "\r\n";
  csv += "Chip,MAC," + diagnosticData.macAddress + "\r\n";
  if (diagnosticData.temperature != -999) {
    csv += "Chip," + String(Texts::cpu_temp) + " C," + String(diagnosticData.temperature, 1) + "\r\n";
  }
  
  csv += String(Texts::memory_details) + ",Flash MB (" + String(Texts::board) + ")," + String(detailedMemory.flashSizeReal / 1048576.0, 2) + "\r\n";
  csv += String(Texts::memory_details) + ",Flash MB (config)," + String(detailedMemory.flashSizeChip / 1048576.0, 2) + "\r\n";
  csv += String(Texts::memory_details) + "," + String(Texts::flash_type) + "," + getFlashType() + "\r\n";
  csv += String(Texts::memory_details) + ",PSRAM MB," + String(detailedMemory.psramTotal / 1048576.0, 2) + "\r\n";
  csv += String(Texts::memory_details) + ",PSRAM " + String(Texts::free) + " MB," + String(detailedMemory.psramFree / 1048576.0, 2) + "\r\n";
  String psramStatus = detailedMemory.psramAvailable ? String(Texts::detected_active)
                                                    : (detailedMemory.psramBoardSupported ? String(Texts::supported_not_enabled)
                                                                                        : String(Texts::not_detected));
  csv += String(Texts::memory_details) + ",PSRAM Statut,\"" + psramStatus + "\"\r\n";
  if (!detailedMemory.psramAvailable && detailedMemory.psramBoardSupported) {
    String psramHint = String(Texts::enable_psram_hint);
    psramHint.replace("%TYPE%", detailedMemory.psramType ? detailedMemory.psramType : "PSRAM");
    psramHint.replace("\"", "'");
    csv += String(Texts::memory_details) + ",PSRAM Conseils,\"" + psramHint + "\"\r\n";
  }
  csv += String(Texts::memory_details) + ",SRAM KB," + String(detailedMemory.sramTotal / 1024.0, 2) + "\r\n";
  csv += String(Texts::memory_details) + ",SRAM " + String(Texts::free) + " KB," + String(detailedMemory.sramFree / 1024.0, 2) + "\r\n";
  csv += String(Texts::memory_details) + "," + String(Texts::memory_fragmentation) + " %," + String(detailedMemory.fragmentationPercent, 1) + "\r\n";
  
  csv += "WiFi,SSID," + diagnosticData.wifiSSID + "\r\n";
  csv += "WiFi,RSSI dBm," + String(diagnosticData.wifiRSSI) + "\r\n";
  csv += "WiFi,IP," + diagnosticData.ipAddress + "\r\n";
  csv += "WiFi," + String(Texts::gateway) + "," + WiFi.gatewayIP().toString() + "\r\n";

  csv += "GPIO," + String(Texts::total_gpio) + "," + String(diagnosticData.totalGPIO) + "\r\n";
  
  csv += String(Texts::i2c_peripherals) + "," + String(Texts::device_count) + "," + String(diagnosticData.i2cCount) + "\r\n";
  csv += String(Texts::i2c_peripherals) + "," + String(Texts::devices) + "," + diagnosticData.i2cDevices + "\r\n";
  
  csv += String(Texts::test) + "," + String(Texts::builtin_led) + "," + builtinLedTestResult + "\r\n";
  csv += String(Texts::test) + "," + String(Texts::neopixel) + "," + neopixelTestResult + "\r\n";
  csv += String(Texts::test) + ",OLED," + oledTestResult + "\r\n";
  csv += String(Texts::test) + ",ADC," + adcTestResult + "\r\n";
  csv += String(Texts::test) + ",PWM," + pwmTestResult + "\r\n";
  csv += String(Texts::test) + ",SD Card," + sdTestResult + "\r\n";
  csv += String(Texts::test) + ",Rotary Encoder," + rotaryTestResult + "\r\n";

  if (diagnosticData.cpuBenchmark > 0) {
    csv += String(Texts::performance_bench) + ",CPU us," + String(diagnosticData.cpuBenchmark) + "\r\n";
    csv += String(Texts::performance_bench) + "," + String(Texts::memory_benchmark) + " us," + String(diagnosticData.memBenchmark) + "\r\n";
  }

  // === ENVIRONNEMENT ===
  csv += "Environnement,AHT20 disponible," + String(envData.aht20_available ? "Oui" : "Non") + "\r\n";
  csv += "Environnement,Température (AHT20)," + (envData.temperature_aht20 != -999.0 ? String(envData.temperature_aht20, 1) : "N/A") + "\r\n";
  csv += "Environnement,Humidité," + (envData.humidity != -999.0 ? String(envData.humidity, 1) : "N/A") + "\r\n";
  csv += "Environnement,Statut AHT20," + envData.aht20_status + "\r\n";
  csv += "Environnement,BMP280 disponible," + String(envData.bmp280_available ? "Oui" : "Non") + "\r\n";
  csv += "Environnement,Température (BMP280)," + (envData.temperature_bmp280 != -999.0 ? String(envData.temperature_bmp280, 1) : "N/A") + "\r\n";
  csv += "Environnement,Pression," + (envData.pressure != -999.0 ? String(envData.pressure, 1) : "N/A") + "\r\n";
  csv += "Environnement,Altitude," + (envData.altitude != -999.0 ? String(envData.altitude, 1) : "N/A") + "\r\n";
  csv += "Environnement,Statut BMP280," + envData.bmp280_status + "\r\n";
  csv += "Environnement,Température moyenne," + (envData.temperature_avg != -999.0 ? String(envData.temperature_avg, 1) : "N/A") + "\r\n";
  csv += "Environnement,Statut global," + envData.combined_status + "\r\n";

  // === GPS ===
  csv += "GPS,Module disponible," + String(gpsAvailable ? "Oui" : "Non") + "\r\n";
  csv += "GPS,Statut," + gpsData.status_str + "\r\n";
  csv += "GPS,Fix," + String(gpsData.hasFix ? "Oui" : "Non") + "\r\n";
  csv += "GPS,Satellites," + String(gpsData.satellites) + "\r\n";
  csv += "GPS,Latitude," + (gpsData.hasFix ? String(gpsData.latitude, 6) : "N/A") + "\r\n";
  csv += "GPS,Longitude," + (gpsData.hasFix ? String(gpsData.longitude, 6) : "N/A") + "\r\n";
  csv += "GPS,Altitude," + (gpsData.hasFix ? String(gpsData.altitude, 1) : "N/A") + "\r\n";
  csv += "GPS,Vitesse," + (gpsData.hasFix ? String(gpsData.speed, 2) : "N/A") + "\r\n";
  csv += "GPS,HDOP," + (gpsData.hasFix ? String(gpsData.hdop, 2) : "N/A") + "\r\n";
  csv += "GPS,Date/Heure,";
  if (gpsData.hasTime && gpsData.hasDate) {
    csv += String(gpsData.day) + "/" + String(gpsData.month) + "/" + String(gpsData.year) + " ";
    if (gpsData.hour < 10) csv += "0";
    csv += String(gpsData.hour) + ":";
    if (gpsData.minute < 10) csv += "0";
    csv += String(gpsData.minute) + ":";
    if (gpsData.second < 10) csv += "0";
    csv += String(gpsData.second);
  } else {
    csv += "N/A";
  }
  csv += "\r\n";

  csv += "System," + String(Texts::uptime) + " ms," + String(diagnosticData.uptime) + "\r\n";
  csv += "System," + String(Texts::last_reset) + "," + getResetReason() + "\r\n";

  server.sendHeader("Content-Disposition", "attachment; filename=esp32_diagnostic_v" + String(PROJECT_VERSION) + ".csv");
  server.send(200, "text/csv; charset=utf-8", csv);
}

void handlePrintVersion() {
  collectDiagnosticInfo();
  collectDetailedMemory();
  
  // [OPT-007]: Buffer-based HTML title (1 vs 4 allocations)
  char titleBuf[256];
  snprintf(titleBuf, sizeof(titleBuf), "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>%s %s%s</title>",
           Texts::title.str().c_str(), Texts::version.str().c_str(), PROJECT_VERSION);
  String html = String(titleBuf);
  html += "<style>";
  html += "@page{size:A4;margin:10mm}";
  html += "body{font:11px Arial;margin:10px;color:#333}";
  html += "h1{font-size:18px;margin:0 0 5px;border-bottom:3px solid #667eea;color:#667eea;padding-bottom:5px}";
  html += "h2{font-size:14px;margin:15px 0 8px;color:#667eea;border-bottom:1px solid #ddd;padding-bottom:3px}";
  html += ".section{margin-bottom:20px}";
  html += ".grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:10px 0}";
  html += ".row{display:flex;margin:3px 0;padding:5px;background:#f8f9fa;border-radius:3px}";
  html += ".row b{min-width:150px;color:#667eea}";
  html += ".badge{display:inline-block;padding:2px 8px;border-radius:10px;font-size:10px;font-weight:bold}";
  html += ".badge-success{background:#d4edda;color:#155724}";
  html += ".badge-warning{background:#fff3cd;color:#856404}";
  html += ".badge-danger{background:#f8d7da;color:#721c24}";
  html += "table{width:100%;border-collapse:collapse;margin:10px 0;font-size:10px}";
  html += "th{background:#667eea;color:#fff;padding:5px;text-align:left}";
  html += "td{border:1px solid #ddd;padding:4px}";
  html += ".footer{margin-top:20px;padding-top:10px;border-top:1px solid #ddd;font-size:9px;color:#666;text-align:center}";
  html += "</style></head>";
  html += "<body onload='window.print()'>";
  
  // Header traduit
  html += "<h1>" + String(Texts::title) + " " + String(Texts::version) + String(PROJECT_VERSION) + "</h1>";
  html += "<div style='margin:10px 0;font-size:12px;color:#666'>";
  html += String(Texts::export_generated) + " " + String(millis()/1000) + "s " + String(Texts::export_after_boot) + " | IP: " + diagnosticData.ipAddress;
  html += "</div>";
  
  // Chip
  html += "<div class='section'>";
  html += "<h2>" + String(Texts::chip_info) + "</h2>";
  html += "<div class='grid'>";
  html += "<div class='row'><b>" + String(Texts::full_model) + ":</b><span>" + diagnosticData.chipModel + " Rev" + diagnosticData.chipRevision + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::cpu_cores) + ":</b><span>" + String(diagnosticData.cpuCores) + " " + String(Texts::cores) + " @ " + String(diagnosticData.cpuFreqMHz) + " MHz</span></div>";
  html += "<div class='row'><b>" + String(Texts::mac_wifi) + ":</b><span>" + diagnosticData.macAddress + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::sdk_version) + ":</b><span>" + diagnosticData.sdkVersion + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::idf_version) + ":</b><span>" + diagnosticData.idfVersion + "</span></div>";
  if (diagnosticData.temperature != -999) {
    html += "<div class='row'><b>" + String(Texts::cpu_temp) + ":</b><span>" + String(diagnosticData.temperature, 1) + " °C</span></div>";
  }

  unsigned long seconds = diagnosticData.uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  html += "<div class='row'><b>" + String(Texts::uptime) + ":</b><span>" + formatUptime(days, hours % 24, minutes % 60) + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::last_reset) + ":</b><span>" + getResetReason() + "</span></div>";
  html += "</div></div>";


  // Mémoire
  html += "<div class='section'>";
  html += "<h2>" + String(Texts::memory_details) + "</h2>";
  html += "<table>";
  html += "<tr><th>" + String(Texts::category) + "</th><th>" + String(Texts::total_size) + "</th><th>" + String(Texts::free) + "</th><th>" + String(Texts::used) + "</th><th>" + String(Texts::status) + "</th></tr>";
  // ...existing code for memory table...
  bool flashMatch = (detailedMemory.flashSizeReal == detailedMemory.flashSizeChip);
  html += "<tr><td><b>" + String(Texts::flash_memory) + "</b></td>";
  html += "<td>" + String(detailedMemory.flashSizeReal / 1048576.0, 1) + " MB</td>";
  html += "<td>-</td><td>-</td>";
  html += "<td><span class='badge " + String(flashMatch ? "badge-success'>" + String(Texts::ok) : "badge-warning'>" + String(Texts::ide_config)) + "</span></td></tr>";
  // ...existing code for PSRAM and SRAM...
  if (detailedMemory.psramAvailable) {
    html += "<tr><td><b>" + String(Texts::psram_external) + "</b></td>";
    html += "<td>" + String(detailedMemory.psramTotal / 1048576.0, 1) + " MB</td>";
    html += "<td>" + String(detailedMemory.psramFree / 1048576.0, 1) + " MB</td>";
    html += "<td>" + String(detailedMemory.psramUsed / 1048576.0, 1) + " MB</td>";
    html += "<td><span class='badge badge-success'>" + String(Texts::detected_active) + "</span></td></tr>";
  } else if (detailedMemory.psramBoardSupported) {
    html += "<tr><td><b>" + String(Texts::psram_external) + "</b></td>";
    html += "<td>" + String(detailedMemory.psramTotal / 1048576.0, 1) + " MB</td>";
    html += "<td>-</td><td>-</td>";
    String psramHint = String(Texts::enable_psram_hint);
    psramHint.replace("%TYPE%", detailedMemory.psramType ? detailedMemory.psramType : "PSRAM");
    html += "<td><span class='badge badge-warning'>" + String(Texts::supported_not_enabled) + "</span><br><small>" + psramHint + "</small></td></tr>";
  } else {
    html += "<tr><td><b>" + String(Texts::psram_external) + "</b></td>";
    html += "<td>" + String(detailedMemory.psramTotal / 1048576.0, 1) + " MB</td>";
    html += "<td>-</td><td>-</td>";
    html += "<td><span class='badge badge-danger'>" + String(Texts::not_detected) + "</span></td></tr>";
  }
  html += "<tr><td><b>" + String(Texts::internal_sram) + "</b></td>";
  html += "<td>" + String(detailedMemory.sramTotal / 1024.0, 1) + " KB</td>";
  html += "<td>" + String(detailedMemory.sramFree / 1024.0, 1) + " KB</td>";
  html += "<td>" + String(detailedMemory.sramUsed / 1024.0, 1) + " KB</td>";
  html += "<td><span class='badge badge-success'>" + detailedMemory.memoryStatus + "</span></td></tr>";
  html += "</table>";
  html += "<div class='row'><b>" + String(Texts::memory_fragmentation) + ":</b><span>" + String(detailedMemory.fragmentationPercent, 1) + "% - " + detailedMemory.memoryStatus + "</span></div>";
  html += "</div>";

  // === ENVIRONNEMENT ===
  html += "<div class='section'>";
  html += "<h2>Environnement</h2>";
  html += "<table>";
  html += "<tr><th>Capteur</th><th>Paramètre</th><th>Valeur</th></tr>";
  html += "<tr><td>AHT20</td><td>Disponible</td><td>" + String(envData.aht20_available ? "Oui" : "Non") + "</td></tr>";
  html += "<tr><td>AHT20</td><td>Température</td><td>" + (envData.temperature_aht20 != -999.0 ? String(envData.temperature_aht20, 1) + " °C" : "N/A") + "</td></tr>";
  html += "<tr><td>AHT20</td><td>Humidité</td><td>" + (envData.humidity != -999.0 ? String(envData.humidity, 1) + " %" : "N/A") + "</td></tr>";
  html += "<tr><td>AHT20</td><td>Statut</td><td>" + envData.aht20_status + "</td></tr>";
  html += "<tr><td>BMP280</td><td>Disponible</td><td>" + String(envData.bmp280_available ? "Oui" : "Non") + "</td></tr>";
  html += "<tr><td>BMP280</td><td>Température</td><td>" + (envData.temperature_bmp280 != -999.0 ? String(envData.temperature_bmp280, 1) + " °C" : "N/A") + "</td></tr>";
  html += "<tr><td>BMP280</td><td>Pression</td><td>" + (envData.pressure != -999.0 ? String(envData.pressure, 1) + " hPa" : "N/A") + "</td></tr>";
  html += "<tr><td>BMP280</td><td>Altitude</td><td>" + (envData.altitude != -999.0 ? String(envData.altitude, 1) + " m" : "N/A") + "</td></tr>";
  html += "<tr><td>BMP280</td><td>Statut</td><td>" + envData.bmp280_status + "</td></tr>";
  html += "<tr><td colspan='2'>Température moyenne</td><td>" + (envData.temperature_avg != -999.0 ? String(envData.temperature_avg, 1) + " °C" : "N/A") + "</td></tr>";
  html += "<tr><td colspan='2'>Statut global</td><td>" + envData.combined_status + "</td></tr>";
  html += "</table></div>";

  // === GPS ===
  html += "<div class='section'>";
  html += "<h2>GPS</h2>";
  html += "<table>";
  html += "<tr><th>Paramètre</th><th>Valeur</th></tr>";
  html += "<tr><td>Module disponible</td><td>" + String(gpsAvailable ? "Oui" : "Non") + "</td></tr>";
  html += "<tr><td>Statut</td><td>" + gpsData.status_str + "</td></tr>";
  html += "<tr><td>Fix</td><td>" + String(gpsData.hasFix ? "Oui" : "Non") + "</td></tr>";
  html += "<tr><td>Satellites</td><td>" + String(gpsData.satellites) + "</td></tr>";
  html += "<tr><td>Latitude</td><td>" + (gpsData.hasFix ? String(gpsData.latitude, 6) : "N/A") + "</td></tr>";
  html += "<tr><td>Longitude</td><td>" + (gpsData.hasFix ? String(gpsData.longitude, 6) : "N/A") + "</td></tr>";
  html += "<tr><td>Altitude</td><td>" + (gpsData.hasFix ? String(gpsData.altitude, 1) + " m" : "N/A") + "</td></tr>";
  html += "<tr><td>Vitesse</td><td>" + (gpsData.hasFix ? String(gpsData.speed, 2) + " noeuds" : "N/A") + "</td></tr>";
  html += "<tr><td>HDOP</td><td>" + (gpsData.hasFix ? String(gpsData.hdop, 2) : "N/A") + "</td></tr>";
  html += "<tr><td>Date/Heure</td><td>";
  if (gpsData.hasTime && gpsData.hasDate) {
    html += String(gpsData.day) + "/" + String(gpsData.month) + "/" + String(gpsData.year) + " ";
    if (gpsData.hour < 10) html += "0";
    html += String(gpsData.hour) + ":";
    if (gpsData.minute < 10) html += "0";
    html += String(gpsData.minute) + ":";
    if (gpsData.second < 10) html += "0";
    html += String(gpsData.second);
  } else {
    html += "N/A";
  }
  html += "</td></tr>";
  html += "</table></div>";

  // WiFi
  html += "<div class='section'>";
  html += "<h2>" + String(Texts::wifi_connection) + "</h2>";
  html += "<div class='grid'>";
  html += "<div class='row'><b>" + String(Texts::connected_ssid) + ":</b><span>" + diagnosticData.wifiSSID + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::signal_power) + ":</b><span>" + String(diagnosticData.wifiRSSI) + " dBm</span></div>";
  html += "<div class='row'><b>" + String(Texts::signal_quality) + ":</b><span>" + getWiFiSignalQuality() + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::ip_address) + ":</b><span>" + diagnosticData.ipAddress + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::subnet_mask) + ":</b><span>" + WiFi.subnetMask().toString() + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::dns) + ":</b><span>" + WiFi.dnsIP().toString() + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::gateway) + ":</b><span>" + WiFi.gatewayIP().toString() + "</span></div>";
  html += "</div></div>";

  // GPIO et Périphériques
  html += "<div class='section'>";
  html += "<h2>" + String(Texts::gpio_interfaces) + "</h2>";
  html += "<div class='grid'>";
  html += "<div class='row'><b>" + String(Texts::total_gpio) + ":</b><span>" + String(diagnosticData.totalGPIO) + " " + String(Texts::pins) + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::i2c_peripherals) + ":</b><span>" + String(diagnosticData.i2cCount) + " " + String(Texts::devices) + " - " + diagnosticData.i2cDevices + "</span></div>";
  html += "<div class='row'><b>" + String(Texts::spi_bus) + ":</b><span>" + spiInfo + "</span></div>";
  html += "</div></div>";

  // Tests Matériels
  html += "<div class='section'>";
  html += "<h2>" + String(Texts::nav_tests) + "</h2>";
  html += "<table>";
  html += "<tr><th>" + String(Texts::parameter) + "</th><th>" + String(Texts::status) + "</th></tr>";
  html += "<tr><td>" + String(Texts::builtin_led) + "</td><td>" + builtinLedTestResult + "</td></tr>";
  html += "<tr><td>" + String(Texts::neopixel) + "</td><td>" + neopixelTestResult + "</td></tr>";
  html += "<tr><td>" + String(Texts::oled_screen) + "</td><td>" + oledTestResult + "</td></tr>";
  html += "<tr><td>" + String(Texts::adc_test) + "</td><td>" + adcTestResult + "</td></tr>";
  html += "<tr><td>" + String(Texts::pwm_test) + "</td><td>" + pwmTestResult + "</td></tr>";
  html += "</table></div>";

  // Performance
  if (diagnosticData.cpuBenchmark > 0) {
    html += "<div class='section'>";
    html += "<h2>" + String(Texts::performance_bench) + "</h2>";
    html += "<div class='grid'>";
    html += "<div class='row'><b>" + String(Texts::cpu_benchmark) + ":</b><span>" + String(diagnosticData.cpuBenchmark) + " µs (" + String(100000.0 / diagnosticData.cpuBenchmark, 2) + " MFLOPS)</span></div>";
    html += "<div class='row'><b>" + String(Texts::memory_benchmark) + ":</b><span>" + String(diagnosticData.memBenchmark) + " µs</span></div>";
    html += "<div class='row'><b>" + String(Texts::memory_stress) + ":</b><span>" + stressTestResult + "</span></div>";
    html += "</div></div>";
  }
  
  // Footer
  html += "<div class='footer'>";
  html += String(PROJECT_NAME) + " v"+ String(PROJECT_VERSION) + " | " + diagnosticData.chipModel + " | MAC: " + diagnosticData.macAddress;
  html += "</div>";
  
  html += "</body></html>";
  
  server.send(200, "text/html; charset=utf-8", html);
}

String htmlEscape(const String& raw) {
  String escaped;
  escaped.reserve(raw.length() * 6);
  for (size_t i = 0; i < raw.length(); ++i) {
    char c = raw[i];
    switch (c) {
      case '&':
        escaped += "&amp;";
        break;
      case '<':
        escaped += "&lt;";
        break;
      case '>':
        escaped += "&gt;";
        break;
      case '\"':
        escaped += "&quot;";
        break;
      case '\'':
        escaped += "&#39;";
        break;
      default:
        escaped += c;
        break;
    }
  }
  return escaped;
}

String jsonEscape(const char* raw) {
  if (raw == nullptr) {
    return "";
  }

  const size_t rawLength = strlen(raw);
  String escaped;
  escaped.reserve(rawLength * 2);
  while (*raw) {
    char c = *raw++;
    switch (c) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped += c;
        break;
    }
  }
  return escaped;
}

static inline void appendJsonField(String& json, bool& first, const JsonFieldSpec& field) {
  if (!first) {
    json += ',';
  }
  first = false;
  json += '"';
  json += field.key;
  json += '"';
  json += ':';
  if (field.raw) {
    json += field.value;
  } else {
    json += '"';
    json += jsonEscape(field.value.c_str());
    json += '"';
  }
}

String buildJsonObject(std::initializer_list<JsonFieldSpec> fields) {
  String json;
  json.reserve(fields.size() * 50 + 10);  // Estimate size based on field count
  json = "{";
  bool first = true;
  for (const auto& field : fields) {
    appendJsonField(json, first, field);
  }
  json += '}';
  return json;
}

inline void sendJsonResponse(int statusCode, std::initializer_list<JsonFieldSpec> fields) {
  server.send(statusCode, "application/json", buildJsonObject(fields));
}

static String buildActionResponseJson(bool success,
                                      const String& message,
                                      std::initializer_list<JsonFieldSpec> extraFields) {
  String json;
  json.reserve(extraFields.size() * 50 + message.length() + 50);  // Estimate size
  json = "{";
  bool first = true;
  appendJsonField(json, first, jsonBoolField("success", success));
  if (message.length() > 0) {
    appendJsonField(json, first, jsonStringField("message", message));
  }
  for (const auto& field : extraFields) {
    appendJsonField(json, first, field);
  }
  json += '}';
  return json;
}

inline void sendActionResponse(int statusCode,
                               bool success,
                               const String& message,
                               std::initializer_list<JsonFieldSpec> extraFields) {
  server.send(statusCode, "application/json", buildActionResponseJson(success, message, extraFields));
}

inline void sendOperationSuccess(const String& message,
                                 std::initializer_list<JsonFieldSpec> extraFields) {
  sendActionResponse(200, true, message, extraFields);
}

inline void sendOperationError(int statusCode,
                               const String& message,
                               std::initializer_list<JsonFieldSpec> extraFields) {
  sendActionResponse(statusCode, false, message, extraFields);
}

static inline void appendInfoItem(String& chunk,
                                  const char* labelKey,
                                  const char* labelText,
                                  const String& valueText,
                                  const String& valueAttrs = String(),
                                  const String& wrapperAttrs = String()) {
  chunk += F("<div class='info-item'");
  if (wrapperAttrs.length() > 0) {
    chunk += ' ';
    chunk += wrapperAttrs;
  }
  chunk += F("><div class='info-label");
  if (labelKey != nullptr && labelKey[0] != '\0') {
    chunk += F(" data-i18n='");
    chunk += labelKey;
    chunk += F("'>");
  } else {
    chunk += F("'>");
  }
  chunk += labelText;
  chunk += F("</div><div class='info-value'");
  if (valueAttrs.length() > 0) {
    chunk += ' ';
    chunk += valueAttrs;
  }
  chunk += F(">");
  chunk += valueText;
  chunk += F("</div></div>");
}

// Dynamic bilingual string export for the web interface
String buildTranslationsJSON(Language lang) {
  String json;
  json.reserve(20000);
  json += '{';
  bool first = true;
  size_t count = 0;
  const Texts::ResourceEntry* const entries = Texts::getResourceEntries(count);
  for (size_t i = 0; i < count; ++i) {
    const Texts::ResourceEntry& entry = entries[i];
    if (entry.field == nullptr) {
      continue;
    }
    if (!first) {
      json += ',';
    }
    first = false;
    json += '"';
    json += entry.key;
    json += F("\":\"");
    const __FlashStringHelper* raw = entry.field->get(lang);
    String fieldValue;
    if (raw != nullptr) {
      fieldValue = String(raw);
    }
    json += jsonEscape(fieldValue.c_str());
    json += '"';
  }
  json += '}';
  return json;
}

String buildTranslationsJSON() {
  return buildTranslationsJSON(currentLanguage);
}





// Runtime language switching endpoint
void handleSetLanguage() {
  if (!server.hasArg("lang")) {
    sendJsonResponse(400, {
      jsonBoolField("success", false),
      jsonStringField("error", String(Texts::language_switch_error))
    });
    return;
  }

  String lang = server.arg("lang");
  lang.toLowerCase();
  Language target = LANG_EN;
  if (lang == "fr") {
    target = LANG_FR;
  } else if (lang == "en") {
    target = LANG_EN;
  } else {
    sendJsonResponse(400, {
      jsonBoolField("success", false),
      jsonStringField("error", String(Texts::language_switch_error))
    });
    return;
  }

  setLanguage(target);

  String response = F("{\"success\":true,\"language\":\"");
  response += (target == LANG_FR) ? "fr" : "en";
  response += F("\"}");
  server.send(200, "application/json; charset=utf-8", response);
}

// Language-aware translation extraction endpoint
void handleGetTranslations() {
  Language target = currentLanguage;
  if (server.hasArg("lang")) {
    String requested = server.arg("lang");
    requested.toLowerCase();
    if (requested == "fr") {
      target = LANG_FR;
    } else if (requested == "en") {
      target = LANG_EN;
    } else {
      sendJsonResponse(400, {
        jsonBoolField("success", false),
        jsonStringField("error", String(Texts::language_switch_error))
      });
      return;
    }
  }

  server.send(200, "application/json; charset=utf-8", buildTranslationsJSON(target));
}

// Log client connection with IP address
void logClientConnection(const char* endpoint) {
  if (server.client()) {
    IPAddress clientIP = server.client().remoteIP();
    Serial.printf("[Client] %s connected from IP: %s\r\n", endpoint, clientIP.toString().c_str());
  }
}

// ========== INTERFACE WEB PRINCIPALE MULTILINGUE ==========
// Unique JavaScript handler defined in sketch (handleJavaScriptRoute)
void handleJavaScriptRoute() {
  logClientConnection("JS Resource");
  unsigned long startTime = millis();

  // Use chunked transfer to avoid memory issues
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/javascript; charset=utf-8", "");

  // ===== LOGS DE DÉBOGAGE =====
  Serial.println("\n========== JAVASCRIPT DEBUG (CHUNKED) ==========");
  Serial.printf("Free heap at start: %d bytes\n", ESP.getFreeHeap());

  // Send preamble
  String preamble = "console.log('";
  preamble += String(PROJECT_NAME) + " v";
  preamble += DIAGNOSTIC_VERSION_STR;
  preamble += " - Initialisation');const UPDATE_INTERVAL=5000;let currentLang='";
  preamble += (currentLanguage == LANG_FR) ? "fr" : "en";
  preamble += "';let updateTimer=null;let isConnected=true;";

  Serial.printf("Sending preamble: %d bytes\n", preamble.length());
  server.sendContent(preamble);

  // Send pin constants from board_config.h (BEFORE translations and static code)
  String pinVars = "const RGB_LED_PIN_R=";
  pinVars += String(rgb_led_pin_r);
  pinVars += ";const RGB_LED_PIN_G=";
  pinVars += String(rgb_led_pin_g);
  pinVars += ";const RGB_LED_PIN_B=";
  pinVars += String(rgb_led_pin_b);
  pinVars += ";const DHT_PIN=";
  pinVars += String(dht_pin);
  pinVars += ";const LIGHT_SENSOR_PIN=";
  pinVars += String(light_sensor_pin);
  pinVars += ";const DISTANCE_TRIG_PIN=";
  pinVars += String(distance_trig_pin);
  pinVars += ";const DISTANCE_ECHO_PIN=";
  pinVars += String(distance_echo_pin);
  pinVars += ";const MOTION_SENSOR_PIN=";
  pinVars += String(motion_sensor_pin);
  pinVars += ";const PWM_PIN=";
  pinVars += String(pwm_pin);
  pinVars += ";const BUZZER_PIN=";
  pinVars += String(buzzer_pin);
  pinVars += ";const SD_MISO_PIN=";
  pinVars += String(sd_miso_pin);
  pinVars += ";const SD_MOSI_PIN=";
  pinVars += String(sd_mosi_pin);
  pinVars += ";const SD_SCLK_PIN=";
  pinVars += String(sd_sclk_pin);
  pinVars += ";const SD_CS_PIN=";
  pinVars += String(sd_cs_pin);
  pinVars += ";const ROTARY_CLK_PIN=";
  pinVars += String(rotary_clk_pin);
  pinVars += ";const ROTARY_DT_PIN=";
  pinVars += String(rotary_dt_pin);
  pinVars += ";const ROTARY_SW_PIN=";
  pinVars += String(rotary_sw_pin);
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
  pinVars += ";console.log('GPIO Pins from board_config:',{SD_MISO:SD_MISO_PIN,SD_MOSI:SD_MOSI_PIN,SD_SCLK:SD_SCLK_PIN,SD_CS:SD_CS_PIN,ROTARY_CLK:ROTARY_CLK_PIN,ROTARY_DT:ROTARY_DT_PIN,ROTARY_SW:ROTARY_SW_PIN,BUTTON_BOOT:BUTTON_BOOT,BUTTON_1:BUTTON_1,BUTTON_2:BUTTON_2,TFT_MISO:TFT_MISO_PIN});";

  Serial.printf("Sending pin variables: %d bytes\n", pinVars.length());
  Serial.printf("  RGB_R=%d, RGB_G=%d, RGB_B=%d\n", rgb_led_pin_r, rgb_led_pin_g, rgb_led_pin_b);
  Serial.printf("  DHT=%d, LIGHT=%d, MOTION=%d, PWM=%d\n", dht_pin, light_sensor_pin, motion_sensor_pin, buzzer_pin);
  Serial.printf("  DIST_TRIG=%d, DIST_ECHO=%d\n", distance_trig_pin, distance_echo_pin);
  Serial.printf("  SD_MISO=%d, SD_MOSI=%d, SD_SCLK=%d, SD_CS=%d\n", sd_miso_pin, sd_mosi_pin, sd_sclk_pin, sd_cs_pin);
  Serial.printf("  ROTARY_CLK=%d, ROTARY_DT=%d, ROTARY_SW=%d\n", rotary_clk_pin, rotary_dt_pin, rotary_sw_pin);
  Serial.printf("  BUTTON_BOOT=%d, BUTTON_1=%d, BUTTON_2=%d\n", BUTTON_BOOT, BUTTON_1, BUTTON_2);
  Serial.printf("  TFT_MISO=%d\n", TFT_MISO);
  server.sendContent(pinVars);

  // Send translations
  String translations = "const DEFAULT_TRANSLATIONS=";
  translations += buildTranslationsJSON();
  translations += ";let translationsCache=DEFAULT_TRANSLATIONS;";

  Serial.printf("Sending translations: %d bytes\n", translations.length());
  server.sendContent(translations);

  // Send main JavaScript from PROGMEM without allocating a giant String
  // Stream in chunks to avoid heap exhaustion on ESP32 classic
  const char* staticJsPtr = DIAGNOSTIC_JS_STATIC;
  size_t staticJsLen = strlen_P(staticJsPtr);

  Serial.printf("Sending static JS (chunked): %d bytes\n", staticJsLen);
  const size_t CHUNK_SIZE = 1024;
  char chunkBuf[CHUNK_SIZE + 1];
  size_t sent = 0;
  while (sent < staticJsLen) {
    size_t n = (staticJsLen - sent) > CHUNK_SIZE ? CHUNK_SIZE : (staticJsLen - sent);
    memcpy_P(chunkBuf, staticJsPtr + sent, n);
    chunkBuf[n] = '\0';
    server.sendContent(chunkBuf);
    sent += n;
  }

  // Verify critical functions are present (direct pointer access works on ESP32)
  bool hasShowTab = (strstr(staticJsPtr, "function showTab") != NULL);
  bool hasChangeLang = (strstr(staticJsPtr, "function changeLang") != NULL);

  Serial.printf("Function showTab: %s\n", hasShowTab ? "YES" : "NO [ERROR]");
  Serial.printf("Function changeLang: %s\n", hasChangeLang ? "YES" : "NO [ERROR]");

  if (!hasShowTab || !hasChangeLang) {
    Serial.println("CRITICAL ERROR: JS functions missing in PROGMEM!");
    Serial.println("Check DIAGNOSTIC_JS_STATIC in web_interface.h");
  }

  // End chunked transfer
  server.sendContent("");

  unsigned long generateTime = millis() - startTime;
  Serial.printf("Total generation time: %lu ms\n", generateTime);
  Serial.printf("Free heap at end: %d bytes\n", ESP.getFreeHeap());
  Serial.println("======================================\n");
}

// Modern web interface with dynamic tabs
void handleRoot() {
  logClientConnection("Web Interface");
  server.send(200, "text/html; charset=utf-8", generateHTML());
}

// ========== DEBUG ROUTES (Troubleshooting) ==========
void setupDebugRoutes() {
  // Minimal test JavaScript
  server.on("/js/test.js", []() {
    String js = "console.log('Test JS loaded successfully');\n";
    js += "function showTab(n,b){console.log('showTab called:',n);alert('Test showTab: '+n);}\n";
    js += "function changeLang(l,b){console.log('changeLang called:',l);alert('Test changeLang: '+l);}\n";
    js += "console.log('Test functions defined');\n";
    server.send(200, "application/javascript; charset=utf-8", js);
  });

  // Debug status endpoint
  server.on("/debug/status", []() {
    String json = "{";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"jsSize\":" + String(generateJavaScript().length()) + ",";
    json += "\"version\":\"" + String(DIAGNOSTIC_VERSION_STR) + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  Serial.println("[DEBUG] Debug routes installed: /js/test.js, /debug/status");
}

// ========== SETUP COMPLET ==========
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\r\n===============================================");
  Serial.println("     DIAGNOSTIC ESP32 MULTILINGUE");
  Serial.printf("     Version %s - FR/EN\r\n", DIAGNOSTIC_VERSION_STR);
  Serial.printf("     Arduino Core %s\r\n", getArduinoCoreVersionString().c_str());
#if defined(TARGET_ESP32_S3)
  Serial.println("     TARGET: ESP32-S3 (N16R8/N8R8)");
#elif defined(TARGET_ESP32_CLASSIC)
  Serial.println("     TARGET: ESP32 CLASSIC (DevKitC)");
#else
  Serial.println("     TARGET: UNKNOWN - CHECK platformio.ini!");
#endif
  Serial.printf("     RGB LED Pins: R=%d G=%d B=%d\r\n", rgb_led_pin_r, rgb_led_pin_g, rgb_led_pin_b);
  Serial.println("===============================================\r\n");

  printPSRAMDiagnostic();

  // Early OLED detection for WiFi status feedback
  detectOLED();

  // TFT Display initialization
  if (initTFT()) {
    displayBootSplash();
    delay(1000);
  }

  // NeoPixel (WiFi feedback heartbeat)
  detectNeoPixelSupport();
  if (strip != nullptr) {
    strip->begin();
    strip->setBrightness(60);
    strip->clear();
    strip->show();
  }

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);

  WiFi.setSleep(false);
  configureNetworkHostname();
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 3, 0)
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
#ifdef WIFI_CONNECT_AP_BY_SIGNAL
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
#endif
#endif
  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

  Serial.println("Connexion WiFi...");
  const int maxWiFiAttempts = 40;
  int attempt = 0;
  neopixelShowConnecting();
  if (oledAvailable) {
    oledShowWiFiStatus(String(Texts::wifi_connection),
                       String(Texts::loading),
                       "",
                       0);
  }
  displayWiFiStatus("Connecting to WiFi...");
  while (wifiMulti.run() != WL_CONNECTED && attempt < maxWiFiAttempts) {
    delay(500);
    Serial.print(".");
    attempt++;
    if (oledAvailable) {
      int progress = (attempt * 100) / maxWiFiAttempts;
      if (progress > 100) {
        progress = 100;
      }
      // [OPT-007]: Buffer-based progress text (1 vs 3 allocations)
      char detailBuf[64];
      snprintf(detailBuf, sizeof(detailBuf), "%s %d%%", Texts::loading.str().c_str(), progress);
      oledShowWiFiStatus(String(Texts::wifi_connection),
                         String(detailBuf),
                         "",
                         progress);
    }
  }

  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
#if DIAGNOSTIC_HAS_MDNS
  wifiPreviouslyConnected = wifiConnected;
#endif

  if (wifiConnected) {
    Serial.println("\r\n\r\nWiFi OK!");
    Serial.printf("SSID: %s\r\n", WiFi.SSID().c_str());
    Serial.printf("IP: %s\r\n\r\n", WiFi.localIP().toString().c_str());
    if (oledAvailable) {
      // [OPT-007]: Buffer-based WiFi status detail (1 vs 2 allocations)
      char detailBuf[96];
      snprintf(detailBuf, sizeof(detailBuf), "%s: %s", Texts::connected.str().c_str(), WiFi.SSID().c_str());
      String footer = WiFi.localIP().toString();
      oledShowWiFiStatus(String(Texts::wifi_connection), String(detailBuf), footer, 100);
    }
    displayWiFiConnected(WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    neopixelSetWifiState(true);
    if (startMDNSService(true)) {
      Serial.printf("[Accès] Lien constant : %s\r\n", getStableAccessURL().c_str());
    } else {
      Serial.println("[mDNS] Initialisation différée - nouvel essai automatique");
    }
  } else {
    Serial.println("\r\n\r\nPas de WiFi\r\n");
    Serial.printf("[Accès] Lien constant disponible après connexion : %s\r\n", getStableAccessURL().c_str());
    if (oledAvailable) {
      oledShowWiFiStatus(String(Texts::wifi_connection),
                         String(Texts::disconnected),
                         getStableAccessURL(),
                         -1);
    }
  }
  
  scanSPI();


  listPartitions();
  
  collectDiagnosticInfo();
  collectDetailedMemory();

  // Initialize GPS module
  initGPS();
  
  // Initialize environmental sensors (AHT20 + BMP280)
  initEnvironmentalSensors();

  // ========== ROUTES SERVEUR ==========
  server.on("/", handleRoot);
  server.on("/js/app.js", handleJavaScriptRoute);

  // **TRANSLATION API**
  server.on("/api/get-translations", handleGetTranslations);
  server.on("/api/set-language", handleSetLanguage);

  // Data endpoints
  server.on("/api/status", handleStatus);
  server.on("/api/overview", handleOverview);
  server.on("/api/system-info", handleSystemInfo);
  server.on("/api/memory", handleMemory);
  server.on("/api/wifi-info", handleWiFiInfo);
  server.on("/api/peripherals", handlePeripherals);
  server.on("/api/leds-info", handleLedsInfo);
  server.on("/api/screens-info", handleScreensInfo);

  // GPIO & WiFi
  server.on("/api/test-gpio", handleTestGPIO);
  server.on("/api/wifi-scan", handleWiFiScan);
  server.on("/api/i2c-scan", handleI2CScan);

  // LED intégrée
  server.on("/api/builtin-led-config", handleBuiltinLEDConfig);
  server.on("/api/builtin-led-test", handleBuiltinLEDTest);
  server.on("/api/builtin-led-control", handleBuiltinLEDControl);
  
  // NeoPixel
  server.on("/api/neopixel-config", handleNeoPixelConfig);
  server.on("/api/neopixel-test", handleNeoPixelTest);
  server.on("/api/neopixel-pattern", handleNeoPixelPattern);
  server.on("/api/neopixel-color", handleNeoPixelColor);
  
  // Écrans
  server.on("/api/oled-test", handleOLEDTest);
  server.on("/api/oled-step", handleOLEDStep);
  server.on("/api/oled-message", handleOLEDMessage);
  server.on("/api/oled-config", handleOLEDConfig);
  server.on("/api/oled-boot", handleOLEDBoot);
  
  // TFT display routes
  server.on("/api/tft-test", handleTFTTest);
  server.on("/api/tft-step", handleTFTStep);
  server.on("/api/tft-boot", handleTFTBoot);
  server.on("/api/tft-config", handleTFTConfig);
  server.on("/api/tft-brightness", handleTFTBrightness);  // v3.33.2: PWM brightness control

  // Tests avancés
  server.on("/api/adc-test", handleADCTest);
  server.on("/api/pwm-test", handlePWMTest);
  server.on("/api/spi-scan", handleSPIScan);
  server.on("/api/partitions-list", handlePartitionsList);
  server.on("/api/stress-test", handleStressTest);

  // Routes API pour les nouveaux capteurs
  server.on("/api/rgb-led-config", handleRGBLedConfig);
  server.on("/api/rgb-led-test", handleRGBLedTest);
  server.on("/api/rgb-led-color", handleRGBLedColor);

  server.on("/api/buzzer-config", handleBuzzerConfig);
  server.on("/api/buzzer-test", handleBuzzerTest);
  server.on("/api/buzzer-tone", handleBuzzerTone);

  server.on("/api/dht-config", handleDHTConfig);
  server.on("/api/dht-test", handleDHTTest);

  server.on("/api/light-sensor-config", handleLightSensorConfig);
  server.on("/api/light-sensor-test", handleLightSensorTest);

  server.on("/api/distance-sensor-config", handleDistanceSensorConfig);
  server.on("/api/distance-sensor-test", handleDistanceSensorTest);

  server.on("/api/motion-sensor-config", handleMotionSensorConfig);
  server.on("/api/motion-sensor-test", handleMotionSensorTest);

  // SD Card
  server.on("/api/sd-config", handleSDConfig);
  server.on("/api/sd-test", handleSDTest);
  server.on("/api/sd-info", handleSDInfo);
  server.on("/api/sd-test-read", handleSDTestRead);
  server.on("/api/sd-test-write", handleSDTestWrite);
  server.on("/api/sd-format", handleSDFormat);

  // Rotary Encoder
  server.on("/api/rotary-config", handleRotaryConfig);
  server.on("/api/rotary-test", handleRotaryTest);
  server.on("/api/rotary-position", handleRotaryPosition);
  server.on("/api/rotary-reset", handleRotaryReset);

  // Buttons (v3.28.3)
  server.on("/api/button-states", handleButtonStates);
  server.on("/api/button-state", handleButtonState);  // v3.28.4 - individual button query

  // GPS Module
  server.on("/api/gps", handleGPSData);
  server.on("/api/gps-test", handleGPSTest);

  // Environmental Sensors (AHT20 + BMP280)
  server.on("/api/environmental-sensors", handleEnvironmentalSensors);
  server.on("/api/environmental-test", handleEnvironmentalTest);

  // Performance & Mémoire
  server.on("/api/benchmark", handleBenchmark);
  server.on("/api/memory-details", handleMemoryDetails);
  
  // Exports
  server.on("/export/txt", handleExportTXT);
  server.on("/export/json", handleExportJSON);
  server.on("/export/csv", handleExportCSV);
  server.on("/print", handlePrintVersion);

  server.begin();

  // Install debug routes for troubleshooting
  setupDebugRoutes();

  Serial.println("Serveur Web OK!");
  Serial.println("\r\n===============================================");
  Serial.println("            PRET - En attente");
  Serial.println("   Langue par défaut: FRANCAIS");
  Serial.println("   Changement dynamique via interface web");
  Serial.println("===============================================\r\n");

#if ENABLE_BUTTONS
  initButtons();
  Serial.printf("Boutons actifs: BTN1=%d, BTN2=%d\r\n", button1Pin, button2Pin);
#endif

  // Initialize rotary encoder on startup (v3.28.3 fix)
  Serial.println("Initialisation de l'encodeur rotatif...");
  initRotaryEncoder();
  if (rotaryAvailable) {
    Serial.printf("Encodeur rotatif OK: CLK=%d, DT=%d, SW=%d\r\n",
                  rotary_clk_pin, rotary_dt_pin, rotary_sw_pin);
  } else {
    Serial.println("Encodeur rotatif: non disponible ou configuration invalide");
  }
}

// ========== LOOP ==========
void loop() {
    // --- Earthbeat NeoPixel fade (animation continue) ---
    if (strip != nullptr) {
      static unsigned long neopixelHeartbeatPreviousMillis = 0;
      static float phase = 0.0f;
      static bool fadeUp = true;
      unsigned long now = millis();
      const unsigned long NEOPIXEL_HEARTBEAT_FADE_PERIOD_MS = 1200;
      const uint8_t NEOPIXEL_HEARTBEAT_BRIGHTNESS_MIN = 10;
      const uint8_t NEOPIXEL_HEARTBEAT_BRIGHTNESS_MAX = 60;
      const float step = (float)(now - neopixelHeartbeatPreviousMillis) / (float)NEOPIXEL_HEARTBEAT_FADE_PERIOD_MS;
      neopixelHeartbeatPreviousMillis = now;
      if (fadeUp) {
        phase += step;
        if (phase >= 1.0f) { phase = 1.0f; fadeUp = false; }
      } else {
        phase -= step;
        if (phase <= 0.0f) { phase = 0.0f; fadeUp = true; }
      }
      uint8_t brightness = (uint8_t)(NEOPIXEL_HEARTBEAT_BRIGHTNESS_MIN + (NEOPIXEL_HEARTBEAT_BRIGHTNESS_MAX - NEOPIXEL_HEARTBEAT_BRIGHTNESS_MIN) * phase);
      bool connected = (WiFi.status() == WL_CONNECTED);
      uint32_t color = connected ? strip->Color(0, brightness, 0) : strip->Color(brightness, 0, 0);
      strip->setBrightness(brightness);
      strip->setPixelColor(0, color);
      strip->show();
    }
  server.handleClient();
  maintainNetworkServices();
  updateNeoPixelWifiStatus();

#if ENABLE_BUTTONS
  maintainButtons();
#endif

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 30000) {
    lastUpdate = millis();
    collectDiagnosticInfo();
    
    Serial.println("\r\n=== UPDATE ===");
    Serial.printf("Heap: %.2f KB | Uptime: %.2f h\r\n", 
                  diagnosticData.freeHeap / 1024.0, 
                  diagnosticData.uptime / 3600000.0);
    if (diagnosticData.temperature != -999) {
      Serial.printf("Temp: %.1f°C\r\n", diagnosticData.temperature);
    }
  }

  delay(10);
}
