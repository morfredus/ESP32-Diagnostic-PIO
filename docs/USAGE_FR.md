# Guide d’utilisation (FR) – v3.29.0

> NOUVEAUTÉ v3.29.0 : Sélection dynamique du contrôleur TFT (ILI9341 ou ST7789) et configuration de la résolution dans `config.h`. Voir README_FR.md et CONFIG_FR.md.

> **AVERTISSEMENT** : v3.28.5 corrige des doublons de mapping pour ESP32 Classic et conserve le remapping GPIO pour ESP32-S3. Assurez-vous que votre câblage et la cible compilée correspondent aux broches documentées. Lisez [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) et [docs/PIN_MAPPING.md](docs/PIN_MAPPING.md) avant de flasher.

**Support multi-environnements :** La version 3.17.1 actualise les mappings par défaut pour ESP32-S3 et ESP32 Classic tout en conservant les trois environnements spécifiques au matériel et les optimisations mémoire pour l'ESP32 Classic. Avant la première utilisation, assurez-vous d'avoir sélectionné et flashé le bon environnement pour votre carte (voir [BUILD_AND_DEPLOY_FR.md](BUILD_AND_DEPLOY_FR.md)).

## 1. Premier démarrage
1. Ouvrez le moniteur série à 115200 bauds juste après le flash.
2. Patientez pendant la connexion Wi-Fi. En cas de succès, l'IP attribuée et l'URL `http://ESP32-Diagnostic.local` sont affichées, et tout écran OLED raccordé reflète les étapes association/DHCP/progression.
3. En cas d'échec, la console fournit des indices (mot de passe erroné, réseau introuvable, etc.). Mettez `secrets.h` à jour.

## 2. Accéder au tableau de bord
- Rendez-vous sur l'adresse IP affichée ou sur `http://ESP32-Diagnostic.local` (mDNS) depuis un appareil du même réseau.
- L'interface s'ouvre en français. Utilisez le commutateur FR/EN en haut à droite pour changer de langue sans rechargement.

## 3. Aperçu de l'interface web
Le tableau de bord est organisé en onglets :
- **Vue d'ensemble** – résumé matériel, mémoire, version firmware, statut Bluetooth®.
- **LEDs** – contrôle de la LED embarquée et motifs NeoPixel/WS2812B.
- **Écrans** – diagnostics OLED avec animations pas-à-pas et déclenchement manuel.
- **Tests avancés** – benchmarks CPU/mémoire, stress tests et partitions flash.
- **GPIO** – test interactif des broches avec état en direct.
- **Wi-Fi** – résultats détaillés (RSSI, canal, sécurité, PHY, largeur de bande).
- **Performance** – historique des benchmarks et métriques temps réel.
- **Export** – téléchargement des rapports TXT/JSON/CSV ou ouverture de la vue imprimable.

La version 3.15.1 supporte trois environnements de compilation avec différents mappages de broches pour I2C, TFT, LED RGB et boutons. Les configurations spécifiques au matériel sont automatiquement compilées selon l'environnement sélectionné. ESP32 Classic bénéficie d'optimisations mémoire pour un fonctionnement fiable de l'interface web.

La barre de navigation s'adapte aux mobiles et conserve l'onglet actif.

## 4. Outils Bluetooth® Low Energy
- La diffusion BLE démarre automatiquement lorsque la carte est compatible.
- Une cartouche dédiée dans l'onglet Vue d'ensemble affiche le nom, l'état de connexion et autorise le renommage.
- Les noms invalides sont refusés avec un HTTP 400 pour préserver la cohérence du service GATT.

## 5. API REST
Toutes les routes renvoient du JSON sauf mention contraire :
- `GET /api/test-gpio` – lance le balayage GPIO.
- `GET /api/wifi-scan` – effectue un scan Wi-Fi.
- `GET /api/benchmark` – exécute les benchmarks CPU et mémoire.
- `GET /api/set-language?lang=fr|en` – change la langue de l'interface.
- `GET /api/get-translations` – récupère le catalogue de traductions courant.
- `GET /api/oled-config?sda=<pin>&scl=<pin>` – reconfigure les broches I2C de l'OLED.
- `GET /api/oled-test` – lance la séquence complète de tests OLED (~25 secondes).
- `GET /api/oled-message?message=TEXTE` – affiche un message personnalisé sur l'OLED.
- `GET /api/memory-details` – renvoie les métriques flash/PSRAM/SRAM ainsi que le pourcentage de fragmentation pour l'analyse post-diagnostic.
- `GET /export/txt`, `/export/json`, `/export/csv` – télécharge les rapports dans différents formats.

## 6. Rapports et journalisation
- La sortie série reflète les actions clés (Wi-Fi, BLE, résultats de tests).
- Les exports incluent les informations carte, la mémoire, les benchmarks, le scan Wi-Fi, les GPIO et l'état OLED.
- Utilisez le JSON pour l'analyse automatisée et TXT/CSV pour la consultation manuelle.
- Exploitez l'endpoint `/api/memory-details` lorsque les exports signalent une fragmentation et, en 3.8.0, contrôlez que les réponses de scan BLE renvoient soit des résultats frais, soit un message d'erreur explicite si la radio est occupée.

## 7. Boutons matériels (optionnel)
- Si activés via `ENABLE_BUTTONS`, les actions par défaut s’appliquent sans modifier le pin mapping :
  - BTN1 : appui court → bip bref du buzzer.
  - BTN2 : appui court → cycle des couleurs de la LED RGB (rouge → vert → bleu → blanc).
- Les broches sont spécifiques à la cible (`PIN_BUTTON_1` / `PIN_BUTTON_2` définies dans `config.h`). Sur ESP32 Classic, les GPIO 34–39 n’ont pas de pull‑up interne ; prévoir une résistance externe si nécessaire pour BTN2=35.

## 8. Bonnes pratiques
- Relancez un scan Wi-Fi après tout déplacement pour rafraîchir les RSSI.
- Arrêtez les animations NeoPixel avant de couper l'alimentation des LED externes.
- Après changement des broches OLED, exécutez `/api/oled-config` puis `/api/oled-test` pour valider le câblage.

## 9. Checklist de débogage post-release (3.8.0)
- Redémarrez la carte avec un OLED connecté et vérifiez que l'écran de démarrage Wi-Fi progresse bien sur association, DHCP puis réussite avant chargement du tableau de bord.
- Déclenchez `/api/wifi-scan` et `/api/ble-scan` (ou le bouton d'interface) pour confirmer que les builds NimBLE renvoient des résultats valides et reprennent la diffusion sans conversion erronée.
- Téléchargez les rapports TXT/JSON/CSV et assurez-vous que les noms de fichiers incluent la version 3.9.0 tandis que les sections BLE et Wi-Fi reflètent les consignes de compatibilité renforcées.
