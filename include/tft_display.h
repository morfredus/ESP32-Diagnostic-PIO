/*
 * TFT_DISPLAY.H - Dynamic TFT Display Management
 * Support for runtime selection between ILI9341 and ST7789 TFT controllers
 * v3.30.0 - Dynamic driver switching without recompilation
 * v3.33.2 - PWM backlight brightness control
 */

#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "config.h"

// Colors (compatible with both drivers)
#define TFT_BLACK       0x0000
#define TFT_WHITE       0xFFFF
#define TFT_RED         0xF800
#define TFT_GREEN       0x07E0
#define TFT_BLUE        0x001F
#define TFT_CYAN        0x07FF
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_ORANGE      0xFD20
#define TFT_DARKGREY    0x7BEF
#define TFT_LIGHTGREY   0xC618

// TFT Driver types
enum TFT_DriverType {
  TFT_DRIVER_ILI9341 = 0,
  TFT_DRIVER_ST7789 = 1
};

#if ENABLE_TFT_DISPLAY

// Global TFT objects - both drivers instantiated
Adafruit_ILI9341* tft_ili9341 = nullptr;
Adafruit_ST7789* tft_st7789 = nullptr;

// Current active driver
TFT_DriverType currentTFTDriver = TFT_DRIVER_ILI9341;  // Default from config.h
Adafruit_GFX* tft = nullptr;  // Generic pointer to active driver

bool tftAvailable = false;

// ============================================================
// TFT BACKLIGHT PWM CONTROL (v3.33.2)
// ============================================================
// PWM channel dedicated to TFT backlight (avoiding conflicts with channel 0 used in tests)
const uint8_t TFT_BACKLIGHT_PWM_CHANNEL = 1;
const uint16_t TFT_BACKLIGHT_PWM_FREQ = 5000;  // 5 kHz
const uint8_t TFT_BACKLIGHT_PWM_RESOLUTION = 8;  // 8-bit (0-255)

// Current brightness level (0-255)
uint8_t tft_current_brightness = TFT_BACKLIGHT_PWM;  // Default from config.h

// Get current driver type as string
const char* getTFTDriverName() {
  return (currentTFTDriver == TFT_DRIVER_ILI9341) ? "ILI9341" : "ST7789";
}

// Get current driver type
TFT_DriverType getTFTDriverType() {
  return currentTFTDriver;
}

// Cleanup and deinitialize current driver
void deinitTFT() {
  if (!tftAvailable) return;

  Serial.print("[TFT] Deinitializing ");
  Serial.print(getTFTDriverName());
  Serial.println(" driver...");

  // Turn off backlight before deinit (PWM off)
  if (TFT_BL >= 0) {
    ledcWrite(TFT_BACKLIGHT_PWM_CHANNEL, 0);
    ledcDetachPin(TFT_BL);
  }

  // Clear the generic pointer
  tft = nullptr;
  tftAvailable = false;

  // Clean up driver objects
  if (tft_ili9341 != nullptr) {
    delete tft_ili9341;
    tft_ili9341 = nullptr;
  }

  if (tft_st7789 != nullptr) {
    delete tft_st7789;
    tft_st7789 = nullptr;
  }

  Serial.println("[TFT] Driver deinitialized");
}

// Initialize TFT display with specified driver
bool initTFT(TFT_DriverType driverType, int width, int height, int rotation) {
  // Deinitialize any existing driver first
  deinitTFT();

  Serial.print("[TFT] Initializing ");
  Serial.print((driverType == TFT_DRIVER_ILI9341) ? "ILI9341" : "ST7789");
  Serial.println(" display...");

  // Configure backlight PWM if enabled
  if (TFT_BL >= 0) {
    // Setup PWM channel for backlight control
    ledcSetup(TFT_BACKLIGHT_PWM_CHANNEL, TFT_BACKLIGHT_PWM_FREQ, TFT_BACKLIGHT_PWM_RESOLUTION);
    ledcAttachPin(TFT_BL, TFT_BACKLIGHT_PWM_CHANNEL);
    ledcWrite(TFT_BACKLIGHT_PWM_CHANNEL, tft_current_brightness);  // Set to default brightness

    Serial.print("[TFT] Backlight PWM initialized (brightness: ");
    Serial.print(tft_current_brightness);
    Serial.println("/255)");
  }

  // Initialize the selected driver
  currentTFTDriver = driverType;

  if (driverType == TFT_DRIVER_ILI9341) {
    // Create and initialize ILI9341
    tft_ili9341 = new Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
    tft = tft_ili9341;

    tft_ili9341->begin();
    tft_ili9341->setRotation(rotation);
    tft_ili9341->fillScreen(TFT_BLACK);

  } else if (driverType == TFT_DRIVER_ST7789) {
    // Create and initialize ST7789
    tft_st7789 = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
    tft = tft_st7789;

    tft_st7789->init(width, height, SPI_MODE0);
    tft_st7789->setRotation(rotation);
    tft_st7789->fillScreen(TFT_BLACK);
  }

  tftAvailable = true;
  Serial.print("[TFT] ");
  Serial.print(getTFTDriverName());
  Serial.println(" display initialized successfully");

  return true;
}

// Overload: Initialize with default driver from config.h
bool initTFT() {
  #if defined(TFT_USE_ILI9341)
    currentTFTDriver = TFT_DRIVER_ILI9341;
  #elif defined(TFT_USE_ST7789)
    currentTFTDriver = TFT_DRIVER_ST7789;
  #else
    currentTFTDriver = TFT_DRIVER_ILI9341;  // Default fallback
  #endif

  return initTFT(currentTFTDriver, TFT_WIDTH, TFT_HEIGHT, TFT_ROTATION);
}

// Switch TFT driver dynamically
bool switchTFTDriver(TFT_DriverType newDriver, int width, int height, int rotation) {
  if (newDriver == currentTFTDriver && tftAvailable) {
    Serial.println("[TFT] Already using requested driver");
    return true;
  }

  Serial.print("[TFT] Switching driver from ");
  Serial.print(getTFTDriverName());
  Serial.print(" to ");
  Serial.println((newDriver == TFT_DRIVER_ILI9341) ? "ILI9341" : "ST7789");

  return initTFT(newDriver, width, height, rotation);
}

// Display boot splash screen
void displayBootSplash() {
  if (!tftAvailable || tft == nullptr) return;

  tft->fillScreen(TFT_BLACK);

  // Title
  tft->setTextColor(TFT_CYAN);
  tft->setTextSize(3);
  tft->setCursor(20, 30);
  tft->println("ESP32");

  tft->setTextSize(2);
  tft->setCursor(15, 65);
  tft->println("Diagnostic");

  // Version
  tft->setTextColor(TFT_WHITE);
  tft->setTextSize(1);
  tft->setCursor(60, 95);
  tft->print("v");
  tft->println(PROJECT_VERSION);

  // Divider line
  tft->drawFastHLine(20, 115, 200, TFT_DARKGREY);

  // System info
  tft->setTextColor(TFT_LIGHTGREY);
  tft->setCursor(20, 130);
  tft->print("Driver: ");
  tft->println(getTFTDriverName());

  delay(500);
}

// Display WiFi connection status
void displayWiFiStatus(const char* status, uint16_t color = TFT_YELLOW) {
  if (!tftAvailable || tft == nullptr) return;

  // Clear status area
  tft->fillRect(20, 150, 200, 20, TFT_BLACK);

  tft->setTextColor(color);
  tft->setTextSize(1);
  tft->setCursor(20, 150);
  tft->println(status);
}

// Display WiFi connected with IP
void displayWiFiConnected(const char* ssid, const char* ipAddress) {
  if (!tftAvailable || tft == nullptr) return;

  // Clear previous status
  tft->fillRect(20, 150, 200, 60, TFT_BLACK);

  // WiFi icon (simplified)
  tft->fillCircle(120, 160, 3, TFT_GREEN);
  tft->drawCircle(120, 160, 8, TFT_GREEN);
  tft->drawCircle(120, 160, 13, TFT_GREEN);

  // SSID
  tft->setTextColor(TFT_GREEN);
  tft->setTextSize(1);
  tft->setCursor(20, 180);
  tft->print("WiFi: ");
  tft->println(ssid);

  // IP Address
  tft->setTextColor(TFT_CYAN);
  tft->setCursor(20, 195);
  tft->print("IP: ");
  tft->println(ipAddress);

  // Ready message
  tft->setTextColor(TFT_WHITE);
  tft->setCursor(40, 215);
  tft->println("System Ready!");
}

// Display WiFi connection failed
void displayWiFiFailed() {
  if (!tftAvailable || tft == nullptr) return;

  tft->fillRect(20, 150, 200, 60, TFT_BLACK);

  tft->setTextColor(TFT_RED);
  tft->setTextSize(1);
  tft->setCursor(20, 170);
  tft->println("WiFi: Failed");
  tft->setCursor(20, 185);
  tft->println("Check configuration");
}

// Update display with runtime info
void updateTFTDisplay(const char* chipModel, const char* ipAddr, unsigned long uptime) {
  if (!tftAvailable || tft == nullptr) return;

  tft->fillScreen(TFT_BLACK);

  // Header
  tft->setTextColor(TFT_CYAN);
  tft->setTextSize(2);
  tft->setCursor(30, 10);
  tft->println("ESP32 Diag");

  // Chip model
  tft->setTextColor(TFT_WHITE);
  tft->setTextSize(1);
  tft->setCursor(20, 40);
  tft->print("Chip: ");
  tft->println(chipModel);

  // IP Address
  tft->setCursor(20, 60);
  tft->print("IP: ");
  tft->println(ipAddr);

  // Uptime
  tft->setCursor(20, 80);
  tft->print("Uptime: ");
  unsigned long seconds = uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  tft->print(days);
  tft->print("d ");
  tft->print(hours % 24);
  tft->print("h ");
  tft->print(minutes % 60);
  tft->println("m");

  // Status indicator
  tft->fillCircle(120, 120, 10, TFT_GREEN);
  tft->setTextColor(TFT_GREEN);
  tft->setCursor(70, 140);
  tft->println("ONLINE");
}

// ============================================================
// BACKLIGHT CONTROL FUNCTIONS (v3.33.2)
// ============================================================

// Set TFT backlight brightness using PWM (0-255)
void setTFTBrightness(uint8_t brightness) {
  if (TFT_BL >= 0) {
    tft_current_brightness = brightness;
    ledcWrite(TFT_BACKLIGHT_PWM_CHANNEL, brightness);

    Serial.print("[TFT] Brightness set to: ");
    Serial.print(brightness);
    Serial.println("/255");
  }
}

// Get current brightness level
uint8_t getTFTBrightness() {
  return tft_current_brightness;
}

// Turn off backlight (set brightness to 0)
void tftBacklightOff() {
  setTFTBrightness(0);
}

// Turn on backlight (restore to previous brightness or default)
void tftBacklightOn() {
  if (tft_current_brightness == 0) {
    setTFTBrightness(TFT_BACKLIGHT_PWM);  // Restore to default
  } else {
    setTFTBrightness(tft_current_brightness);
  }
}

// Clear display
void clearTFT() {
  if (tftAvailable && tft != nullptr) {
    tft->fillScreen(TFT_BLACK);
  }
}

#else
// Stub functions when TFT is disabled
bool initTFT() { return false; }
bool initTFT(TFT_DriverType driverType, int width, int height, int rotation) { return false; }
bool switchTFTDriver(TFT_DriverType newDriver, int width, int height, int rotation) { return false; }
void deinitTFT() {}
const char* getTFTDriverName() { return "None"; }
TFT_DriverType getTFTDriverType() { return TFT_DRIVER_ILI9341; }
void displayBootSplash() {}
void displayWiFiStatus(const char* status, uint16_t color = 0) {}
void displayWiFiConnected(const char* ssid, const char* ipAddress) {}
void displayWiFiFailed() {}
void updateTFTDisplay(const char* chipModel, const char* ipAddr, unsigned long uptime) {}
void setTFTBrightness(uint8_t brightness) {}
uint8_t getTFTBrightness() { return 0; }
void tftBacklightOff() {}
void tftBacklightOn() {}
void clearTFT() {}
#endif // ENABLE_TFT_DISPLAY

#endif // TFT_DISPLAY_H
