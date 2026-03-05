#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// =========================================================
//         CONFIGURATION ESP32-S3 DevKitC-1 (N16R8)
// =========================================================
#if defined(TARGET_ESP32_S3)

#define BOARD_NAME "ESP32-S3 DevKitC-1"

// ============================================================
// Paramètres écran TFT
// ============================================================
#define DISPLAY_WIDTH      240   // Largeur écran pixels.
#define DISPLAY_HEIGHT     240   // Hauteur écran pixels.
#define DISPLAY_SPI_FREQ   40000000UL // 40 MHz : stable et rapide sur S3.

// ============================================================
// RAPPELS DE SÉCURITÉ ESP32-S3
// ============================================================
// - GPIO 3.3V uniquement (aucune broche 5V tolérante).
// - GPIO0 : strapping BOOT — ne rien connecter qui force LOW.
// - GPIO46 : entrée uniquement, attention au boot/JTAG.
// - GPS TX (vers RXD 18) et HC-SR04 ECHO (vers GPIO 35) : diviseur obligatoire si capteurs 5V.
// - I2C : pull-up 4.7 kΩ obligatoire.
// - LED : résistance série 220–470 Ω.
// - Buzzer : transistor obligatoire si >12 mA.
// ============================================================


// ============================================================
// SPI TFT + SD — SPI Natif ESP32-S3 (HSPI)
// ============================================================
#define DISPLAY_SCK_PIN     12   // SPI SCLK — natif, haute vitesse
#define DISPLAY_MOSI_PIN    11   // SPI MOSI — natif
#define DISPLAY_MISO_PIN    13   // SPI MISO — natif (lecture SD)
#define DISPLAY_CS_PIN      10   // CS TFT — GPIO libre
#define DISPLAY_DC_PIN       9   // Data/Command — GPIO libre
#define DISPLAY_RST_PIN      8   // Reset matériel TFT
#define DISPLAY_BL_PIN      18   // Backlight — PWM possible

// Alias TFT (compatibilité librairies)
#define TFT_MISO_PIN DISPLAY_MISO_PIN
#define TFT_MOSI_PIN DISPLAY_MOSI_PIN
#define TFT_SCLK_PIN DISPLAY_SCK_PIN
#define TFT_CS_PIN   DISPLAY_CS_PIN
#define TFT_DC_PIN   DISPLAY_DC_PIN
#define TFT_RST_PIN  DISPLAY_RST_PIN
#define TFT_BL_PIN   DISPLAY_BL_PIN

// SD Card (SPI partagé)
#define SD_MISO_PIN   DISPLAY_MISO_PIN
#define SD_MOSI_PIN   DISPLAY_MOSI_PIN
#define SD_SCLK_PIN   DISPLAY_SCK_PIN
#define SD_CS_PIN     1   // CS SD — GPIO sûr


// ============================================================
// GPS (UART1)
// ============================================================
#define GPS_RXD_PIN 18  // RX ESP32 — entrée 3.3V (diviseur si GPS 5V)
#define GPS_TXD_PIN 17  // TX ESP32 — sortie 3.3V
#define GPS_PPS_PIN  7  // PPS GPS — entrée 3.3V


// ============================================================
// I2C — Broches recommandées ESP32-S3
// ============================================================
#define I2C_SDA_PIN  15  // SDA — open-drain, pull-up 4.7 kΩ
#define I2C_SCL_PIN  16  // SCL — open-drain, pull-up 4.7 kΩ


// ============================================================
// LED RGB & NeoPixel
// ============================================================
#define LED_RED_PIN    21  // LED Rouge — sortie 3.3V
#define LED_GREEN_PIN  41  // LED Verte — sortie 3.3V
#define LED_BLUE_PIN   42  // LED Bleue — sortie 3.3V
#define NEOPIXEL_PIN   48  // NeoPixel — sortie 3.3V


// ============================================================
// Boutons
// ============================================================
#define BUTTON_BOOT_PIN 0   // BOOT — strapping
#define BUTTON_1_PIN   38   // Bouton user — entrée, pull-up interne
#define BUTTON_2_PIN   39   // Bouton user — entrée, pull-up interne


// ============================================================
// Encodeur Rotatif (HW-040)
// ============================================================
#define ROTARY_CLK_PIN  47  // Signal A — entrée rapide
#define ROTARY_DT_PIN   45  // Signal B — entrée rapide
#define ROTARY_SW_PIN   40  // Bouton SW — entrée, pull-up interne


// ============================================================
// Capteurs & Sorties
// ============================================================
#define PWM_PIN            20 // PWM générique — sortie
#define BUZZER_PIN          6 // Buzzer — sortie (transistor recommandé)
#define DHT_PIN             5 // DHT — DATA 3.3V
#define MOTION_SENSOR_PIN  46 // PIR — entrée uniquement
#define LIGHT_SENSOR_PIN    4 // LDR via diviseur — ADC 3.3V max


// ============================================================
// HC-SR04
// ============================================================
#define DISTANCE_TRIG_PIN 2   // TRIG — sortie 3.3V
#define DISTANCE_ECHO_PIN 35  // ECHO — entrée, diviseur obligatoire



// =========================================================
//         CONFIGURATION ESP32 CLASSIC (WROOM DevKitC)
// =========================================================
#elif defined(TARGET_ESP32_CLASSIC)

#define BOARD_NAME "ESP32 Classic DevKitC"

// ============================================================
// Paramètres écran TFT
// ============================================================
#define DISPLAY_WIDTH      240   // Largeur écran pixels.
#define DISPLAY_HEIGHT     240   // Hauteur écran pixels.
#define DISPLAY_SPI_FREQ   40000000UL // 40 MHz : stable et rapide sur S3/Classic.

// ============================================================
// RAPPELS DE SÉCURITÉ ESP32-WROOM
// ============================================================
// - GPIO 3.3V uniquement.
// - GPIO0 : boot — éviter.
// - GPIO1/3 : UART0 console — ne pas utiliser.
// - GPIO34–39 : entrées uniquement.
// - GPS TX et HC-SR04 ECHO : diviseur obligatoire si 5V.
// ============================================================


// ============================================================
// SPI TFT + SD — SPI Natif VSPI
// ============================================================
#define DISPLAY_SCK_PIN     18   // VSPI SCLK — natif
#define DISPLAY_MOSI_PIN    23   // VSPI MOSI — natif
#define DISPLAY_MISO_PIN    19   // VSPI MISO — natif
#define DISPLAY_CS_PIN       5   // CS TFT — GPIO sûr
#define DISPLAY_DC_PIN      16   // Data/Command — GPIO sûr
#define DISPLAY_RST_PIN     17   // Reset matériel — GPIO sûr
#define DISPLAY_BL_PIN       4   // Backlight — PWM possible

// Alias TFT
#define TFT_MISO_PIN DISPLAY_MISO_PIN
#define TFT_MOSI_PIN DISPLAY_MOSI_PIN
#define TFT_SCLK_PIN DISPLAY_SCK_PIN
#define TFT_CS_PIN   DISPLAY_CS_PIN
#define TFT_DC_PIN   DISPLAY_DC_PIN
#define TFT_RST_PIN  DISPLAY_RST_PIN
#define TFT_BL_PIN   DISPLAY_BL_PIN

// SD Card (SPI partagé)
#define SD_MISO_PIN   DISPLAY_MISO_PIN
#define SD_MOSI_PIN   DISPLAY_MOSI_PIN
#define SD_SCLK_PIN   DISPLAY_SCK_PIN
#define SD_CS_PIN     DISPLAY_CS_PIN


// ============================================================
// GPS (UART2)
// ============================================================
#define GPS_RXD_PIN 32  // RX ESP32 — entrée 3.3V
#define GPS_TXD_PIN 33  // TX ESP32 — sortie 3.3V
#define GPS_PPS_PIN 36  // PPS — entrée uniquement


// ============================================================
// I2C
// ============================================================
#define I2C_SDA_PIN 21  // SDA — open-drain, pull-up 4.7 kΩ
#define I2C_SCL_PIN 22  // SCL — open-drain, pull-up 4.7 kΩ


// ============================================================
// LED RGB & NeoPixel
// ============================================================
#define LED_RED_PIN     13 // LED Rouge — sortie
#define LED_GREEN_PIN   26 // LED Verte — sortie
#define LED_BLUE_PIN    25 // LED Bleue — sortie
#define NEOPIXEL_PIN     2 // NeoPixel — sortie
#define LED_BUILTIN NEOPIXEL_PIN // LED intégrée (si présente) — sortie

// ============================================================
// Boutons
// ============================================================
#define BUTTON_BOOT_PIN 0  // BOOT — strapping
#define BUTTON_1_PIN    27 // Bouton user — entrée
#define BUTTON_2_PIN    12 // Bouton user — entrée


// ============================================================
// Encodeur Rotatif
// ============================================================
#define ROTARY_CLK_PIN  14  // Signal A — entrée
#define ROTARY_DT_PIN   27  // Signal B — entrée
#define ROTARY_SW_PIN   26  // Bouton SW — entrée


// ============================================================
// Capteurs & Sorties
// ============================================================
#define PWM_PIN            25 // PWM générique — sortie
#define BUZZER_PIN         13 // Buzzer — sortie
#define DHT_PIN            15 // DHT — DATA
#define MOTION_SENSOR_PIN  34 // PIR — entrée uniquement
#define LIGHT_SENSOR_PIN   39 // LDR — ADC entrée uniquement


// ============================================================
// HC-SR04
// ============================================================
#define DISTANCE_TRIG_PIN 1  // TRIG — sortie
#define DISTANCE_ECHO_PIN 35 // ECHO — entrée, diviseur obligatoire


#else
#error "Aucune cible définie ! Vérifiez platformio.ini (TARGET_ESP32_...)"
#endif

#endif // BOARD_CONFIG_H
