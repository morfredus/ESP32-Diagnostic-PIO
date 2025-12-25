
# ESP32 Diagnostic Suite – Compilation & Déploiement (v3.29.0)

> NOUVEAUTÉ v3.29.0 : Sélection dynamique du contrôleur TFT (ILI9341 ou ST7789) et configuration de la résolution dans `config.h`. Voir README et CONFIG_FR.md.

> **AVERTISSEMENT** : v3.29.0 permet le choix du contrôleur TFT (ILI9341/ST7789) et la résolution dans `config.h`. Assurez-vous que votre câblage et la cible compilée correspondent aux broches documentées. Lisez [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) et [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) avant de flasher.

Ce document décrit les toolchains supportées et la checklist recommandée pour livrer le firmware 3.16.0.

> **Important :** La version 3.12.0 a été migrée depuis Arduino IDE vers **PlatformIO**. La version originale Arduino IDE [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) est maintenant **archivée**. Le support Bluetooth/BLE a été **supprimé**.

## Environnements de Build

Le projet supporte trois configurations matérielles distinctes via les environnements PlatformIO définis dans `platformio.ini` :

### 1. esp32s3_n16r8 (Par défaut)
- **Carte :** ESP32-S3-DevKitC-1 N16R8
- **Flash :** 16Mo (mode QIO, 80MHz)
- **PSRAM :** 8Mo OPI/QSPI (80MHz, activée)
- **Partition :** `huge_app.csv`
- **Define Cible :** `TARGET_ESP32_S3`
- **Pin Mapping :** Optimisé pour disposition GPIO ESP32-S3

### 2. esp32s3_n8r8
- **Carte :** ESP32-S3-DevKitC-1 N8R8
- **Flash :** 8Mo
- **PSRAM :** 8Mo (activée)
- **Partition :** `huge_app.csv`
- **Define Cible :** `TARGET_ESP32_S3`
- **Pin Mapping :** Identique au N16R8 (disposition ESP32-S3)

### 3. esp32devkitc
- **Carte :** ESP32-DevKitC (Classic)
- **Flash :** 4Mo
- **PSRAM :** Non disponible
- **Partition :** `default.csv`
- **Define Cible :** `TARGET_ESP32_CLASSIC`
- **Pin Mapping :** Adapté aux contraintes GPIO ESP32 Classic

## Configuration Toolchain

Le projet utilise PlatformIO pour la gestion de compilation. Toutes les dépendances sont déclarées dans `platformio.ini` :

1. Ouvrir le projet dans **Visual Studio Code** avec l'extension **PlatformIO IDE**.
2. Sélectionner votre environnement cible dans `platformio.ini` :
   ```ini
   [platformio]
   default_envs = esp32s3_n16r8  ; Changer pour esp32s3_n8r8 ou esp32devkitc
   ```
3. Lancer **Build** (Ctrl+Alt+B) pour compiler, puis **Upload** (Ctrl+Alt+U) pour flasher.
4. Ouvrir **Serial Monitor** (Ctrl+Alt+S) à 115200 bauds pour vérifier.

Alternativement, utiliser la ligne de commande PlatformIO :
```bash
# Build & upload environnement par défaut
pio run --target upload
pio device monitor --baud 115200

# Build environnement spécifique
pio run -e esp32s3_n16r8 --target upload
pio run -e esp32s3_n8r8 --target upload
pio run -e esp32devkitc --target upload

# Build tous les environnements
pio run

# Nettoyage
pio run --target clean
```

**Note :** Arduino IDE et Arduino CLI ne sont **plus supportés** pour cette version. Utiliser le dépôt archivé [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) pour la compatibilité Arduino IDE.

## Statut de Build (2025-11-27)
1. `esp32s3_n16r8` : ✓ Build OK, ✓ Upload OK, ✓ Testé
2. `esp32s3_n8r8` : ✓ Build OK, ✓ Compilation validée
3. `esp32devkitc` : ✓ Build OK, ⚠ Tests matériels en attente

## Notes Configuration des Broches

Chaque environnement possède des mappings de broches dédiés dans `include/config.h` :
- **ESP32-S3 :** I2C sur SDA=15/SCL=16, LED RGB sur 21/45/47 (Rouge/Vert/Bleu), TFT sur GPIOs 7-13, GPS sur 18/17/8, capteurs mis à jour (voir PIN_MAPPING_FR.md). Laisser GPIO19/20 libres pour l’USB D-/D+ afin de garder l’OTG stable.
- **ESP32 Classic :** I2C sur SDA=21/SCL=22, LED RGB sur 12/14/15, TFT sur GPIOs 13/18/19/23/26/27, GPS sur 16/17/4, capteurs mis à jour (voir PIN_MAPPING_FR.md).

Voir [PIN_MAPPING_FR.md](PIN_MAPPING_FR.md) pour la référence complète.

## Nouveautés v3.17.1
Pin mapping par défaut actualisé pour ESP32-S3 et ESP32 Classic (GPS, TFT, LED RGB, capteurs, boutons) et documentation synchronisée ; version portée à 3.17.1.

## Nouveautés v3.16.0
Configuration dynamique des écrans OLED et TFT via interface web, incluant résolution et pin mapping. Surveillance réseau avec journalisation des IP clients. Voir CHANGELOG_FR.md pour détails.

## Nouveautés v3.15.0
1. **Support multi-environnements :** Trois configurations de build distinctes avec mappings de broches spécifiques au matériel.
2. **Compilation conditionnelle :** Defines `TARGET_ESP32_S3` et `TARGET_ESP32_CLASSIC` pour code spécifique à la plateforme.
3. **Mappings partagés :** Affectations de broches communes entre ESP32-S3 N8R8 et ESP32 Classic lorsque possible.
4. **Documentation :** Références pin mapping et instructions de build mises à jour pour les trois cibles.

## Checklist Pré-déploiement
- [ ] Mettre à jour `secrets.h` avec les identifiants production et, le cas échéant, les paramètres entreprise.
- [ ] Vérifier que `DIAGNOSTIC_VERSION` vaut `3.17.1` dans `platformio.ini` et la documentation.
- [ ] Vérifier que l'environnement cible est correctement défini dans `platformio.ini` (`default_envs`).
- [ ] Réviser les mappings de broches dans `config.h` pour votre configuration matérielle spécifique.
- [ ] Compiler les ressources multilingues sans avertissement (`languages.h`).
- [ ] Exécuter un cycle complet sur une carte de référence et exporter les rapports JSON/CSV.
- [ ] Capturer des captures d'écran ou impressions du tableau de bord pour les notes de version si nécessaire.

## Tests d'acceptation
| Test | Procédure | Résultat attendu |
|------|-----------|------------------|
| Smoke test connectivité | Connexion au Wi-Fi labo, lancer un diagnostic rapide. | PASS avec RSSI > -70 dBm, ping < 40 ms. |
| Endurance mémoire | Lancer 5 diagnostics complets consécutifs. | Aucune saturation du tas, PSRAM stable. |
| Validation périphériques | Brancher OLED + chaîne NeoPixel, activer les modules. | PASS avec FPS stable et animation cohérente. |
| Sanity REST API | Déclencher `/api/run` puis `/api/status`. | File d'attente reçue et payload JSON valide. |

## Packaging release
1. Taguer le dépôt avec `v3.9.0` (tag annoté).
2. Joindre les binaires compilés (`.bin`) pour chaque carte si diffusion via GitHub Releases.
3. Publier les notes en se référant au [CHANGELOG_FR.md](../CHANGELOG_FR.md).
4. Partager les liens du wiki mis à jour (`docs/home.md`) avec les équipes support.

## Considérations OTA
- La diffusion principale reste le flash USB. Pour l'OTA, intégrer votre bootloader et vérifier que le binaire reste sous la taille
  de partition OTA.
- Toujours valider la signature ou le checksum après transfert OTA pour éviter un diagnostic corrompu.

## Stratégie de rollback
- Conserver une image stable (ex. v3.2.21-maint) prête pour un retour arrière d'urgence.
- Documenter les étapes de rollback dans la procédure interne en se référant à ce guide pour le reflashing.

## Ressources associées
- [Guide d'installation](INSTALL_FR.md)
- [Guide de configuration](CONFIG_FR.md)
- [Notes de version](../CHANGELOG_FR.md)
