# ESP32 Diagnostic Suite – Matrice des fonctionnalités (v3.28.5)
# ESP32 Diagnostic Suite – Matrice des fonctionnalités (v3.29.0)

> NOUVEAUTÉ v3.29.0 : Sélection dynamique du contrôleur TFT (ILI9341 ou ST7789) et configuration de la résolution dans `config.h`.
| Test TFT | ✅ | ✅ | ✅ | ESP32 classique. 4Mo Flash, sans PSRAM. Pas de restriction GPIO 35-48. |
| Test TFT | ✅ | ✅ | ✅ | ESP32 classique. 4Mo Flash, sans PSRAM. Pas de restriction GPIO 35-48. Prend en charge ST7789 (SPI, 240x240/240x320) et ILI9341 (SPI, 240x320) – configurable dans `config.h`. |
> **AVERTISSEMENT** : Ce document reflète le firmware v3.28.5 avec mappings de broches EXACTS depuis `include/board_config.h`. Toutes les assignations GPIO ont été vérifiées et synchronisées avec le code. La documentation précédente (v3.22.1) contenait des erreurs CRITIQUES. Lisez [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) avant de flasher.

Cette matrice résume la couverture des diagnostics pour les cartes Espressif supportées par la version 3.28.5 et ses trois environnements de build.
Elle aide à planifier les campagnes de validation et à vérifier si des périphériques optionnels nécessitent un câblage supplémentaire.

> **Important :** La version 3.28.5 (PlatformIO) n'active pas le Bluetooth/BLE. Pour les diagnostics BLE, utilisez la version Arduino IDE archivée [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE).

**✨ Nouveau en v3.28.5 :** Corrections monitoring dispositifs d'entrée - état bouton encodeur lit GPIO réel, monitoring boutons utilise accès direct aux constantes.

**✨ Nouveau en v3.28.4 :** Corrections endpoint API monitoring boutons - ajout `/api/button-state` (singulier) pour requêtes boutons individuels.

**✨ Nouveau en v3.28.3 :** Auto-initialisation encodeur rotatif dans setup(), endpoints API backend monitoring boutons.

**✨ Nouveau en v3.28.x :** Support encodeur rotatif (HW-040) avec monitoring temps réel. Endpoints API test carte SD. Configuration broche TFT MISO via interface web. Avertissement partage GPIO 13 (TFT/SD MISO).

**✨ Nouveau en v3.27.x :** Monitoring boutons matériels (BUTTON_BOOT, BUTTON_1, BUTTON_2) avec mises à jour état temps réel.

**✨ Nouveau en v3.22.0 :** MQTT Publisher pour publication en temps réel des métriques système (mémoire, WiFi, capteurs) vers Home Assistant, NodeRED, InfluxDB et autres brokers MQTT. Non-bloquant avec auto-reconnexion. **Désactivé par défaut.**

**⚠️ Mis à jour en v3.28.2 :** Documentation entièrement synchronisée avec `board_config.h`. Toutes erreurs GPIO corrigées (11+ corrections critiques depuis v3.22.1).

## Légende
- ✅ – Pris en charge nativement par le firmware.
- ⚙️ – Pris en charge avec configuration supplémentaire ou adaptation matérielle (voir colonne Notes).
- ⛔ – Non disponible pour la carte cible ou nécessite une extension firmware spécifique.

## Capacités des cartes principales
| Carte | Environnement | Scan Wi-Fi | Scan BLE | Sonde PSRAM | Intégrité flash | Test NeoPixel | Test OLED | Test TFT | Notes |
|-------|---------------|-----------|----------|-------------|-----------------|---------------|-----------|----------|-------|
| ESP32-S3 N16R8 | `esp32s3_n16r8` | ✅ | ⛔ | ✅ | ✅ | ✅ | ✅ | ✅ | Cible principale. 16Mo Flash, 8Mo PSRAM. Mémoire OPI (GPIO 35-48 réservés). |
| ESP32-S3 N8R8 | `esp32s3_n8r8` | ✅ | ⛔ | ✅ | ✅ | ✅ | ✅ | ✅ | 8Mo Flash, 8Mo PSRAM. Mêmes restrictions broches que N16R8. |
| ESP32 DevKitC | `esp32devkitc` | ✅ | ⛔ | ⛔ | ✅ | ✅ | ✅ | ✅ | ESP32 classique. 4Mo Flash, sans PSRAM. Pas de restriction GPIO 35-48. |

## Couverture des bus et périphériques
| Bus / Périphérique | Broches par cible | Cartes supportées | Notes |
|--------------------|--------------------|-------------------|-------|
| I2C principal | **ESP32-S3:** SDA=15, SCL=16 · **ESP32 Classic:** SDA=21, SCL=22 | Toutes les cartes supportées | Utilisé pour l'OLED, les packs capteurs, l'EEPROM (laisser GPIO19/20 libres pour l'USB). |
| I2C secondaire | Désactivé par défaut | ESP32, ESP32-S3 | Activer via flag `ENABLE_SECONDARY_I2C` dans `config.h`. |
| Bus SPI test | **Varie selon cible** (voir PIN_MAPPING_FR.md) | ESP32, ESP32-S3 | Valide adaptateurs flash/SD externes. |
| Boucle UART | TX0/RX0 & UART1/UART2 optionnel | Toutes | Nécessite un cavalier TX↔RX sur l'UART choisi. |
| Écran TFT ST7789 | **ESP32-S3:** MISO=13 ⚠️ (partagé avec SD), MOSI=11, SCLK=12, CS=10, DC=9, RST=14 ⚠️ (était 13 – FAUX !), BL=7 · **ESP32 Classic:** MISO=19, MOSI=23, SCLK=18, CS=27, DC=14, RST=25, BL=32 | Toutes les cartes supportées | Affichage 240x240. GPIO 13 partagé avec carte SD sur ESP32-S3. |
| Carte SD (SPI) | **ESP32-S3:** MISO=13 ⚠️ (partagé avec TFT), MOSI=11, SCLK=12, CS=1 · **ESP32 Classic:** Varie selon configuration | ESP32-S3 (complet), ESP32 Classic (varie) | Ajouté v3.28.0. Partage GPIO 13 nécessite gestion CS appropriée. |
| LED RGB (séparée) | **ESP32-S3:** R=21, G=41 ⚠️ (était 45 – FAUX !), B=42 ⚠️ (était 47 – FAUX !) · **ESP32 Classic:** R=13, V=26, B=33 | Toutes | Broches PWM pour LED RGB. S3 : GPIO 41/42 PAS 45/47 ! |
| **Indicateur Wi-Fi NeoPixel** | **ESP32-S3:** GPIO 48 · **ESP32 Classic:** -1 (désactivé) | ESP32-S3 uniquement | LED RGB unique affichant l'état Wi-Fi : jaune (connexion), vert pulsant (connecté), rouge pulsant (déconnecté), flash violet (redémarrage). |
| Boutons | **ESP32-S3:** BOOT=0 (lecture seule), BTN1=38, BTN2=39 · **ESP32 Classic:** BOOT=0 (lecture seule), BTN1=5 ⚠️ (était 32 en v3.22.1 – FAUX !), BTN2=12 ⚠️ (était 33 – FAUX !) | Toutes les cartes supportées | BUTTON_BOOT non configurable. S3 : pull-ups internes. Classic : GPIO 5/12 avec pull-ups internes. |
| Encodeur rotatif (HW-040) | **ESP32-S3:** CLK=47, DT=45, SW=40 · **ESP32 Classic:** CLK=4, DT=13, SW=26 | Toutes les cartes supportées | Ajouté v3.27.x. Monitoring position et bouton temps réel via interface web. |

## Modules optionnels
| Module | Flag firmware | État par défaut | Description |
|--------|---------------|-----------------|-------------|
| Diagnostics OLED | `ENABLE_OLED_TESTS` | Activé si câblage OLED détecté | Lance balayage de pixels, contraste et récupération I2C. |
| Animateur NeoPixel | `ENABLE_NEOPIXEL_TESTS` | Activé | Exécute roue chromatique et stress test à 800 kHz. |
| **Battement Wi-Fi** | Aucun (toujours activé) | Activé | État Wi-Fi en temps réel sur LED NeoPixel. |
| Reporter MQTT | `ENABLE_MQTT_BRIDGE` | Désactivé | Publie des résumés ; nécessite les identifiants broker dans `config.h`. |
| Automatisation REST | `ENABLE_REST_API` | Activé | Fournit des endpoints JSON pour CI/CD ou outils flotte. |
| Moniteur d'énergie | `ENABLE_POWER_MONITOR` | Désactivé | Lit les capteurs INA219/INA3221 via I2C. |

## Hypothèses environnementales
- Arduino Core ESP32 **3.3.3** (PlatformIO `espressif32@6.6.x` ou Boards Manager 3.1.x+).
- Alimentation USB délivrant au moins 500 mA durant les stress tests OLED/NeoPixel.
- Réseau Wi-Fi stable (2,4 GHz) pour les mesures de débit ; les portails captifs ne sont pas gérés.

## Documents liés
- [Guide de configuration](CONFIG_FR.md) – activation ou désactivation des modules optionnels.
- [Modules de diagnostic détaillés](DIAGNOSTIC_MODULES_FR.md) – ordre d'exécution et métriques collectées.
- [Dépannage](TROUBLESHOOTING_FR.md) – étapes à suivre lorsqu'un module est signalé indisponible.
