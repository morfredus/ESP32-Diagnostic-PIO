# Dépannage (FR) – v3.29.0

> NOUVEAUTÉ v3.29.0 : Sélection dynamique du contrôleur TFT (ILI9341 ou ST7789) et configuration de la résolution dans `config.h`. Voir README_FR.md et CONFIG_FR.md.

> **AVERTISSEMENT** : v3.28.5 corrige des doublons de mapping pour ESP32 Classic et conserve le remapping GPIO pour ESP32-S3. Assurez-vous que votre câblage et la cible compilée correspondent aux broches documentées. Lisez [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) et [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) avant de flasher.

**Support multi-environnements :** Cette version supporte trois environnements de compilation (ESP32-S3 N16R8, ESP32-S3 N8R8, ESP32 Classic) avec des mappages de broches spécifiques au matériel. Assurez-vous d'avoir sélectionné et flashé le bon environnement pour votre carte avant de dépanner.

## Mauvais environnement de compilation sélectionné
- **Symptôme :** Périphériques non fonctionnels, conflits de broches, comportement inattendu.
- **Solution :** Vérifiez que votre matériel correspond à l'environnement de compilation. Les cartes ESP32-S3 nécessitent les environnements `esp32s3_n16r8` ou `esp32s3_n8r8`. Les cartes ESP32 Classic DevKitC nécessitent l'environnement `esp32devkitc`.
- **Vérification CLI :** Exécutez `pio run -t envlist` pour voir les environnements disponibles. Flashez le bon avec `pio run -e <env_name> -t upload`.
- **Vérification des broches :** Consultez [PIN_MAPPING_FR.md](PIN_MAPPING_FR.md) pour confirmer que votre configuration matérielle correspond au firmware compilé.

## Fonctionnalité BLE
**Note :** Le support BLE a été supprimé dans cette version PlatformIO. Pour les diagnostics BLE, utilisez la version archivée [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE).

## Le changement de langue n'a aucun effet
- Assurez-vous d'exécuter le firmware 3.8.0 afin que les liaisons de traduction en direct restent actives tout en bénéficiant de l'écran Wi-Fi et des garde-fous NimBLE.
- Vérifiez que `languages.h` est présent à côté du sketch et compilé correctement.
- Videz le cache du navigateur (Ctrl+F5) pour recharger les traductions.
- Contrôlez que `/api/get-translations` renvoie HTTP 200 via la console ou un client REST.

## Les diagnostics additionnels affichent « Non teste » sans accent
- Passez en version 3.9.0 (ou toute maintenance 3.x ultérieure) pour conserver l'intitulé « Non testé » et bénéficier du rafraîchissement instantané des traductions sur les onglets de diagnostics.
- Rafraîchissez le tableau de bord (Ctrl+F5) afin de recharger les traductions mises à jour.

## L'écran OLED reste noir
- Relancez `/api/oled-config?sda=<pin>&scl=<pin>` avec le câblage correct.
- Contrôlez l'adresse I2C (0x3C par défaut). Remplacez l'écran s'il ne répond pas sur le bus.
- Assurez une alimentation stable (3,3 V, masse commune avec l'ESP32).

## L'écran TFT reste noir (v3.11.0+)
- Vérifiez que la configuration des pins TFT dans `include/config.h` correspond au câblage matériel.
- Assurez-vous que le pin de rétro-éclairage (TFT_BL) est correctement configuré sur GPIO 15 et ne conflicte pas avec NeoPixel (GPIO 48).
- Vérifiez les connexions SPI : MOSI (GPIO 11), SCLK (GPIO 12), CS (GPIO 10), DC (GPIO 9), RST (GPIO 46).
- Vérifiez que `ENABLE_TFT_DISPLAY` est défini à `true` dans config.h.
- Assurez une alimentation suffisante pour l'écran TFT (3.3V ou 5V selon le modèle, masse commune avec l'ESP32).
- Consultez le moniteur série pour les messages d'initialisation TFT : "[TFT] Initializing ST7789 display..."

## La connexion Wi-Fi échoue
- Revérifiez les couples SSID/mot de passe dans `secrets.h`.
- Surveillez le moniteur série pour les codes `WL_NO_SSID_AVAIL` ou `WL_CONNECT_FAILED`.
- Rapprochez la carte du point d'accès ou testez un SSID 2,4 GHz.

## Alertes de fragmentation mémoire dans les exports
- Appelez `/api/memory-details` et analysez le pourcentage de `fragmentation` ainsi que les drapeaux PSRAM en vérifiant que les libellés traduits correspondent à la langue active en 3.8.0.
- Activez la PSRAM sur les cartes ESP32-S3 afin de garder le drapeau `psramAvailable` cohérent avec la documentation.
- Si la fragmentation reste au-dessus de 20 %, redémarrez la carte puis relancez les diagnostics pour purger l'usage temporaire de la heap.

## Les tests GPIO détectent des conflits
- Libérez la broche de tout périphérique (I2C, SPI, UART) avant le balayage.
- Certaines broches sont entrée uniquement ou réservées (ex. GPIO34-39). Consultez le schéma de votre carte.

## Les exports sont vides
- Attendez la fin des diagnostics avant de déclencher un export.
- Autorisez les téléchargements depuis l'adresse IP de l'appareil dans votre navigateur.
- Vérifiez les logs série pour détecter une alerte mémoire pouvant interrompre le streaming.
