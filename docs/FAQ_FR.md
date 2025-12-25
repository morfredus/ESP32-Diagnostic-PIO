# ESP32 Diagnostic Suite – FAQ (v3.29.0)

> NOUVEAUTÉ v3.29.0 : Sélection dynamique du contrôleur TFT (ILI9341 ou ST7789) et configuration de la résolution dans `config.h`. Voir README_FR.md et CONFIG_FR.md.

> **AVERTISSEMENT** : v3.28.5 corrige des doublons de mapping pour ESP32 Classic et conserve le remapping GPIO pour ESP32-S3. Assurez-vous que votre câblage et la cible compilée correspondent aux broches documentées. Lisez [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) et [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) avant de flasher.

## Généralités
**Q : Quelles cartes sont officiellement supportées ?**
R : Trois cartes sont officiellement supportées dans cette version :
- ESP32-S3-DevKitC-1 N16R8 (16Mo Flash, 8Mo PSRAM) — Cible principale
- ESP32-S3-DevKitC-1 N8R8 (8Mo Flash, 8Mo PSRAM)
- ESP32-DevKitC (4Mo Flash, sans PSRAM)

Voir [FEATURE_MATRIX_FR.md](FEATURE_MATRIX_FR.md) pour la comparaison détaillée des capacités.

**Q : Puis-je lancer les diagnostics sans Wi-Fi ?**
R : Oui. La suite fonctionne en mode hotspot hors ligne ou via commandes série USB. Les modules réseau sont simplement ignorés.

**Q : Existe-t-il un mécanisme OTA ?**
R : Pas en natif. Vous pouvez intégrer le binaire à votre chargeur OTA. Consultez [BUILD_AND_DEPLOY_FR.md](BUILD_AND_DEPLOY_FR.md) pour la préparation.

## Installation & configuration
**Q : Où stocker les identifiants Wi-Fi ?**
R : Copier `secrets-example.h` en `secrets.h`, renseigner vos SSID/mots de passe et exclure le fichier du dépôt.

**Q : Comment activer/désactiver les modules optionnels ?**
R : Ajuster les flags dans `config.h` ou utiliser l'endpoint REST `POST /api/modules/<id>/toggle`. Voir [DIAGNOSTIC_MODULES_FR.md](DIAGNOSTIC_MODULES_FR.md).

**Q : Puis-je changer la langue par défaut ?**
R : Oui, définir `DEFAULT_LANGUAGE` à `LANG_FR` ou `LANG_EN` dans `config.h`.

## Exécution des diagnostics
**Q : Différence entre diagnostic complet et rapide ?**
R : Le complet exécute tous les modules. Le rapide ignore les stress tests périphériques et se concentre sur connectivité, mémoire et reporting.

**Q : Durée d'un cycle complet ?**
R : Sur ESP32-S3 connecté en Wi-Fi, compter ~90 secondes incluant les benchmarks réseau et tests OLED/NeoPixel.

**Q : Où sont stockés les rapports ?**
R : En RAM jusqu'à l'export. Utiliser le tableau de bord ou `/api/report` immédiatement après la fin du cycle.

## Dépannage
**Q : Le module OLED est indisponible.**
R : Vérifier le câblage I2C (SDA/SCL), l'adresse 0x3C et l'activation `ENABLE_OLED_TESTS`.

**Q : Les tests NeoPixel échouent sur ma carte.**
R : Vérifier que l'assignation de broche LED dans `config.h` correspond au câblage matériel. Certaines cartes nécessitent une alimentation externe dédiée pour le ruban LED (5V externe).

**Q : L'API REST renvoie 409.**
R : Un autre cycle est en cours. Attendre la fin ou appeler `POST /api/stop` avant de relancer.

## Développement
**Q : Comment ajouter un module personnalisé ?**
R : Implémenter la logique dans `registerCustomDiagnostics()` dans `votre fichier source principal`. Émettre PASS/WARN/FAIL et alimenter le générateur de rapport.

**Q : Des tests unitaires existent-ils ?**
R : Le projet privilégie les tests d'intégration matériels. Utiliser le tableau de bord et les exports pour valider le comportement.

**Q : Comment contribuer aux traductions ?**
R : Soumettre une pull request modifiant `languages.h` en gardant les versions EN/FR synchronisées.

## Support
**Q : Où signaler un bug ?**
R : Sur le [tracker GitHub](https://github.com/morfredus/ESP32-Diagnostic/issues) en joignant le dernier rapport JSON. Note : La version originale Arduino IDE [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) est archivée.

**Q : Un support commercial est-il disponible ?**
R : Non officiel. Les contributions communautaires sont encouragées via GitHub discussions et pull requests.
