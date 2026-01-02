#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// =========================================================
//         Configuration Pinout ESP32-S3 DevKitC-1 N16R8
// =========================================================
#if defined(TARGET_ESP32_S3)

    #define BOARD_NAME "ESP32-S3 DevKitC-1"

// ============================================================
// RAPPELS DE SÉCURITÉ ESP32-S3
// ============================================================
// - Tension logique GPIO : 3.3 V uniquement (aucune broche n'est 5V tolérante).
// - Ne jamais appliquer 5V directement sur une GPIO (utiliser diviseur ou interface).
// - GPIO0 : broche BOOT (strapping) – ne rien connecter qui la force à LOW au boot.
// - GPIO46 : utilisé pour PIR, attention au JTAG / boot.
// - GPS TX (vers RXD 18) et HC-SR04 ECHO (vers GPIO 35) : Diviseur de tension OBLIGATOIRE si capteurs alimentés en 5V.
// - LEDs : ajouter résistance série 220–470 O (0.25 W) pour limiter le courant.
// - I2C (GPIO 15/16) : pull-up 4.7 kO vers 3.3 V OBLIGATOIRE.
// - Buzzer : transistor nécessaire (avec résistance de base 1–10 kO) si passif ou courant >12 mA.
// ============================================================


// ------------------------------------
// DÉTAIL TECHNIQUE : Diviseur de Tension
// ------------------------------------
// Vout = Vin * (R2 / (R1 + R2))
// Exemple recommandé : R1 = 10 kO (haut), R2 = 20 kO (bas) ? 5V ? ~3.3V
// Variante basse impédance : R1 = 2 kO, R2 = 3 kO
// Utilisation : GPS TX ? ESP32 RX, HC-SR04 ECHO ? ESP32
// ------------------------------------


// ------------------------------------
// GPS (UART1)
// ------------------------------------
#define GPS_RXD 18  // Entrée RX ESP32 (depuis TX GPS) – 3.3V, diviseur obligatoire si GPS 5V
#define GPS_TXD 17  // Sortie TX ESP32 (vers RX GPS) – 3.3V
#define GPS_PPS  8  // Entrée PPS GPS (1PPS) – 3.3V


// ------------------------------------
// TFT ST7789 (SPI) — Ordre SPI standardisé
// ------------------------------------
// Alias :
//   MOSI = DIN / SDA / SDI / DATA / DI
//   MISO = DO / SDO
//   SCLK = SCK / CLK / SCL
//   CS   = CS / CE / SS / TCS
//   DC   = D/C / A0 / RS
//   RST  = RST / RESET / RES
//   BL   = BL / LED / BACKLIGHT
// Tension logique : 3.3V

#define TFT_MISO      13  // MISO TFT/SD (DO / SDO) – entrée 3.3V (ne jamais recevoir 5V)
#define TFT_MOSI      11  // MOSI (DIN / SDA / SDI / DATA / DI) – sortie 3.3V
#define TFT_SCLK      12  // Horloge SPI (SCK / CLK / SCL) – sortie 3.3V
#define TFT_CS        10  // Chip Select TFT – sortie 3.3V
#define TFT_DC         9  // Data/Command – sortie 3.3V
#define TFT_RST       14  // Reset matériel TFT – sortie 3.3V
#define TFT_BL         7  // Rétroéclairage TFT – sortie 3.3V (PWM possible)
#define TOUCH_CS      -1  // Pas de tactile câblé


// ------------------------------------
// ILI9341 (SPI) — Compatible ST7789
// ------------------------------------
#define ILI9341_MISO  TFT_MISO
#define ILI9341_MOSI  TFT_MOSI
#define ILI9341_SCLK  TFT_SCLK
#define ILI9341_CS    TFT_CS
#define ILI9341_DC    TFT_DC
#define ILI9341_RST   TFT_RST
#define ILI9341_BL    TFT_BL


// ------------------------------------
// SD Card (SPI partagé)
// ------------------------------------
#define SD_MISO      13  // MISO SD – entrée 3.3V
#define SD_MOSI      11  // MOSI SD – sortie 3.3V
#define SD_SCLK      12  // Horloge SD – sortie 3.3V
#define SD_CS         1  // Chip Select SD – sortie 3.3V


// ------------------------------------
// I2C
// ------------------------------------
#define I2C_SDA 15  // SDA – open-drain 3.3V, pull-up externe 4.7 kO ? 3.3V
#define I2C_SCL 16  // SCL – open-drain 3.3V, pull-up externe 4.7 kO ? 3.3V


// ------------------------------------
// LED RGB & NeoPixel
// ------------------------------------
#define LED_RED    21 // LED Rouge – sortie 3.3V, résistance série 220–470 O
#define LED_GREEN  41 // LED Verte – sortie 3.3V, résistance série 220–470 O
#define LED_BLUE   42 // LED Bleue – sortie 3.3V, résistance série 220–470 O
#define NEOPIXEL   48 // Data NeoPixel – sortie 3.3V (LED souvent alimentée en 5V)


// ------------------------------------
// Boutons
// ------------------------------------
#define BUTTON_BOOT 0   // Bouton BOOT – entrée 3.3V, vers GND à l’appui
#define BUTTON_1   38   // Bouton utilisateur 1 – entrée 3.3V, pull-up interne
#define BUTTON_2   39   // Bouton utilisateur 2 – entrée 3.3V, pull-up interne


// ------------------------------------
// ENCODEUR ROTATIF (HW-040)
// ------------------------------------
// - Module passif (contacts mécaniques).
// - Recommandation : condensateurs 10 nF entre CLK?GND et DT?GND pour debounce matériel.
// - SW : utiliser pull-up interne.
#define ROTARY_CLK   47  // Signal A / CLK / S1 – entrée 3.3V, ~10 nF recommandé
#define ROTARY_DT    45  // Signal B / DT / S2 – entrée 3.3V, ~10 nF recommandé
#define ROTARY_SW    40  // Bouton SW – entrée 3.3V, pull-up interne


// ------------------------------------
// Capteurs & Sorties diverses
// ------------------------------------
#define PWM            20 // PWM générique – sortie 3.3V (LED via R 220–470 O ou transistor)
#define BUZZER          6 // Commande buzzer – sortie 3.3V, transistor + R base 1–10 kO recommandé
#define DHT             5 // Capteur DHT – DATA 3.3V, pull-up typique 10 kO si nécessaire
#define MOTION_SENSOR  46 // PIR – entrée 3.3V (adapter si module 5V)
#define LIGHT_SENSOR    4 // LDR via diviseur – entrée ADC 3.3V max, R typique 10 kO


// ------------------------------------
// Capteur de Distance HC-SR04
// ------------------------------------
// - TRIG : accepte 3.3V ? OK
// - ECHO : sort du 5V ? diviseur obligatoire
#define DISTANCE_TRIG 2   // TRIG – sortie 3.3V
#define DISTANCE_ECHO 35  // ECHO – entrée 3.3V, diviseur recommandé (10 kO / 20 kO)


// =========================================================
//         MAPPING ESP32 CLASSIQUE (DevKitC)
// =========================================================
#elif defined(TARGET_ESP32_CLASSIC)

    #define BOARD_NAME "ESP32 Classic DevKitC"

// ============================================================
// RAPPELS DE SÉCURITÉ ESP32-WROOM
// ============================================================
// - GPIO 3.3V uniquement.
// - GPIO0 : boot, éviter.
// - GPIO1/3 : UART0 console.
// - GPIO34–39 : entrées uniquement.
// - GPS TX et HC-SR04 ECHO : diviseur obligatoire si 5V.
// - LEDs : résistance série 220–470 O.
// - I2C : pull-up 4.7 kO obligatoire.
// ============================================================


// ------------------------------------
// GPS (UART2)
// ------------------------------------
#define GPS_RXD 16  // RX ESP32 – entrée 3.3V, diviseur si GPS 5V
#define GPS_TXD 17  // TX ESP32 – sortie 3.3V
#define GPS_PPS 36  // PPS GPS – entrée 3.3V (GPIO36 = entrée uniquement)


// ------------------------------------
// TFT ST7789 (SPI)
// ------------------------------------
#define TFT_MISO     -1  // Non câblé sur ce module TFT
#define TFT_MOSI      23 // MOSI – sortie 3.3V
#define TFT_SCLK      18 // Horloge SPI – sortie 3.3V
#define TFT_CS        27 // Chip Select TFT – sortie 3.3V
#define TFT_DC        14 // Data/Command – sortie 3.3V
#define TFT_RST       25 // Reset TFT – sortie 3.3V
#define TFT_BL        32 // Backlight – sortie 3.3V (PWM possible)


// ------------------------------------
// ILI9341 (SPI)
// ------------------------------------
#define ILI9341_MISO  -1
#define ILI9341_MOSI  TFT_MOSI
#define ILI9341_SCLK  TFT_SCLK
#define ILI9341_CS    TFT_CS
#define ILI9341_DC    TFT_DC
#define ILI9341_RST   TFT_RST
#define ILI9341_BL    TFT_BL


// ------------------------------------
// I2C
// ------------------------------------
#define I2C_SDA 21  // SDA – open-drain 3.3V, pull-up 4.7 kO
#define I2C_SCL 22  // SCL – open-drain 3.3V, pull-up 4.7 kO


// ------------------------------------
// SD Card (SPI)
// ------------------------------------
#define SD_MISO      19  // MISO SD – entrée 3.3V
#define SD_MOSI      13  // MOSI SD – sortie 3.3V (partagé avec TFT)
#define SD_SCLK       5  // Horloge SD – sortie 3.3V (partagé avec TFT)
#define SD_CS         4  // Chip Select SD – sortie 3.3V

// ------------------------------------
// LED RGB
// ------------------------------------
#define LED_RED     13 // LED Rouge – sortie 3.3V, résistance 220–470 O
#define LED_GREEN   26 // LED Verte – sortie 3.3V, résistance 220–470 O
#define LED_BLUE    33 // LED Bleue – sortie 3.3V, résistance 220–470 O
#define LED_BUILTIN  2 // LED intégrée – sortie 3.3V


// ------------------------------------
// Boutons
// ------------------------------------
#define BUTTON_BOOT 0  // Bouton BOOT – entrée 3.3V, vers GND
#define BUTTON_1    5  // Bouton user 1 – entrée 3.3V, pull-up interne
#define BUTTON_2   12  // Bouton user 2 – entrée 3.3V, pull-up interne


// ------------------------------------
// ENCODEUR ROTATIF (HW-040)
// ------------------------------------
#define ROTARY_CLK   4   // Signal A / CLK / S1 – entrée 3.3V, ~10 nF recommandé
#define ROTARY_DT    13  // Signal B / DT / S2 – entrée 3.3V, ~10 nF possible
#define ROTARY_SW    26  // Bouton SW – entrée 3.3V, pull-up interne


// ------------------------------------
// Capteurs & Sorties
// ------------------------------------
#define PWM            4  // PWM générique – sortie 3.3V
#define BUZZER        19  // Commande buzzer – sortie 3.3V, transistor + R base 1–10 kO
#define DHT           15  // Capteur DHT – DATA 3.3V, pull-up 10 kO si nécessaire
#define MOTION_SENSOR 34  // PIR – entrée 3.3V (GPIO34 = entrée uniquement, adapter si module 5V)
#define LIGHT_SENSOR  39  // LDR via diviseur – entrée ADC 3.3V max, R typique 10 kO
#define NEOPIXEL       2  // Data NeoPixel – sortie 3.3V (partagé avec LED_BUILTIN)


// ------------------------------------
// Capteur de Distance
// ------------------------------------
#define DISTANCE_TRIG 1  // TRIG – sortie 3.3V
#define DISTANCE_ECHO 35 // ECHO – entrée 3.3V, diviseur obligatoire si capteur 5V


#else
    #error "Aucune cible definie ! Verifiez platformio.ini (TARGET_ESP32_...)"
#endif

#endif // BOARD_CONFIG_H
