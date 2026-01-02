# Mapping des pins — Référence rapide (v3.33.1)

> NOUVEAUTÉ v3.33.1 : Ajout des définitions de broches MOTION_SENSOR et NEOPIXEL manquantes pour ESP32 Classic (correction de compilation).

> NOUVEAUTÉ v3.33.0 : Barre de progression TFT fluide, NeoPixel violet, logique BOOT robuste, et sélection dynamique du contrôleur TFT (ILI9341 ou ST7789). Voir FEATURE_MATRIX_FR.md et RELEASE_NOTES_3.33.0_FR.md.

> **AVERTISSEMENT** : Ce document reflète le mapping de pins EXACT depuis `include/board_config.h` version v3.33.1. Toutes les assignations GPIO ont été vérifiées et synchronisées avec le code. Un câblage incorrect peut endommager votre ESP32 ou vos périphériques. Lisez ce document entièrement avant de flasher.

- **Cartes supportées :**
  - ESP32-S3-DevKitC-1 N16R8 (16Mo Flash, 8Mo PSRAM) — `esp32s3_n16r8`
  - ESP32-S3-DevKitC-1 N8R8 (8Mo Flash, 8Mo PSRAM) — `esp32s3_n8r8`
  - ESP32-DevKitC (4Mo Flash, sans PSRAM) — `esp32devkitc`
- Pin mapping défini dans `include/board_config.h` via `TARGET_ESP32_S3` ou `TARGET_ESP32_CLASSIC`.

## ESP32-S3 (N16R8 / N8R8)
- **I2C :** SDA=15, SCL=16
- **Boutons :** BUTTON_BOOT=0 (lecture seule), BUTTON_1=38, BUTTON_2=39
- **Encodeur rotatif (HW-040) :** CLK=47, DT=45, SW=40
- **LED RGB :** R=21, G=41 ⚠️ (était 45 dans docs v3.22.1 — FAUX !), B=42 ⚠️ (était 47 — FAUX !)
- **Capteurs :**
  - DHT=5
  - Lumière=4
  - HC-SR04 : TRIG=2 ⚠️ (était 3 — FAUX !), ECHO=35 ⚠️ (était 6 — FAUX !)
  - Mouvement (PIR)=46
  - Buzzer=6 ⚠️ (était 14 dans docs v3.22.1 — FAUX !)
  - PWM=20 ⚠️ (était 14 dans docs v3.22.1 — FAUX !)
- **TFT ST7789 :** MISO=13 ⚠️ (partagé avec SD), MOSI=11, SCLK=12, CS=10, DC=9, RST=14 ⚠️ (était 13 — FAUX !), BL=7
- **Carte SD (SPI) :** MISO=13 ⚠️ (partagé avec TFT), MOSI=11, SCLK=12, CS=1
- **GPS (UART1) :** RXD=18, TXD=17, PPS=8
- **NeoPixel :** pin 48 (activé), 1 LED, luminosité 50
- **Notes importantes :**
  - GPIO 35–44 réservés (PSRAM/Flash) — NE PAS UTILISER
  - GPIO 48 dédié au NeoPixel
  - LED RGB utilise GPIO 21/41/42 ⚠️ **PAS 21/45/47** (erreur documentation courante !)
  - Encodeur rotatif utilise GPIO 47/45/40 (ajouté en v3.27.x)
  - GPIO 13 partagé : MISO TFT et SD utilisent tous deux GPIO 13 — gestion CS appropriée requise
  - Broches de strapping : GPIO 0, 45, 46 — garder stables au boot
  - USB CDC activé : GPIO 19/20 réservées pour USB D-/D+ (I2C déplacé en 15/16)
  - Bouton de boot : GPIO 0 (BUTTON_BOOT) en lecture seule, non configurable

## ESP32 Classic (DevKitC)
- **I2C :** SDA=21, SCL=22
- **Boutons :** BUTTON_BOOT=0 (lecture seule), BUTTON_1=5 ⚠️ (était 2 dans docs v3.22.1 — FAUX !), BUTTON_2=12 ⚠️ (était 5 — FAUX !)
- **Encodeur rotatif (HW-040) :** CLK=4, DT=13, SW=26
- **LED RGB :** R=13, V=26, B=33
- **Capteurs :**
  - DHT=15
  - Mouvement (PIR)=34 (entrée seule, ajouté en v3.33.1)
  - Lumière=39
  - HC-SR04 : TRIG=1 ⚠️ (était 12 dans docs v3.22.1 — FAUX !), ECHO=35
  - PWM=4
  - Buzzer=19
- **TFT ST7789 :** MISO=19 (si nécessaire), MOSI=23, SCLK=18, CS=27, DC=14, RST=25, BL=32
- **Carte SD (SPI) :** MISO=19, MOSI=13 (partagé avec TFT), SCLK=5 (partagé avec TFT), CS=4
- **GPS (UART2) :** RXD=16, TXD=17, PPS=36
- **NeoPixel :** pin 2 (partagé avec LED_BUILTIN, ajouté en v3.33.1), nombre 1, luminosité 50
- **Notes importantes :**
  - Broches entrée seule : GPIO 34/35/36/39 — lecture uniquement, pas de sortie
  - Broches de strapping : GPIO 0/2/4/5/12/15 — garder stables au boot
    - BUTTON_1 (GPIO 5) et BUTTON_2 (GPIO 12) utilisent pull-ups internes pour sécurité
    - ⚠️ Documentation précédente listait boutons GPIO 2/5 ou 32/33 — TOUS FAUX !
  - Encodeur rotatif : GPIO 4/13/26 (ajouté en v3.27.x)
  - Conflit UART0 : DISTANCE_TRIG utilise GPIO 1 (TX0) — gestion boot/flash prudente requise
  - GPIO 6-11 réservées au flash SPI — NE PAS UTILISER
  - Limitation ADC2 : ADC2 (GPIO 0, 2, 4, 12–15, 25–27) inutilisable quand Wi-Fi actif
  - Limite courant : Max 12 mA par GPIO ; utiliser transistor/driver pour charges élevées
  - Bouton de boot : GPIO 0 (BUTTON_BOOT) en lecture seule, non configurable

## Conflits à éviter et avertissements critiques

### ESP32-S3
- **Bus PSRAM/Flash :** Garder GPIO 35–44 complètement libres (réservés par PSRAM/Flash octal)
- **LED RGB :** Utilise GPIO 21/41/42 (Rouge/Vert/Bleu) ⚠️ **PAS 21/45/47** (erreur docs courante !)
  - NeoPixel utilise GPIO 48 (conflit résolu en v3.18.3)
- **Encodeur rotatif :** Utilise GPIO 47/45/40 (conflits évités par sélection pins soigneuse)
- **Partage GPIO 13 :** TFT MISO et SD MISO utilisent tous deux GPIO 13 — gestion CS appropriée requise
- **Capteur distance :** TRIG GPIO 2 / ECHO GPIO 35 ⚠️ (docs v3.22.1 listaient TRIG=3, ECHO=6 — FAUX !)
- **Buzzer/PWM :** Buzzer GPIO 6, PWM GPIO 20 ⚠️ (docs v3.22.1 listaient tous deux GPIO 14 — FAUX !)
- **Écran TFT :** Occupe GPIO 7–14 (RST=14, pas 13) ; GPS utilise GPIO 17–18
- **Capteur mouvement :** GPIO 46 ; Capteur lumière GPIO 4

### ESP32 Classic
- **Broches strapping :** GPIO 0/2/4/5/12/15 doivent être stables au boot
  - BUTTON_1 (GPIO 5) et BUTTON_2 (GPIO 12) utilisent pull-ups internes pour sécurité
  - ⚠️ Docs précédentes listaient boutons GPIO 2/5 ou GPIO 32/33 — TOUS FAUX !
- **Encodeur rotatif :** Utilise GPIO 4/13/26 (CLK/DT/SW) — broche strapping GPIO 4 gérée soigneusement
- **Conflit UART0 :** DISTANCE_TRIG utilise GPIO 1 (TX0) — gestion boot/flash prudente requise
  - Protéger GPIO 1 (TX0) et GPIO 3 (RX0) pendant flashage
- **Écran TFT :** Utilise GPIO 18/23/27/14/25/32 ; gérer CS lors du partage bus SPI
- **ADC2 + WiFi :** Broches ADC2 (GPIO 0, 2, 4, 12–15, 25–27) inutilisables quand WiFi actif
  - Préférer ADC1 (GPIO 32–39) pour détection analogique
- **Entrée seule :** GPIO 34–39 ne peuvent pas piloter sorties ; utilisées pour GPS PPS (36) et capteur lumière (39)

## Comment changer les pins
- **Statique (compilation) :** Éditez `include/board_config.h`, reconstruisez avec `pio run -e <environment>`
- **Dynamique (runtime) :** Via l'interface web (section Capteurs) ou console série ; persiste jusqu'au redémarrage

## Sélection environnement de build

### Option 1 : platformio.ini
Éditez la ligne `default_envs` :
```ini
[platformio]
default_envs = esp32s3_n16r8  ; Changez en esp32s3_n8r8 ou esp32devkitc
```

### Option 2 : Ligne de commande
```bash
# Build environnement spécifique
pio run -e esp32s3_n16r8
pio run -e esp32s3_n8r8
pio run -e esp32devkitc

# Upload vers environnement spécifique
pio run -e esp32devkitc --target upload
```

## Conseils de câblage matériel

### Capteur ultrasonique HC-SR04
- **Alimentation :** 5V, GND
- **TRIG :** Connexion directe (sortie ESP32 3.3V suffisante)
- **ECHO :** Utiliser diviseur de tension (5V → 3.3V) ou level shifter
  - Diviseur résistif : 1kΩ (ECHO vers ESP) + 2kΩ (ESP vers GND)

### Écran TFT ST7789
- **Alimentation :** 3.3V (vérifiez spécifications module)
- **Rétroéclairage :** Connecter broche BL via résistance 100Ω
- **Vitesse SPI :** Typique 40-80 MHz

### Capteur DHT (DHT11/DHT22)
- **Alimentation :** 3.3V ou 5V
- **Pull-up :** Résistance 4.7k-10kΩ de DATA vers VCC
- **Condensateur :** Condensateur découplage 100nF recommandé

### LEDs NeoPixel / WS2812B
- **Alimentation :** 5V pour luminosité complète
- **Données :** Compatible logique 3.3V, mais 5V préféré pour fiabilité
- **Courant :** ~60mA par LED en blanc complet

## Historique des versions

- **v3.33.1 (2026-01-02) :** ✅ **Correction de Compilation pour ESP32 Classic**
  - Ajout des définitions manquantes MOTION_SENSOR (GPIO 34) et NEOPIXEL (GPIO 2) pour ESP32 Classic
  - Correction des erreurs de compilation dans l'environnement `esp32devkitc`
  - MOTION_SENSOR utilise GPIO 34 (entrée seule, idéal pour capteur PIR)
  - NEOPIXEL utilise GPIO 2 (partagé avec LED_BUILTIN pour simplicité)
  - Mapping de pins ESP32-S3 inchangé

- **v3.28.5 (2025-12-24) :** ✅ **Corrections du monitoring des dispositifs d'entrée**
  - Correction bouton encodeur bloqué sur "Pressed" - lecture d'état GPIO réel
  - Correction monitoring boutons (BOOT/Bouton1/Bouton2) - accès direct aux constantes
  - Aucun changement de pin mapping depuis v3.28.2

- **v3.28.4 (2025-12-24) :** Corrections endpoint API monitoring boutons
  - Ajout `/api/button-state` (singulier) pour requêtes boutons individuels
  - Aucun changement de pin mapping

- **v3.28.3 (2025-12-24) :** Initialisation encodeur rotatif et API monitoring boutons
  - L'encodeur rotatif s'initialise maintenant dans setup()
  - Ajout endpoint `/api/button-states`
  - Aucun changement de pin mapping

- **v3.28.2 (2025-12-24) :** ✅ **Documentation entièrement synchronisée avec `board_config.h`**
  - **CORRECTIONS CRITIQUES :** Corrigé 11+ erreurs d'assignation GPIO depuis documentation v3.22.1 :
    - ESP32-S3 : RGB Vert 45→41, Bleu 47→42, Distance TRIG 3→2, ECHO 6→35, Buzzer 14→6, PWM 14→20, TFT RST 13→14
    - ESP32 Classic : Bouton1 2→5, Bouton2 5→12, Distance TRIG 12→1
  - **AJOUTÉ :** Documentation encodeur rotatif (ESP32-S3: 47/45/40, ESP32 Classic: 4/13/26)
  - **AJOUTÉ :** Mapping broches carte SD (ESP32-S3: MISO=13 partagé avec TFT, CS=1)
  - **AJOUTÉ :** BUTTON_BOOT (GPIO 0) pour toutes cartes – lecture seule, non configurable
  - ⚠️ **AVERTISSEMENT :** Documentation précédente (v3.22.1) contenait erreurs DANGEREUSES pouvant endommager matériel

- **v3.27.x (2025-12-23) :** Fonctionnalités encodeur rotatif (HW-040) et monitoring boutons ajoutées au firmware

- **v3.21.0 :** Révision complète mapping pins ESP32 Classic — 11 modifications pour résoudre problèmes boot (broches strapping GPIO 4/12/15), communication USB-UART (protection GPIO 1/3), et boutons (GPIO 32/33→5/12 avec pull-up interne). Migration matérielle requise pour utilisateurs ESP32 Classic

- **v3.20.2 :** Références pins interface web maintenant dynamiquement sourcées depuis `board_config.h`

- **v3.20.1 :** I2C ESP32-S3 déplacé vers 15/16 et LED RGB Rouge vers 21 pour libérer USB D-/D+ (19/20)

- **v3.18.3 :** Résolution conflit GPIO 48 entre NeoPixel et LED RGB sur ESP32-S3

- **v3.17.1 :** Mapping pins ESP32-S3 et Classic actualisé (GPS, TFT, LED RGB, capteurs, boutons)

- **v3.15.0 :** Support multi-environnements avec mappings pins dédiés
