# ESP32 Diagnostic Suite – Vue d'ensemble (v3.28.5)

> **AVERTISSEMENT** : v3.28.5 corrige des doublons de mapping pour ESP32 Classic et conserve le remapping GPIO pour ESP32-S3. Assurez-vous que votre câblage et la cible compilée correspondent aux broches documentées. Lisez [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) et [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) avant de flasher.

La suite ESP32 Diagnostic fournit une boîte à outils prête pour la production afin de valider les cartes basées sur ESP32 avant déploiement. La version 3.15.1 inclut des **optimisations mémoire critiques pour ESP32 Classic** et le support multi-environnements avec trois configurations de build distinctes optimisées pour différentes variantes matérielles ESP32.

## Mission
- Proposer une méthodologie homogène pour qualifier les cartes ESP32-S3 et ESP32 avec plusieurs variantes matérielles.
- Supporter trois cartes spécifiques avec environnements dédiés : **ESP32-S3 N16R8** (principale), **ESP32-S3 N8R8**, et **ESP32 Classic DevKitC**.
- Mappings de broches spécifiques au matériel via compilation conditionnelle pour placement périphérique optimal.
- Offrir une interface web dédiée aux diagnostics en laboratoire comme sur le terrain, avec un minimum de configuration.
- Maintenir l'alignement entre firmware, documentation et rapports exportés afin de faciliter la traçabilité des anomalies.

| Domaine | Points clés |
|---------|-------------|
| Tableau de bord web | Interface HTML réactive servie directement par l'ESP32 avec indicateurs temps réel, journal d'activité et exports. |
| Vérifications automatiques | Planificateurs couvrant la connectivité, la mémoire, les bus de périphériques, l'affichage et les LEDs. |
| Reporting | Téléchargements en TXT/JSON/CSV, vue imprimable et hooks REST API pour l'intégration CI. |
| Localisation | Bascule FR/EN à l'exécution côté firmware (sortie série) et interface web. |
| Extensibilité | Registre modulaire de tests, bus d'événements pour callbacks personnalisés et helpers OLED/NeoPixel optionnels. |

## Nouveauté v3.29.0 : Sélection dynamique du contrôleur TFT

- Vous pouvez désormais choisir le contrôleur TFT (`ILI9341` ou `ST7789`) et la résolution directement dans `include/config.h` :
```cpp
#define TFT_CONTROLLER      "ST7789" // ou "ILI9341"
#define TFT_WIDTH           240
#define TFT_HEIGHT          320
```
- L'affichage est identique quel que soit le contrôleur.
- Voir la documentation et le changelog pour plus de détails.

## Focus de la version 3.15.1

**Correction mémoire critique** : La version 3.15.1 résout les échecs de chargement de l'interface web sur cartes ESP32 Classic en implémentant le streaming PROGMEM par morceaux pour le contenu JavaScript, réduisant le pic d'allocation heap de ~50Ko.

## Focus de la version 3.15.0 (Support multi-environnements)
- **Support multi-environnements :** Trois environnements de build PlatformIO (`esp32s3_n16r8`, `esp32s3_n8r8`, `esp32devkitc`) avec configurations de broches spécifiques au matériel.
- **Compilation conditionnelle :** Chemins de code spécifiques à la cible utilisant les defines `TARGET_ESP32_S3` et `TARGET_ESP32_CLASSIC`.
- **Déploiement flexible :** Base de code unique supporte ESP32-S3 (Flash 16Mo/8Mo avec PSRAM) et ESP32 Classic (Flash 4Mo, sans PSRAM).
- **Documentation complète :** Référence pin mapping, guides de build et détails de configuration mis à jour pour les trois cibles.
- **Support écran TFT ST7789 :** Support complet des écrans TFT 240x240 avec écran de démarrage et statut WiFi/IP en temps réel (v3.11.0+).
- **Double affichage :** Fonctionnement simultané OLED et TFT pour visualisation diagnostique améliorée.
- **Stabilité améliorée :** Résolution des problèmes de chargement JavaScript, erreurs de compilation FPSTR(), et optimisation mémoire avec livraison en streaming.
- **Validation configuration :** Finalisation du mapping des pins pour ESP32-S3 DevKitC-1 N16R8 avec rétro-éclairage TFT sur GPIO 15.
- **Optimisation PlatformIO :** Configuration PSRAM améliorée avec stabilité et performance accrues.

## Carte du wiki
Consultez cette carte pour accéder directement à la bonne rubrique :

- [Accueil](home_FR.md) – porte d'entrée, points forts et objectifs généraux.
- [Installation](INSTALL_FR.md) – préparation de l'environnement, toolchain, pilotes USB et méthodes de flash.
- [Configuration](CONFIG_FR.md) – identifiants Wi-Fi, langue par défaut, ordonnancement des diagnostics et options de journalisation.
- [Utilisation](USAGE_FR.md) – workflows opérateur, formats d'export et interprétation du tableau de bord.
- [Architecture](ARCHITECTURE_FR.md) – couches firmware, planification des tâches, structure mémoire et gabarits d'interface.
- [Dépannage](TROUBLESHOOTING_FR.md) – procédures de récupération, matrice des codes d'erreur et scénarios de reset sécurisé.
- [Contribution](CONTRIBUTING_FR.md) – conventions de code, processus de revue et attentes en matière de tests.
- [Matrice des fonctionnalités](FEATURE_MATRIX_FR.md) – couverture matérielle par famille de cartes et périphériques optionnels.
- [Modules de diagnostic](DIAGNOSTIC_MODULES_FR.md) – description détaillée de chaque test automatisé et des métriques produites.
- [Guide de l'interface web](WEB_INTERFACE_FR.md) – anatomie de l'UI, états des composants et directives d'accessibilité.
- [Référence REST API](API_REFERENCE_FR.md) – endpoints, payloads, limites de débit et scénarios d'automatisation.
- [Compilation & déploiement](BUILD_AND_DEPLOY_FR.md) – compilation firmware, contrôles qualité et checklist de publication.
- [Directives de sécurité](SECURITY_FR.md) – renforcement Wi-Fi, rotation des identifiants et sécurité OTA.
- [FAQ](FAQ_FR.md) – réponses rapides aux questions d'intégration, support et déploiement les plus fréquentes.

## Terminologie
- **Cycle de diagnostic** – exécution ordonnée de tous les modules activés avec agrégation en RAM jusqu'à l'export.
- **Rapport instantané** – copie figée du dernier cycle, exportable via l'interface web, la REST API ou la commande série.
- **Profil contrôleur** – ensemble de configuration pré-définie (type de carte, options d'interface) chargé au démarrage.
- **Version de maintenance** – livraison axée sur la documentation et la stabilité sans modification de la surface fonctionnelle.

## Ressources associées
- [CHANGELOG_FR.md](../CHANGELOG_FR.md) – suivi chronologique des corrections et améliorations.
- [Table de compatibilité ESP-IDF](https://docs.espressif.com/projects/esp-idf/fr/latest/esp32/) pour comparer avec les attentes de la plateforme.
- [Issues GitHub](https://github.com/morfredus/ESP32-Diagnostic/issues) – suivi des anomalies et propositions.
- **Note :** La version originale Arduino IDE [ESP32-Diagnostic-Arduino-IDE](https://github.com/morfredus/ESP32-Diagnostic-Arduino-IDE) est archivée.
