/*
 * TFT_DISPLAY.H - TFT ST7789 Display Management
 * Support for 240x240 TFT LCD display with boot splash screen
 */

#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H


#include <Adafruit_GFX.h>
#include <SPI.h>
#include "config.h"


#if defined(TFT_USE_ILI9341)
#include <Adafruit_ILI9341.h>
#elif defined(TFT_USE_ST7789)
#include <Adafruit_ST7789.h>
#else
#error "Vous devez définir TFT_USE_ILI9341 ou TFT_USE_ST7789 dans config.h !"
#endif

// Colors
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


// Global TFT object abstraction

#if ENABLE_TFT_DISPLAY
#if defined(TFT_USE_ILI9341)
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
#elif defined(TFT_USE_ST7789)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
#endif
bool tftAvailable = false;
#endif

// Initialize TFT display

bool initTFT() {
#if ENABLE_TFT_DISPLAY
#if defined(TFT_USE_ILI9341)
  Serial.println("[TFT] Initializing ILI9341 display...");
#elif defined(TFT_USE_ST7789)
  Serial.println("[TFT] Initializing ST7789 display...");
#endif

  // Configure backlight if enabled
  if (TFT_BL >= 0) {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Turn on backlight
  }


  // Initialize display
  #if defined(TFT_USE_ILI9341)
    tft.begin();
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(TFT_BLACK);
  #elif defined(TFT_USE_ST7789)
    tft.init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE0);
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(TFT_BLACK);
  #endif

  tftAvailable = true;
  Serial.println("[TFT] Display initialized successfully");
  return true;
#else
  return false;
#endif
}

// Display boot splash screen
void displayBootSplash() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;

  tft.fillScreen(TFT_BLACK);

  // Title
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(3);
  tft.setCursor(20, 30);
  tft.println("ESP32");

  tft.setTextSize(2);
  tft.setCursor(15, 65);
  tft.println("Diagnostic");

  // Version
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(60, 95);
  tft.print("v");
  tft.println(PROJECT_VERSION);

  // Divider line
  tft.drawFastHLine(20, 115, 200, TFT_DARKGREY);

  // System info
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setCursor(20, 130);
  tft.println("Booting system...");

  delay(500);
#endif
}

// Display WiFi connection status
void displayWiFiStatus(const char* status, uint16_t color = TFT_YELLOW) {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;

  // Clear status area
  tft.fillRect(20, 150, 200, 20, TFT_BLACK);

  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.setCursor(20, 150);
  tft.println(status);
#endif
}

// Display WiFi connected with IP
void displayWiFiConnected(const char* ssid, const char* ipAddress) {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;

  // Clear previous status
  tft.fillRect(20, 150, 200, 60, TFT_BLACK);

  // WiFi icon (simplified)
  tft.fillCircle(120, 160, 3, TFT_GREEN);
  tft.drawCircle(120, 160, 8, TFT_GREEN);
  tft.drawCircle(120, 160, 13, TFT_GREEN);

  // SSID
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(1);
  tft.setCursor(20, 180);
  tft.print("WiFi: ");
  tft.println(ssid);

  // IP Address
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(20, 195);
  tft.print("IP: ");
  tft.println(ipAddress);

  // Ready message
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(40, 215);
  tft.println("System Ready!");
#endif
}

// Display WiFi connection failed
void displayWiFiFailed() {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;

  tft.fillRect(20, 150, 200, 60, TFT_BLACK);

  tft.setTextColor(TFT_RED);
  tft.setTextSize(1);
  tft.setCursor(20, 170);
  tft.println("WiFi: Failed");
  tft.setCursor(20, 185);
  tft.println("Check configuration");
#endif
}

// Update display with runtime info
void updateTFTDisplay(const char* chipModel, const char* ipAddr, unsigned long uptime) {
#if ENABLE_TFT_DISPLAY
  if (!tftAvailable) return;

  tft.fillScreen(TFT_BLACK);

  // Header
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(30, 10);
  tft.println("ESP32 Diag");

  // Chip model
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(20, 40);
  tft.print("Chip: ");
  tft.println(chipModel);

  // IP Address
  tft.setCursor(20, 60);
  tft.print("IP: ");
  tft.println(ipAddr);

  // Uptime
  tft.setCursor(20, 80);
  tft.print("Uptime: ");
  unsigned long seconds = uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  tft.print(days);
  tft.print("d ");
  tft.print(hours % 24);
  tft.print("h ");
  tft.print(minutes % 60);
  tft.println("m");

  // Status indicator
  tft.fillCircle(120, 120, 10, TFT_GREEN);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(70, 140);
  tft.println("ONLINE");
#endif
}

// Turn off backlight
void tftBacklightOff() {
#if ENABLE_TFT_DISPLAY
  if (TFT_BL >= 0) {
    digitalWrite(TFT_BL, LOW);
  }
#endif
}

// Turn on backlight
void tftBacklightOn() {
#if ENABLE_TFT_DISPLAY
  if (TFT_BL >= 0) {
    digitalWrite(TFT_BL, HIGH);
  }
#endif
}

// Clear display
void clearTFT() {
#if ENABLE_TFT_DISPLAY
  if (tftAvailable) {
    tft.fillScreen(TFT_BLACK);
  }
#endif
}

#endif // TFT_DISPLAY_H
