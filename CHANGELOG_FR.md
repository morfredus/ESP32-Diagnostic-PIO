## [Version 3.33.1] - 2026-01-02

### 🐛 Corrections de Bugs

**Correction de Compilation pour ESP32 Classic**

Cette version corrective résout une erreur de compilation critique pour l'environnement `esp32devkitc` causée par des définitions de broches manquantes.

#### 🎯 Problèmes Corrigés

- **Définitions de Broches Manquantes** : Ajout des constantes `MOTION_SENSOR` et `NEOPIXEL` pour ESP32 Classic (DevKitC) dans `board_config.h`
  - `MOTION_SENSOR` → GPIO 34 (entrée uniquement, idéal pour capteur PIR)
  - `NEOPIXEL` → GPIO 2 (partagé avec LED_BUILTIN pour simplicité)

#### 📝 Détails Techniques

- **Fichiers Modifiés** :
  - `include/board_config.h` - Ajout des définitions de broches manquantes pour la section TARGET_ESP32_CLASSIC
  - `platformio.ini` - Incrémentation de version à 3.33.1

- **Erreurs de Compilation Corrigées** :
  ```
  src/main.cpp:216:25: error: 'MOTION_SENSOR' was not declared
  src/main.cpp:253:15: error: 'NEOPIXEL' was not declared
  include/web_interface.h:510:16: error: 'MOTION_SENSOR' was not declared
  ```

#### 🔧 Assignation des Broches (ESP32 Classic)

| Constante | GPIO | Type | Notes |
|-----------|------|------|-------|
| MOTION_SENSOR | 34 | Entrée | GPIO34 est entrée uniquement, idéal pour PIR |
| NEOPIXEL | 2 | Sortie | Partagé avec LED_BUILTIN |

#### ⚠️ Notes Importantes

- **ESP32-S3** : Aucun changement requis (broches déjà définies)
- **ESP32 Classic** : Les utilisateurs peuvent maintenant compiler et utiliser les fonctionnalités MOTION_SENSOR et NEOPIXEL
- **Partage GPIO** : NEOPIXEL (GPIO 2) est partagé avec LED_BUILTIN - les utilisateurs doivent choisir l'un ou l'autre dans leur configuration matérielle

### 🔄 Contrôle de Version

- **Version incrémentée** : `3.33.0` → `3.33.1` dans `platformio.ini`
- Ceci est une incrémentation de version **PATCH** selon SEMVER (correction de bug, rétrocompatible)

---

## [Version 3.33.0] - 2025-12-29

### 🚀 Améliorations TFT, NeoPixel et BOOT

- **Barre de progression TFT fluide et sans scintillement** lors de l'appui long sur BOOT
- **Rendu partiel optimisé** : seule la barre est redessinée, cadre dessiné une seule fois
- **NeoPixel violet (#8000FF) pendant toute la progression**
- **Earthbeat NeoPixel restauré** si BOOT relâché avant 100%
- **Retour écran de boot propre** après annulation
- **Machine d'état robuste** pour la gestion BOOT/Progression/Annulation/Action finale
- **Code refactorisé, lisible, optimisé**
- **Documentation interne détaillée**

#### 🛠️ Détails techniques
- Machine d'état : NORMAL, PROGRESSION, ANNULATION, ACTION FINALE
- Optimisation du rendu TFT (frameDrawn, lastProgress)
- Gestion fine NeoPixel (Earthbeat/violet)
- Nettoyage et commentaires dans le code

#### 📚 Documentation
- Mise à jour : `CHANGELOG.md`, `CHANGELOG_FR.md`, `README.md`, `README_FR.md`
- Version SEMVER : **3.33.0** (incrémentation mineure)

---

## [Version 3.31.1] - 2025-12-27

### 🎨 Améliorations de l'Interface

**Amélioration de la Visibilité de l'Indicateur "Bientôt disponible"**

Cette version corrective améliore la présentation visuelle de l'indicateur "Bientôt disponible" dans la section Carte SD de l'onglet Mémoire.

#### 🎯 Améliorations

- **Typographie Améliorée**: Augmentation de la taille de police à 1.3em pour une meilleure lisibilité
- **Emphase Visuelle**: Ajout du poids de police gras pour une présence visuelle plus forte
- **Disposition Centrée**: Le texte est maintenant centré dans son conteneur pour un meilleur équilibre visuel
- **Contraste Amélioré**: Augmentation de l'opacité à 1.0 pour une visibilité maximale
- **Meilleur Espacement**: Ajout de 15px de marge et 10px de padding pour une meilleure séparation visuelle

#### 📝 Détails Techniques

- **Fichiers Modifiés**:
  - `web_src/styles.css` - Amélioration du style de la classe `.coming`
  - `include/web_interface.h` - Mise à jour avec le CSS minifié

- **Modifications CSS**:
  ```css
  .coming {
      color: #f5a623;
      font-size: 1.3em;        /* Nouveau: texte plus grand */
      font-weight: bold;        /* Nouveau: emphase */
      font-style: italic;
      text-align: center;       /* Nouveau: centré */
      opacity: 1;               /* Modifié: de 0.85 */
      margin: 15px 0;           /* Nouveau: espacement */
      padding: 10px;            /* Nouveau: espacement */
  }
  ```

#### 🌍 Internationalisation

- Aucune modification des clés i18n
- Clé de traduction existante `coming_soon` maintenue:
  - Anglais: "Coming Soon"
  - Français: "Bientôt disponible"

### 🔄 Contrôle de Version

- **Version incrémentée**: `3.31.0` → `3.31.1` dans `platformio.ini`
- Ceci est une incrémentation de version **PATCH** selon SEMVER (petite amélioration UI, rétrocompatible)

---

## [Version 3.31.0] - 2025-12-27

### ✨ Nouvelles Fonctionnalités

**Code Source Lisible de l'Interface Web avec Minification Automatique**

Cette version introduit une refonte complète du workflow de développement de l'interface web, rendant le code HTML/CSS/JavaScript maintenable tout en préservant une utilisation optimale de la mémoire sur l'ESP32.

#### 🎯 Améliorations Clés

- **Fichiers Sources Lisibles**: Tout le code de l'interface web est désormais disponible en format lisible dans `web_src/`
  - `styles.css` - CSS formaté et commenté (~13 Ko lisible)
  - `app.js` - JavaScript formaté et documenté pour l'interface complète (~115 Ko lisible)
  - `app-lite.js` - JavaScript formaté pour l'interface lite ESP32 Classic (~3,8 Ko lisible)

- **Workflow de Minification Automatique**: Deux scripts Python automatisent le cycle de développement
  - `tools/extract_web_sources.py` - Extrait et formate le code depuis `web_interface.h`
  - `tools/minify_web.py` - Minifie les fichiers sources et met à jour `web_interface.h`

- **Optimisation Mémoire Préservée**: Le code minifié reste embarqué dans le firmware
  - CSS: ~26% de réduction (13 Ko → 10 Ko)
  - JavaScript Complet: ~18% de réduction (115 Ko → 94 Ko)
  - JavaScript Lite: ~28% de réduction (3,8 Ko → 2,8 Ko)

- **Convivial pour les Développeurs**: Documentation complète dans `web_src/README.md`
  - Instructions détaillées du workflow
  - Bonnes pratiques
  - Guide de dépannage
  - Exemples d'utilisation des outils

### 🔧 Changements Techniques

**Structure du Projet**
- Nouveau répertoire: `web_src/` - Contient tous les fichiers sources lisibles de l'interface web
- Nouveau répertoire: `tools/` - Contient les scripts Python pour le workflow de minification
- Nouveaux fichiers:
  - `web_src/README.md` - Documentation complète du workflow
  - `web_src/styles.css` - Source CSS lisible
  - `web_src/app.js` - Source JavaScript lisible (complet)
  - `web_src/app-lite.js` - Source JavaScript lisible (lite)
  - `tools/extract_web_sources.py` - Script d'extraction et de formatage
  - `tools/minify_web.py` - Script de minification

**Dépendances**
- Packages Python requis pour la minification:
  - `rcssmin` - Minification CSS
  - `rjsmin` - Minification JavaScript
  - `jsbeautifier` - Formatage JavaScript
  - `cssbeautifier` - Formatage CSS

**Workflow**
1. Éditer les fichiers sources lisibles dans `web_src/`
2. Exécuter `python tools/minify_web.py` pour mettre à jour `include/web_interface.h`
3. Compiler et téléverser le firmware comme d'habitude

### 📝 Notes de Migration

- **Aucun changement incompatible**: Le fichier `web_interface.h` existant continue de fonctionner exactement comme avant
- **Nouveau workflow de développement**: Les développeurs doivent maintenant éditer les fichiers sources dans `web_src/` au lieu de `web_interface.h`
- **Configuration unique**: Installer les dépendances Python: `pip install rcssmin rjsmin jsbeautifier cssbeautifier`

### 🎯 Avantages

- **Maintenabilité**: Le code est désormais lisible, correctement formaté et commenté
- **Collaboration**: Plusieurs développeurs peuvent facilement comprendre et modifier l'interface web
- **Contrôle de Version**: Les diffs Git sont significatifs et montrent les vrais changements de code
- **Aucun Impact sur les Performances**: Le code minifié dans le firmware reste optimisé
- **Flexibilité**: Facile de personnaliser l'interface web sans manipuler du code minifié
- **Documentation**: Un README complet guide les développeurs à travers le nouveau workflow

### 📚 Mises à Jour de la Documentation

- Nouveau `web_src/README.md` - Guide complet du workflow et de l'utilisation
- Mise à jour de la documentation de la structure du projet pour refléter les nouveaux répertoires
- Ajout d'exemples de workflow de minification et de bonnes pratiques

### 🔄 Contrôle de Version

- **Version incrémentée**: `3.30.0` → `3.31.0` dans `platformio.ini`
- Ceci est une incrémentation de version **MINEURE** selon SEMVER (nouvelle fonctionnalité, rétrocompatible)

---

## [Version 3.30.0] - 2025-12-25

### ✨ Nouvelles Fonctionnalités

**Sélection Dynamique du Driver TFT depuis l'Interface Web (Changement en Runtime)**

- **Changement de Driver TFT en Runtime**: Basculez entre les drivers ILI9341 et ST7789 dynamiquement depuis l'interface Web sans recompilation
- **Aucun Redémarrage Requis**: Changez le driver TFT actif instantanément sans redémarrer l'ESP32
- **Intégration Interface Web**: Nouveau sélecteur de driver dans la section de configuration TFT
- **Support Dual Driver**: Les deux drivers ILI9341 et ST7789 sont chargés simultanément et peuvent être échangés à la volée
- **Rétrocompatibilité**: La sélection du driver par défaut depuis `config.h` est préservée au démarrage

### 🔧 Modifications Techniques

**Architecture Backend**
- `include/tft_display.h`: Refactorisation complète pour supporter le changement de driver en runtime
  - Nouvel enum `TFT_DriverType` pour l'identification des drivers
  - Initialisation dynamique avec `initTFT(driverType, width, height, rotation)`
  - Nouvelle fonction `switchTFTDriver()` pour changer de driver à la volée
  - Désinitialisation propre avec `deinitTFT()`
  - Pointeur générique `Adafruit_GFX*` pour un accès unifié aux drivers

**Améliorations API**
- `src/main.cpp`:
  - `handleTFTConfig()`: Nouveau paramètre `driver` pour le changement dynamique
  - `handleScreensInfo()`: Retourne maintenant le type de driver actif
  - Nouvelle variable globale `tftDriver` (String) pour suivre le driver actif

**Interface Web**
- `include/web_interface.h`:
  - Nouveau sélecteur de driver dans la section de configuration TFT
  - Fonction `configTFT()` mise à jour pour envoyer le paramètre driver
  - Sélection du driver persistée et affichée dans l'UI

**Configuration**
- `include/config.h`: Commentaires mis à jour pour refléter le support dynamique des drivers
- `platformio.ini`: Version passée à 3.30.0

### 📝 Notes de Migration

- **Aucun changement cassant**: Les configurations existantes continuent de fonctionner comme avant
- **Nouvelle capacité**: Les utilisateurs peuvent maintenant tester différents drivers TFT sans reflasher le firmware
- **Comportement par défaut**: Le driver spécifié dans `config.h` (`TFT_USE_ILI9341` ou `TFT_USE_ST7789`) est utilisé au démarrage

### 🎯 Cas d'Usage

- **Tests Matériels**: Testez rapidement la compatibilité avec différents contrôleurs TFT
- **Échange d'Écrans**: Changez d'écran TFT sans recompiler et reflasher
- **Prototypage**: Évaluez différents contrôleurs d'affichage en temps réel
- **Dépannage**: Changez de driver pour identifier les problèmes matériels/logiciels

---

## [Version 3.29.0] - 2025-12-25

### ✨ Nouveautés

**Support dynamique des écrans TFT ILI9341 et ST7789**

- Ajout de la sélection du contrôleur d'écran (ILI9341 ou ST7789) dans `config.h` via la macro `TFT_CONTROLLER`.
- La résolution de l'écran TFT est désormais configurable dans `config.h`.
- L'affichage est strictement identique quel que soit le contrôleur sélectionné.
- Documentation et guides mis à jour pour refléter ce changement.

**Changement majeur :**
- Pour changer de contrôleur ou de résolution, modifiez simplement `TFT_CONTROLLER` et les macros associées dans `config.h`.

---
## [Version 3.28.5] - 2025-12-24

### 🐛 Corrections de Bugs

**Patch de Maintenance :** Correction du bouton encodeur bloqué + problèmes GPIO monitoring boutons

### Corrigé

#### 1. Bouton Encodeur Rotatif Reste "Pressed" ✅

**Problème :**
- Après avoir appuyé sur le bouton de l'encodeur rotatif, l'état restait "Pressed" même après relâchement
- L'interface affichait toujours "Pressed" en rouge, jamais "Released"
- Impossible de voir l'état réel du bouton en temps réel

**Cause Racine :**
- `handleRotaryPosition()` retournait `rotaryButtonPressed` (variable volatile ISR)
- Variable volatile mise à `true` par ISR lors de l'appui, mais jamais remise à `false` automatiquement
- Pour le monitoring temps réel, on doit lire l'état GPIO réel, pas la variable événementielle

**Solution :**
```cpp
// src/main.cpp:3199-3203 - Nouvelle fonction pour lire GPIO réel
int getRotaryButtonGPIOState() {
  if (rotary_sw_pin < 0 || rotary_sw_pin > 48) return -1;
  return digitalRead(rotary_sw_pin);
}

// src/main.cpp:4369-4379 - Utilisation dans handleRotaryPosition()
void handleRotaryPosition() {
  // v3.28.5 fix: Read REAL GPIO state for monitoring, not ISR variable
  int buttonGPIOState = getRotaryButtonGPIOState();
  bool buttonPressed = (buttonGPIOState == LOW && buttonGPIOState != -1);

  sendJsonResponse(200, {
    jsonNumberField("position", (int32_t)rotaryPosition),
    jsonBoolField("button_pressed", buttonPressed),  // Lit maintenant GPIO réel
    jsonBoolField("available", rotaryAvailable)
  });
}
```

**Impact :**
- ✅ Bouton encodeur affiche maintenant l'état correct en temps réel
- ✅ "Pressed" (rouge) quand bouton enfoncé
- ✅ "Released" (vert) quand bouton relâché
- ✅ Mise à jour immédiate (polling 100ms)

#### 2. Monitoring Boutons BOOT/1/2 Ne Fonctionne Pas ✅

**Problème :**
- Le monitoring des boutons BOOT, Button 1, Button 2 ne fonctionnait toujours pas
- Les états ne se mettaient pas à jour malgré la correction v3.28.4
- Les boutons restaient bloqués sur "Released"

**Cause Racine :**
- Les fonctions utilisaient des variables `static` (`buttonBootPin`, `button1Pin`, `button2Pin`)
- Problème potentiel de visibilité ou d'initialisation des variables statiques
- GPIO peut-être pas correctement accessible via ces variables

**Solution :**
```cpp
// src/main.cpp:3182-3199 - Lecture directe des constantes
// v3.28.5: Utilisation directe des constantes pour garantir l'accès GPIO
int getButtonBootState() {
  // Utilise la constante directement au lieu de la variable statique
  if (BUTTON_BOOT < 0 || BUTTON_BOOT > 48) return -1;
  return digitalRead(BUTTON_BOOT);
}

int getButton1State() {
  if (BUTTON_1 < 0 || BUTTON_1 > 48) return -1;
  return digitalRead(BUTTON_1);
}

int getButton2State() {
  if (BUTTON_2 < 0 || BUTTON_2 > 48) return -1;
  return digitalRead(BUTTON_2);
}

// src/main.cpp:4420-4428 - handleButtonState() utilise les constantes
if (buttonParam == "boot") {
  state = getButtonBootState();
  pin = BUTTON_BOOT;  // v3.28.5: Utilise la constante directement
} else if (buttonParam == "1" || buttonParam == "button1") {
  state = getButton1State();
  pin = BUTTON_1;
} else if (buttonParam == "2" || buttonParam == "button2") {
  state = getButton2State();
  pin = BUTTON_2;
}
```

**Impact :**
- ✅ Monitoring BOOT (GPIO 0) fonctionne maintenant
- ✅ Monitoring Button 1 (GPIO 38/34) fonctionne
- ✅ Monitoring Button 2 (GPIO 39/35) fonctionne
- ✅ Les états se mettent à jour en temps réel
- ✅ "Pressed" (rouge gras) / "Released" (vert) correct

**Fichiers Modifiés :**
- `src/main.cpp` :
  - Lignes 3182-3203 : Lecture d'état boutons mise à jour pour utiliser constantes, ajout `getRotaryButtonGPIOState()`
  - Lignes 4369-4379 : `handleRotaryPosition()` lit maintenant l'état GPIO réel
  - Lignes 4389-4407 : `handleButtonStates()` utilise constantes pour numéros de broches
  - Lignes 4420-4428 : `handleButtonState()` utilise constantes pour broches
- `platformio.ini` : Version 3.28.4 → 3.28.5

**Tests :**
1. **Encodeur Rotatif :**
   - Activer monitoring du bouton encodeur
   - Presser le bouton → "Pressed" (rouge) ✅
   - Relâcher → immédiatement "Released" (vert) ✅
   - Répéter plusieurs fois → états corrects ✅

2. **Boutons BOOT, 1, 2 :**
   - Activer monitoring pour chaque bouton
   - Presser GPIO 0/38/39 → "Pressed" immédiat ✅
   - Relâcher → "Released" immédiat ✅
   - Pas de blocage sur un état ✅

---

## [Version 3.28.4] - 2025-12-24

### 🐛 Correction de Bug

**Patch de Maintenance :** Correction du monitoring des boutons non fonctionnel - états bloqués sur "Released"

### Corrigé

#### Monitoring des Boutons Non Fonctionnel ✅

**Problème :**
- Le monitoring des boutons (BOOT, Bouton 1, Bouton 2) affichait toujours l'état "Released"
- Cliquer sur "Monitor Button" n'avait aucun effet - l'état ne se mettait jamais à jour
- Le JavaScript frontend appelait le mauvais endpoint API

**Cause Racine :**
- Le frontend appelle `/api/button-state?button=boot` (singulier) pour les requêtes de bouton individuel
- Le backend n'avait que l'endpoint `/api/button-states` (pluriel) qui retourne TOUS les boutons
- Incompatibilité d'endpoint : le frontend attendait une requête de bouton individuel, le backend fournissait une requête groupée
- Aucun gestionnaire de route enregistré pour `/api/button-state` (singulier)

**Solution :**
```cpp
// src/main.cpp:4395-4431 - Ajout du gestionnaire d'état de bouton individuel
void handleButtonState() {
  if (!server.hasArg("button")) {
    sendActionResponse(400, false, "Missing 'button' parameter");
    return;
  }

  String buttonParam = server.arg("button");
  int state = -1;
  int pin = -1;

  if (buttonParam == "boot") {
    state = getButtonBootState();
    pin = buttonBootPin;
  } else if (buttonParam == "1" || buttonParam == "button1") {
    state = getButton1State();
    pin = button1Pin;
  } else if (buttonParam == "2" || buttonParam == "button2") {
    state = getButton2State();
    pin = button2Pin;
  } else {
    sendActionResponse(400, false, "Invalid button parameter");
    return;
  }

  // LOW = pressé (pull-up), HIGH = relâché
  bool pressed = (state == LOW && state != -1);
  bool available = (state != -1);

  sendJsonResponse(200, {
    jsonBoolField("pressed", pressed),
    jsonBoolField("released", !pressed && available),
    jsonBoolField("available", available),
    jsonNumberField("pin", pin),
    jsonNumberField("raw_state", state)
  });
}

// src/main.cpp:5798 - Enregistrement de la route
server.on("/api/button-state", handleButtonState);
```

**Format de Réponse API :**
```json
GET /api/button-state?button=boot
{
  "pressed": false,
  "released": true,
  "available": true,
  "pin": 0,
  "raw_state": 1
}
```

**Impact :**
- ✅ Le monitoring des boutons fonctionne maintenant correctement
- ✅ L'état se met à jour en temps réel (polling 100ms) quand le monitoring est activé
- ✅ "Pressed" affiché en rouge gras quand le bouton est pressé
- ✅ "Released" affiché en vert quand le bouton est relâché
- ✅ Fonctionne pour BOOT (GPIO 0), Bouton 1 et Bouton 2

**Fichiers Modifiés :**
- `src/main.cpp` :
  - Lignes 4395-4431 : Ajout du gestionnaire `handleButtonState()`
  - Ligne 5798 : Enregistrement de la route `/api/button-state`
- `platformio.ini` : Version 3.28.3 → 3.28.4

**Tests :**
1. Naviguer vers la page "Dispositifs d'Entrée"
2. Cliquer sur "Monitor Button" pour le bouton BOOT
3. Presser le bouton GPIO 0 (BOOT) sur l'ESP32 - l'état devrait changer vers "Pressed" (rouge) ✅
4. Relâcher le bouton - l'état devrait retourner à "Released" (vert) ✅
5. Répéter pour Bouton 1 et Bouton 2 ✅

---

## [Version 3.28.3] - 2025-12-24

### 🐛 Corrections de Bugs

**Patch de Maintenance :** Correction de l'initialisation de l'encodeur rotatif + Ajout de l'API de monitoring des boutons

### Corrigé

#### 1. Encodeur Rotatif Ne Fonctionnant Qu'Après Reset ✅

**Problème :**
- L'encodeur rotatif ne répondait pas aux rotations ou aux pressions après le démarrage
- Ne fonctionnait qu'après avoir navigué vers la page "Dispositifs d'Entrée" et cliqué sur "Test"
- Rendait l'encodeur rotatif inutilisable pour une utilisation normale

**Cause Racine :**
- `initRotaryEncoder()` n'était JAMAIS appelée pendant le démarrage dans `setup()`
- La fonction n'était appelée que dans `testRotaryEncoder()` qui est déclenchée manuellement via l'interface web
- Les broches GPIO n'étaient pas configurées et les interruptions n'étaient pas attachées au démarrage

**Solution :**
```cpp
// src/main.cpp:5757-5765 - Ajouté dans setup()
// Initialize rotary encoder on startup (v3.28.3 fix)
Serial.println("Initialisation de l'encodeur rotatif...");
initRotaryEncoder();
if (rotaryAvailable) {
  Serial.printf("Encodeur rotatif OK: CLK=%d, DT=%d, SW=%d\r\n",
                rotary_clk_pin, rotary_dt_pin, rotary_sw_pin);
} else {
  Serial.println("Encodeur rotatif: non disponible ou configuration invalide");
}
```

**Impact :**
- ✅ L'encodeur rotatif s'initialise maintenant automatiquement au démarrage
- ✅ La détection de rotation fonctionne immédiatement sans test manuel
- ✅ Les pressions de bouton sont détectées dès la mise sous tension
- ✅ Suivi de position en temps réel disponible via `/api/rotary-position`

#### 2. Monitoring des Boutons Non Fonctionnel ✅

**Problème :**
- Les boutons "Monitor Button" dans l'interface web ne faisaient rien
- Aucun moyen de voir l'état en temps réel des boutons (pressé/relâché)
- Les fonctions JavaScript existaient mais les endpoints API backend manquaient

**Cause Racine :**
- Le code frontend référençait les fonctions de monitoring (`toggleBootButtonMonitoring()`, etc.)
- MAIS aucun endpoint API backend n'existait pour lire l'état des boutons en temps réel
- Route `/api/button-states` manquante

**Solution :**
```cpp
// src/main.cpp:3182-3196 - Ajout des fonctions de lecture d'état des boutons
int getButtonBootState() {
  if (buttonBootPin < 0 || buttonBootPin > 48) return -1;
  return digitalRead(buttonBootPin);
}

int getButton1State() {
  if (button1Pin < 0 || button1Pin > 48) return -1;
  return digitalRead(button1Pin);
}

int getButton2State() {
  if (button2Pin < 0 || button2Pin > 48) return -1;
  return digitalRead(button2Pin);
}

// src/main.cpp:4375-4393 - Ajout du gestionnaire HTTP
void handleButtonStates() {
  int bootState = getButtonBootState();
  int button1State = getButton1State();
  int button2State = getButton2State();

  // LOW = pressé (pull-up), HIGH = relâché
  sendJsonResponse(200, {
    jsonBoolField("boot_pressed", bootState == LOW && bootState != -1),
    jsonBoolField("boot_available", bootState != -1),
    jsonBoolField("button1_pressed", button1State == LOW && button1State != -1),
    jsonBoolField("button1_available", button1State != -1),
    jsonBoolField("button2_pressed", button2State == LOW && button2State != -1),
    jsonBoolField("button2_available", button2State != -1),
    jsonNumberField("boot_pin", buttonBootPin),
    jsonNumberField("button1_pin", button1Pin),
    jsonNumberField("button2_pin", button2Pin)
  });
}

// src/main.cpp:5758-5759 - Enregistrement de la route
server.on("/api/button-states", handleButtonStates);
```

**Impact :**
- ✅ Nouveau endpoint API `/api/button-states` retourne l'état en temps réel des boutons
- ✅ Retourne du JSON avec l'état pressé pour BOOT, Bouton 1 et Bouton 2
- ✅ Inclut les numéros de broches et le statut de disponibilité
- ✅ Le monitoring frontend peut maintenant interroger cet endpoint pour des mises à jour en direct

**Fichiers Modifiés :**
- `src/main.cpp` :
  - Lignes 3182-3196 : Ajout des fonctions de lecture d'état des boutons
  - Lignes 4375-4393 : Ajout du gestionnaire HTTP `handleButtonStates()`
  - Lignes 5757-5765 : Initialisation de l'encodeur rotatif dans `setup()`
  - Ligne 5759 : Enregistrement de la route `/api/button-states`
- `platformio.ini` : Version 3.28.2 → 3.28.3

**Tests :**
1. **Encodeur Rotatif :**
   - Allumer l'ESP32
   - Tourner l'encodeur immédiatement - la position devrait changer ✅
   - Presser le bouton de l'encodeur - devrait être enregistré ✅
   - Naviguer vers "Dispositifs d'Entrée" - encodeur déjà fonctionnel ✅

2. **Monitoring des Boutons :**
   - Naviguer vers la page "Dispositifs d'Entrée"
   - Presser le bouton BOOT (GPIO 0) - retour LED devrait fonctionner ✅
   - Vérifier l'endpoint `/api/button-states` - devrait retourner les états actuels ✅

---

## [Version 3.28.2] - 2025-12-24

### 🐛 Correction Critique

**Patch d'Urgence :** Correction de l'erreur JavaScript BUTTON_BOOT qui n'était PAS réellement corrigée dans 3.28.0/3.28.1

### Corrigé

#### Erreur JavaScript ReferenceError BUTTON_BOOT ✅ (VRAIMENT CORRIGÉ MAINTENANT)

**Problème :**
- L'erreur `ReferenceError: BUTTON_BOOT is not defined` se produisait toujours sur la page Input Devices
- Malgré les tentatives de correction en v3.28.0, l'erreur persistait
- Cause racine mal identifiée dans les versions précédentes

**Cause Racine :**
- Les constantes GPIO (BUTTON_BOOT, BUTTON_1, BUTTON_2, TFT_MISO_PIN) étaient injectées dans `web_interface.h` mais PAS dans `main.cpp:handleJavaScriptRoute()`
- Le JavaScript réel servi au navigateur provient de `handleJavaScriptRoute()`, pas de `web_interface.h:generateJavaScript()`
- `generateJavaScript()` n'est utilisé que pour calculer la taille du JavaScript pour les statistiques
- Par conséquent, les constantes injectées dans `web_interface.h` n'étaient jamais réellement envoyées au navigateur

**Solution :**
```cpp
// src/main.cpp:5397-5405 - Ajouté à handleJavaScriptRoute()
// Button pins
pinVars += ";const BUTTON_BOOT=";
pinVars += String(BUTTON_BOOT);
pinVars += ";const BUTTON_1=";
pinVars += String(BUTTON_1);
pinVars += ";const BUTTON_2=";
pinVars += String(BUTTON_2);
// TFT MISO pin
pinVars += ";const TFT_MISO_PIN=";
pinVars += String(TFT_MISO);
```

**Impact :**
- ✅ La page Input Devices se charge maintenant SANS erreurs JavaScript
- ✅ BUTTON_BOOT s'affiche correctement en lecture seule GPIO 0
- ✅ BUTTON_1 et BUTTON_2 fonctionnent correctement
- ✅ Toutes les constantes GPIO sont maintenant correctement injectées AVANT l'exécution des fonctions JavaScript

**Fichiers Modifiés :**
- `src/main.cpp` (lignes 5397-5415) : Ajout des constantes boutons et TFT MISO à pinVars
- `platformio.ini` : Version 3.28.1 → 3.28.2

**Tests :**
- Naviguer vers la page "Input Devices" - doit se charger sans erreurs ✅
- BUTTON_BOOT doit afficher "GPIO 0 (non configurable)" ✅
- La console du navigateur ne doit afficher aucune ReferenceError ✅

---

## [Version 3.28.1] - 2025-12-24

### 🐛 Corrections Critiques

**Version Corrective :** Intégration backend MISO corrigée + Carte SD fonctionnelle sur ESP32-S3

**NOTE :** L'erreur BUTTON_BOOT n'était PAS entièrement corrigée dans cette version malgré la documentation affirmant le contraire. Voir v3.28.2 pour la vraie correction.

### Corrigé
- **Intégration Backend MISO TFT**:
  - Correction champ MISO manquant dans la réponse JSON `/api/screens-info`
  - Ajout de l'initialisation de la variable `tftMISO` depuis la constante `TFT_MISO`
  - Le backend retourne maintenant correctement le champ `tft.pins.miso` (GPIO 13 pour ESP32-S3)
  - Résout le problème d'affichage "MISO: undefined" dans l'interface web

- **Synchronisation Configuration MISO TFT**:
  - Correction de la fonction JavaScript `configTFT()` qui n'envoyait pas la valeur MISO au backend
  - Le paramètre MISO est maintenant correctement inclus dans la requête `/api/tft-config`
  - Le backend `handleTFTConfig()` accepte et valide maintenant le paramètre MISO
  - Complète le flux de configuration MISO: UI ↔ API ↔ Firmware

- **Support Carte SD sur ESP32-S3**:
  - Correction de l'échec d'initialisation de la carte SD sur ESP32-S3 avec erreurs de compilation/exécution
  - Cause racine: La constante `HSPI` n'est disponible que sur ESP32 classique, pas sur ESP32-S2/S3
  - Implémentation de la sélection conditionnelle du bus SPI:
    - ESP32 classique: `HSPI` (Bus SPI matériel 2)
    - ESP32-S2/S3: `FSPI` (Bus SPI flexible, équivalent à SPI2)
  - Les tests de carte SD sont maintenant pleinement fonctionnels sur ESP32-S3 N16R8

### Détails Techniques
- **Modifications Backend** (`src/main.cpp`):
  - Ligne 261: Ajout de la déclaration de variable `int tftMISO = TFT_MISO;`
  - Ligne 4568: Ajout du champ `miso` au JSON des broches TFT dans `handleScreensInfo()`
  - Lignes 3814-3828: Mise à jour de `handleTFTConfig()` pour accepter et valider le paramètre MISO
  - Lignes 2950-2954: Ajout de la sélection conditionnelle du bus SPI pour l'initialisation de la carte SD
  - La réponse JSON inclut maintenant: `"pins":{"miso":13,"mosi":11,...}`

- **Modifications Frontend** (`include/web_interface.h`):
  - Ligne 119: Mise à jour de `configTFT()` pour récupérer la valeur MISO depuis le champ de saisie
  - Ajouté: `const miso=document.getElementById('tftMISO').value;`
  - L'appel API inclut maintenant: `?miso=${miso}&mosi=${mosi}&...`

### Conformité
- Maintient l'immuabilité de `board_config.h` - toutes les valeurs proviennent des constantes
- Aucune valeur GPIO codée en dur
- Abstraction appropriée du bus SPI pour la compatibilité entre variantes ESP32

### Fichiers Modifiés
- `src/main.cpp`: Variable MISO, réponse JSON, gestionnaire config, bus SPI SD
- `include/web_interface.h`: Paramètre MISO dans configTFT
- `platformio.ini`: Version 3.28.0 → 3.28.1

---

## [Version 3.28.0] - 2025-12-23

### 🚀 Nouvelles Fonctionnalités & Corrections

**Améliorations Majeures:** Correction erreur JavaScript BUTTON_BOOT + Ajout configuration MISO TFT + Nouveaux endpoints API carte SD + Avertissement partage GPIO 13

### Corrigé
- **Erreur JavaScript BUTTON_BOOT**:
  - Correction `ReferenceError: BUTTON_BOOT is not defined`
  - Injection de toutes les constantes de broches manquantes depuis `board_config.h` vers JavaScript
  - Ajouté: `ROTARY_CLK_PIN`, `ROTARY_DT_PIN`, `ROTARY_SW_PIN`, `BUTTON_BOOT`, `BUTTON_1`, `BUTTON_2`
  - Ajouté: `SD_MISO_PIN`, `SD_MOSI_PIN`, `SD_SCLK_PIN`, `SD_CS_PIN`, `TFT_MISO_PIN`
  - Toutes les définitions GPIO correctement sourcées depuis `board_config.h` (contrat immuable)

- **Configuration BUTTON_BOOT**:
  - BUTTON_BOOT rendu non-configurable selon les spécifications
  - Changé d'un input éditable à un affichage en lecture seule
  - Marqué "(non configurable)" dans l'UI pour éviter toute modification accidentelle
  - Préserve l'intégrité du bouton boot natif ESP32

### Ajouté
- **Configuration Broche MISO TFT**:
  - Ajout de la broche MISO à l'affichage des broches SPI: `MISO`, `MOSI`, `SCLK`, `CS`, `DC`, `RST`
  - Ajout d'un champ de saisie MISO configurable dans la section configuration TFT
  - Complète la gestion des broches SPI dans l'interface web
  - Reflète correctement GPIO 13 depuis `board_config.h` pour ESP32-S3

- **Nouveaux Endpoints API Carte SD**:
  - `/api/sd-test-read`: Test des opérations de lecture carte SD
  - `/api/sd-test-write`: Test des opérations d'écriture avec horodatage
  - `/api/sd-format`: Nettoyage des fichiers de test SD (nettoyage sécurisé, pas formatage bas niveau)
  - Format de réponse JSON cohérent avec les endpoints existants
  - Initialisation automatique de la carte SD si indisponible

- **Avertissement Partage GPIO 13**:
  - Ajout d'un avertissement visible dans la section Carte SD (boîte d'avertissement jaune)
  - Alerte les utilisateurs que GPIO 13 est partagé entre TFT et SD (ligne MISO)
  - Souligne le besoin d'un câblage SPI strictement conforme et d'une gestion logicielle adaptée
  - Nouvelles clés i18n: `gpio_shared_warning`, `gpio_13_shared_desc` (EN/FR)

### Détails Techniques
- **Interface Web** (`include/web_interface.h`):
  - L'injection de broches inclut maintenant toutes les broches ROTARY, BUTTON, SD et TFT
  - Bouton BOOT affiché en lecture seule avec indicateur visuel
  - Avertissement GPIO 13 stylisé avec style d'alerte type Bootstrap
  - Support i18n complet maintenu pour toutes les nouvelles fonctionnalités

- **Implémentation API** (`src/main.cpp`):
  - `handleSDTestRead()`: Crée un fichier de test si nécessaire, teste la capacité de lecture
  - `handleSDTestWrite()`: Teste la capacité d'écriture avec données horodatées
  - `handleSDFormat()`: Supprime tous les fichiers de test (`/test_*.txt`)
  - Gestion d'erreur appropriée pour les cartes SD indisponibles

- **Traductions** (`include/languages.h`):
  - `gpio_shared_warning`: "Shared GPIO 13 (TFT + SD – MISO)" / "GPIO 13 partagé (TFT + SD – MISO)"
  - `gpio_13_shared_desc`: Explication complète en EN/FR sur les exigences de partage SPI

### Conformité
- Toutes les définitions GPIO sourcées depuis `board_config.h` (contrat immuable)
- Aucune valeur de broche codée en dur dans JavaScript
- Respecte `board_config.h` comme source unique de vérité
- Aucune modification de `board_config.h` lui-même (comme requis)

### Fichiers Modifiés
- `include/web_interface.h`: Injection broches, affichage bouton BOOT, avertissement GPIO, champ MISO
- `include/languages.h`: Ajout de 2 nouvelles clés de traduction pour l'avertissement GPIO
- `src/main.cpp`: Ajout de 3 nouveaux gestionnaires d'endpoints carte SD + enregistrement routes
- `platformio.ini`: Version 3.27.2 → 3.28.0

---

## [Version 3.27.2] - 2025-12-23

### 🔧 Corrections & Améliorations

**Corrections de Bugs & Nouvelles Fonctionnalités:** Correction du monitoring du bouton HW-040 + Ajout de 3 moniteurs de boutons matériels (BOOT, BOUTON1, BOUTON2).

### Corrigé
- **Monitoring Encodeur Rotatif HW-040** :
  - Correction de l'état du bouton qui ne revenait pas à "Relâché" après être "Appuyé"
  - Remplacement du texte en dur par traductions i18n (`button_pressed`, `button_released`)
  - Le monitoring alterne maintenant correctement entre les labels "Surveiller" et "Arrêter"
  - Les mises à jour d'état en temps réel utilisent maintenant les fonctions de traduction

### Ajouté
- **Monitoring des Boutons Matériels** (3 nouvelles cartouches dans le menu Périphériques) :
  - **Bouton BOOT** (GPIO 0) : Monitoring du bouton boot intégré avec pin configurable
  - **Bouton Utilisateur 1** (GPIO 38/5) : Bouton programmable avec pull-up interne
  - **Bouton Utilisateur 2** (GPIO 39/12) : Bouton programmable avec pull-up interne
- **Nouvelles Clés i18n** (12 ajouts) :
  - `button_boot`, `button_boot_desc`, `button_1`, `button_1_desc`, `button_2`, `button_2_desc`
  - `button_pin`, `button_state`, `button_pressed`, `button_released`
  - `monitor_button`, `stop_monitoring`
- **Fonctions JavaScript** :
  - `toggleBootButtonMonitoring()` : Surveillance en temps réel du bouton BOOT
  - `toggleButton1Monitoring()` : Surveillance en temps réel du Bouton 1
  - `toggleButton2Monitoring()` : Surveillance en temps réel du Bouton 2
  - `applyButtonConfig(buttonId)` : Configuration des pins GPIO des boutons via API

### Détails Techniques
- Chaque cartouche de bouton inclut :
  - Configuration de pin avec validation (min=0, max=48)
  - Affichage d'état en temps réel (code couleur : vert=Relâché, rouge=Appuyé)
  - Bouton de basculement de surveillance (intervalle de polling 100ms)
  - Intégration API : `/api/button-state?button=<boot|1|2>` et `/api/button-config`
- Pins des boutons depuis `board_config.h` : `BUTTON_BOOT`, `BUTTON_1`, `BUTTON_2`
- Entièrement bilingue (anglais/français) avec intégration i18n complète

### Fichiers Modifiés
- `include/languages.h` : Ajout de 12 nouvelles clés de traduction
- `include/web_interface.h` :
  - Correction de `toggleRotaryMonitoring()` pour utiliser les fonctions `tr()`
  - Ajout de 3 fonctions de monitoring de boutons
  - Mise à jour de `buildInputDevices()` avec 3 cartouches de boutons
- `platformio.ini` : Version 3.27.1 → 3.27.2

---

## [Version 3.27.0] - 2025-12-23

### ✨ Réorganisation de l'interface et Internationalisation

**Refonte Majeure :** Internationalisation complète des fonctionnalités Carte SD et Encodeur Rotatif + nouvelle structure de menus.

### Ajouts
- **Nouveaux Menus de Navigation** :
  - Menu "Périphériques" pour boutons, encodeurs et contrôles d'entrée utilisateur
  - Menu "Mémoire" pour carte SD, Flash, SRAM et informations PSRAM
  - Organisation améliorée : Aperçu → Affichage & Signal → Capteurs → Périphériques → Mémoire → Tests Matériel → Sans fil → Performances → Export
- **Internationalisation Complète** (45+ nouvelles clés de traduction) :
  - Carte SD : `sd_card`, `sd_card_desc`, `sd_pins_spi`, `sd_pin_miso/mosi/sclk/cs`, `test_sd`, etc.
  - Encodeur rotatif : `rotary_encoder`, `rotary_encoder_desc`, `rotary_pins`, `rotary_position`, `rotary_button`, etc.
  - Section mémoire : `memory_section`, `memory_intro`, `internal_sram`, `psram_external`, `flash_type/speed`
  - Périphériques d'entrée : `input_devices_section`, `input_devices_intro`
- **Support Multilingue** : Toutes les nouvelles fonctionnalités supportent les traductions anglais/français via `languages.h`

### Modifié
- Remplacement de tout le texte français en dur par des variables i18n
- Boutons de menu réorganisés avec nouvelles entrées `nav_input_devices` et `nav_memory`
- Infrastructure préparée pour catégorisation logique du matériel

### Limitations Connues
- Les menus Périphériques et Mémoire affichent actuellement du contenu temporaire
- Implémentation complète de `buildMemory()` et `buildInputDevices()` en attente
- Carte SD et Encodeur Rotatif toujours dans le menu Capteurs (migration en attente)

---

## [Version 3.26.4] - 2025-12-23

### Corrigé
- **Largeur des Champs d'Entrée** : Augmentation de la largeur des champs GPIO de 50px à 70px pour carte SD et encodeur rotatif
  - Corrige le problème d'affichage où seul 1 chiffre était visible
  - Permet la saisie et l'affichage corrects des numéros GPIO à 2 chiffres (0-48)
  - Affecte toutes les pins SD (MISO, MOSI, SCLK, CS) et pins rotatif (CLK, DT, SW)

---

## [Version 3.26.3] - 2025-12-23

### Corrigé
- **Support ESP32 Classique** : Ajout des defines GPIO manquants pour carte SD pour `TARGET_ESP32_CLASSIC` dans `board_config.h`
  - `SD_MISO = 19`, `SD_MOSI = 23`, `SD_SCLK = 18`, `SD_CS = 5`
  - Corrige les valeurs GPIO incorrectes (1,1,1,1) affichées sur les cartes ESP32 classiques
  - Ajout des attributs `min="0" max="48"` à toutes les entrées GPIO carte SD et encodeur rotatif dans l'interface web
- **Configuration GPIO** : Support maintenant de la plage GPIO complète (0-48) pour toutes les variantes ESP32

---

## [Version 3.26.2] - 2025-12-23

### Ajouté
- **Sortie Console de Débogage** : Ajout de console.log dans le navigateur pour vérifier l'injection des pins GPIO
  - Affiche toutes les valeurs GPIO carte SD et encodeur rotatif : `{SD_MISO: 14, SD_MOSI: 11, ...}`
  - Aide à vérifier que les valeurs de `board_config.h` sont correctement injectées dans JavaScript
  - Utile pour dépanner les problèmes de configuration GPIO

---

## [Version 3.26.1] - 2025-12-23

### Corrigé
- **Injection GPIO Dynamique** : Correction des valeurs GPIO carte SD et encodeur rotatif non injectées dynamiquement
  - Ajout des variables JavaScript `SD_MISO_PIN`, `SD_MOSI_PIN`, `SD_SCLK_PIN`, `SD_CS_PIN` dans `/api/pin-vars`
  - Ajout des variables JavaScript `ROTARY_CLK_PIN`, `ROTARY_DT_PIN`, `ROTARY_SW_PIN`
  - Remplacement des valeurs en dur dans l'interface web par des variables dynamiques
  - Les valeurs GPIO s'initialisent maintenant correctement depuis les defines de `board_config.h`

---

## [Version 3.26.0] - 2025-12-23

### ✨ Nouvelles Fonctionnalités : Support Carte SD et Encodeur Rotatif HW-040

**Ajout Majeur :** Intégration complète du lecteur de carte SD et de l'encodeur rotatif HW-040 avec configuration GPIO dynamique.

### Ajouts - Support Carte SD
- **Gestion de Carte SD** : Initialisation et tests complets basés sur SPI
  - Détection automatique du type de carte (MMC, SDSC, SDHC)
  - Détection de la taille et rapport de capacité
  - Tests de vérification lecture/écriture
  - Support de totalBytes() et usedBytes()
- **Configuration GPIO Dynamique** :
  - `sd_miso_pin`, `sd_mosi_pin`, `sd_sclk_pin`, `sd_cs_pin` (modifiables via interface web)
  - Valeurs par défaut depuis `board_config.h` : `SD_MISO`, `SD_MOSI`, `SD_SCLK`, `SD_CS`
- **Points d'API** :
  - `/api/sd-config` - Configurer les pins de la carte SD
  - `/api/sd-test` - Lancer un test avec vérification lecture/écriture
  - `/api/sd-info` - Obtenir les informations (type, taille, utilisation)
- **Fonctions de Test** :
  - `initSD()` - Initialiser la carte SD avec configuration SPI
  - `testSD()` - Test complet incluant vérification fichier
  - `getSDInfo()` - Récupérer les informations détaillées
  - Support de test asynchrone via `sdTestRunner`

### Ajouts - Support Encodeur Rotatif HW-040
- **Gestion Encodeur Rotatif** : Encodeur basé sur interruptions avec bouton intégré
  - Anti-rebond matériel pour rotation (5ms) et bouton (50ms)
  - Suivi de position avec détection incrémentation/décrémentation
  - Détection d'appui bouton avec auto-reset
- **Configuration GPIO Dynamique** :
  - `rotary_clk_pin`, `rotary_dt_pin`, `rotary_sw_pin` (modifiables via interface web)
  - Valeurs par défaut depuis `board_config.h` : `ROTARY_CLK`, `ROTARY_DT`, `ROTARY_SW`
- **Implémentation ISR** :
  - `rotaryISR()` - Gestionnaire d'interruption IRAM pour détection rotation
  - `rotaryButtonISR()` - Gestionnaire d'interruption IRAM pour appuis bouton
  - Encodage en quadrature pour suivi précis de position
- **Points d'API** :
  - `/api/rotary-config` - Configurer les pins de l'encodeur
  - `/api/rotary-test` - Lancer un test interactif de 5 secondes
  - `/api/rotary-position` - Obtenir position actuelle et état bouton
  - `/api/rotary-reset` - Réinitialiser le compteur de position à zéro
- **Fonctions de Test** :
  - `initRotaryEncoder()` - Initialiser avec attachement d'interruptions
  - `testRotaryEncoder()` - Test interactif de 5 secondes
  - `getRotaryPosition()`, `getRotaryButtonState()`, `resetRotaryPosition()`
  - Support de test asynchrone via `rotaryTestRunner`

### Modifications - board_config.h
- **ESP32-S3 DevKitC-1 N16R8** :
  - Pins carte SD : MISO=14, MOSI=11, SCLK=12, CS=1
  - Pins encodeur rotatif : CLK=47, DT=45, SW=40
- **ESP32 Classic DevKitC** :
  - Pins encodeur rotatif : CLK=4, DT=13, SW=26
  - (La carte SD partage les pins avec le TFT sur la variante classique)

### Modifications - Exports
- **Export TXT** : Ajout des résultats de tests carte SD et encodeur rotatif
- **Export JSON** : Ajout des champs `sd_card` et `rotary_encoder` dans `hardware_tests`
- **Export CSV** : Ajout des lignes de résultats pour carte SD et encodeur rotatif

### Technique
- **Includes Ajoutés** : `<SPI.h>`, `<SD.h>`, `<FS.h>` pour le support carte SD
- **Variables Globales** :
  - SD : `sdTestResult`, `sdAvailable`, `sdCardSize`, `sdCardType`, `sdCardTypeStr`
  - Encodeur : `rotaryPosition` (volatile), `rotaryButtonPressed` (volatile), timestamps d'interruption
- **Allocation de Pile** :
  - Test SD : 6144 octets (opérations I/O fichier)
  - Test encodeur : 4096 octets (boucle de test interactive)
- **Notes Matérielles** :
  - Carte SD nécessite câblage SPI approprié, partage le bus avec TFT sur ESP32-S3
  - Encodeur Rotatif HW-040 : condensateurs 10nF recommandés pour anti-rebond matériel
  - Toutes les pins utilisent des niveaux logiques 3.3V

### Architecture
- Suit le pattern existant de remapping GPIO dynamique (variables en minuscules)
- Exécuteurs de tests asynchrones pour interface web non-bloquante
- Format de réponse API cohérent avec champs `success`, `result`, `available`
- Fonctions ISR marquées avec `IRAM_ATTR` pour sécurité d'interruption

---

## [Version 3.25.1] - 2025-12-22

### Modifié
- Passage en version 3.25.1 : mise à jour du numéro de version dans platformio.ini et les fichiers de documentation.
- Maintenance mineure et alignement documentaire pour le processus de release.

## [Version 3.25.0] - 2025-12-22

### ✅ Restauration de Fonctionnalité : Remapping Dynamique des GPIO via l'UI Web

**Changement Majeur :** Réintroduction du remapping runtime des broches avec une architecture améliorée qui évite les conflits de préprocesseur.

### Ajouté
- **Variables Runtime de Broches en Minuscules** : Nouvelle architecture utilisant des noms en minuscules pour éviter les conflits de préprocesseur
  - Exemple : `int i2c_sda = I2C_SDA;` (variable runtime) initialisée depuis `#define I2C_SDA 15` (constante de compilation)
  - Noms en minuscules (`i2c_sda`, `rgb_led_pin_r`, etc.) empêchent l'expansion de macros
  - Variables déclarées dans `src/main.cpp:201-216`
  - Déclarations externes dans `include/web_interface.h:24-35`

### Restauré
- **Remapping Dynamique des Broches** : L'UI Web peut maintenant modifier les broches GPIO à l'exécution (fonctionnalité restaurée de v3.23.x)
  - `handleOLEDConfig()` - Remapping broches I2C pour OLED/capteurs
  - `handleRGBLedConfig()` - Remapping broches LED RGB
  - `handleBuzzerConfig()` - Remapping broche buzzer
  - `handleDHTConfig()` - Remapping broche capteur DHT
  - `handleLightSensorConfig()` - Remapping broche capteur de lumière
  - `handleDistanceSensorConfig()` - Remapping broches capteur de distance
  - `handleMotionSensorConfig()` - Remapping broche capteur de mouvement

### Modifié
- **Toutes les Références aux Broches GPIO** : Remplacement systématique des defines MAJUSCULES par des variables runtime minuscules dans tout le code
  - `src/main.cpp` : ~100+ références mises à jour dans les fonctions de test, handlers et injection JavaScript
  - `src/environmental_sensors.cpp` : Références broches I2C mises à jour
  - Maintient les defines de compilation dans `board_config.h` (MAJUSCULES) comme valeurs initiales

### Architecture
**Système GPIO à Deux Couches Restauré (avec nommage amélioré)** :
1. **Valeurs par défaut compile-time** (`board_config.h`) : `#define I2C_SDA 15` (MAJUSCULES)
2. **Variables runtime** (`main.cpp`) : `int i2c_sda = I2C_SDA;` (minuscules)
3. **Amélioration Clé** : Conventions de nommage différentes empêchent les conflits de préprocesseur

### Avantages
- ✅ **Remapping dynamique fonctionne** : Les utilisateurs peuvent changer les broches via l'UI Web sans recompilation
- ✅ **Pas de conflits préprocesseur** : Les variables runtime en minuscules ne déclenchent pas l'expansion de macros
- ✅ **Architecture plus claire** : Convention de nommage distingue clairement compile-time vs runtime
- ✅ **Toutes fonctionnalités préservées** : Aucune fonctionnalité perdue par rapport à v3.23.x

### Technique
- **Rétrocompatibilité** : ⚠️ Nécessite une mise à jour du firmware depuis v3.24.0
- **Matériel** : Aucun changement matériel requis
- **Fichiers Modifiés** :
  - `src/main.cpp` : Ajout variables runtime, restauration handlers, mise à jour toutes références broches
  - `include/web_interface.h` : Ajout déclarations extern pour variables runtime
  - `src/environmental_sensors.cpp` : Mise à jour pour utiliser variables runtime minuscules
  - `platformio.ini` : Passage version à 3.25.0

---

## [Version 3.24.0] - 2025-12-22 (ANNULÉE)

### 🔄 Changement Architectural Majeur : Système de Broches GPIO Simplifié (ANNULÉ DANS v3.25.0)

**Changement Non-Rétrocompatible :** Les broches GPIO étaient des constantes de compilation. Le remapping dynamique via l'UI Web avait été supprimé.

### Modifié
- **Architecture GPIO Simplifiée** : Suppression du système à deux couches
  - Élimination du préfixe `DEFAULT_` de tous les noms de broches GPIO dans `board_config.h`
  - Suppression des variables runtime de broches dans `main.cpp` (lignes 198-217)
  - Les broches GPIO sont maintenant accessibles directement comme constantes `#define`
  - Exemple : `RGB_LED_PIN_R` au lieu de `DEFAULT_RGB_LED_PIN_R` + `int RGB_LED_PIN_R`

- **Comportement de l'Interface Web** :
  - Les handlers de configuration de broches ignorent maintenant les changements (assignations commentées)
  - L'UI Web affiche les broches actuelles pour référence seulement
  - Pour changer les broches, les utilisateurs doivent éditer `board_config.h` et recompiler

- **Modifications du Code** :
  - `src/main.cpp` : Suppression des déclarations de variables de broches, mise à jour des handlers
  - `include/web_interface.h` : Suppression des déclarations `extern` de broches
  - `src/environmental_sensors.cpp` : Suppression des déclarations `extern`, utilise les defines directement

### Supprimé
- **Remapping de Broches à Runtime** : L'UI Web ne peut plus modifier les broches GPIO à l'exécution
- **Préfixe `DEFAULT_`** : Toutes les broches GPIO utilisent maintenant des noms directs (ex. `I2C_SDA` et non `DEFAULT_I2C_SDA`)
- **Variables Runtime** : Plus de pattern `int I2C_SDA = DEFAULT_I2C_SDA;`

### Documentation
- **Mise à jour `docs/PIN_POLICY.md`** : Reflète la nouvelle architecture à constantes de compilation
- **Mise à jour `docs/PIN_POLICY_FR.md`** : Documentation française mise à jour
- Suppression des références au remapping runtime et au préfixe `DEFAULT_`

### Avantages
- ✅ **Code plus simple** : Le système GPIO à une couche est plus facile à comprendre
- ✅ **Meilleures performances** : Le compilateur peut optimiser l'accès aux broches constantes
- ✅ **Intention plus claire** : Les assignations de broches sont fixées à la compilation
- ✅ **Pas de conflits préprocesseur** : Élimine les problèmes de collision de noms

### Guide de Migration
**Pour les Utilisateurs :**
- Les changements de broches nécessitent maintenant d'éditer `board_config.h` et de recompiler
- Aucun changement fonctionnel si vous utilisez les assignations de broches par défaut

**Pour les Développeurs :**
- Remplacer `DEFAULT_NOM_GPIO` par `NOM_GPIO` dans `board_config.h`
- Supprimer les déclarations de variables runtime
- Accéder aux broches directement via les defines

### Technique
- **Rétrocompatibilité** : ⚠️ Changement non-rétrocompatible - nécessite une mise à jour du firmware
- **Matériel** : Aucun changement matériel requis
- **Fichiers Modifiés** :
  - `src/main.cpp` : Suppression variables de broches, mise à jour handlers
  - `include/web_interface.h` : Suppression déclarations extern
  - `src/environmental_sensors.cpp` : Utilisation directe des defines
  - `include/board_config.h` : Suppression préfixes `DEFAULT_` (déjà fait par l'utilisateur)
  - `docs/PIN_POLICY.md`, `docs/PIN_POLICY_FR.md` : Mises à jour documentation
  - `platformio.ini` : Passage version à 3.24.0

---

## [Version 3.23.2] - 2025-12-22 (OBSOLÈTE)

### Corrigé
- **Initialisation I2C des Capteurs Environnementaux** : Correction des références aux broches I2C dans les capteurs environnementaux
  - Correction de `environmental_sensors.cpp:56-58` pour utiliser les variables runtime `I2C_SDA` et `I2C_SCL`
  - Référençait précédemment `DEFAULT_I2C_SDA` et `DEFAULT_I2C_SCL` directement (defines de compilation)
  - Ajout de déclarations `extern` pour accéder aux variables runtime depuis `main.cpp`
  - Respecte désormais la configuration dynamique des broches I2C via l'UI Web

### Technique
- **Fichier Modifié** : `src/environmental_sensors.cpp:56-58`
- **Note d'Architecture** : Les variables runtime (`int I2C_SDA`) et les defines de compilation (`#define DEFAULT_I2C_SDA`)
  doivent coexister pour que le remapping dynamique via l'UI Web fonctionne. Retirer le préfixe `DEFAULT_` crée des conflits de préprocesseur.
- **Impact** : Assure que les capteurs environnementaux (AHT20, BMP280) utilisent les bonnes broches I2C lorsqu'elles sont remappées
- **Rétrocompatibilité** : ✅ Entièrement compatible avec v3.23.1

---

## [Version 3.23.1] - 2025-12-22

### Corrigé
- **Affichage Pin Buzzer dans l'UI Web** : Correction de l'initialisation du champ de la broche buzzer
  - Affichait précédemment la valeur de `PWM_PIN` au lieu de `BUZZER_PIN` dans l'interface web
  - Fonction affectée : `buildDisplaySignal()` dans `web_interface.h:85`
  - Affiche désormais correctement la valeur de BUZZER_PIN (ESP32-S3: GPIO 6, ESP32 Classic: GPIO 19)
  - PWM et Buzzer sont des broches distinctes comme défini dans `board_config.h`

### Technique
- **Fichier Modifié** : `include/web_interface.h:85`
- **Impact** : Correction visuelle uniquement - comportement runtime inchangé (backend utilisait déjà la bonne broche)
- **Rétrocompatibilité** : ✅ Entièrement compatible avec v3.23.0

---

## [Version 3.23.0] - 2025-12-22

### Ajouté
- **Variable PWM_PIN** : Ajout de la variable runtime `PWM_PIN` manquante dans `main.cpp`
  - Auparavant, `PWM_PIN` était déclaré comme `extern` dans `web_interface.h` mais non défini
  - Désormais correctement initialisé depuis `DEFAULT_PWM_PIN` dans `board_config.h`
  - ESP32-S3 : PWM sur GPIO 20, Buzzer sur GPIO 6
  - ESP32 Classic : PWM sur GPIO 4, Buzzer sur GPIO 19
- **Documentation Politique des Broches** : Nouveaux guides complets pour la gestion GPIO
  - `docs/PIN_POLICY.md` (Anglais) - Politique complète de mapping des broches pour développeurs
  - `docs/PIN_POLICY_FR.md` (Français) - Guide détaillé de la politique de mapping GPIO
  - Explique le principe de "source unique de vérité" (`board_config.h`)
  - Inclut les considérations de sécurité, conventions de nommage et exemples pratiques

### Modifié
- **Injection JavaScript** : Correction de l'injection de PWM_PIN et BUZZER_PIN dans l'UI Web
  - Les deux broches sont désormais correctement injectées dans les constantes JavaScript
  - Auparavant, `PWM_PIN` se voyait incorrectement attribuer la valeur de `BUZZER_PIN`
  - Fichiers affectés : `main.cpp:4812-4815`, `web_interface.h:456-459`
- **Unification NEOPIXEL_PIN** : Élimination de la redéfinition `DEFAULT_NEOPIXEL_PIN`
  - Suppression de la définition dupliquée dans `config.h` et `config-example.h`
  - Utilise désormais `NEOPIXEL_PIN` directement depuis `board_config.h` (GPIO 48 pour ESP32-S3)
  - Commentaires ajoutés pour clarifier que `NEOPIXEL_PIN` est défini dans `board_config.h`

### Corrigé
- **Cohérence du Mapping des Broches** : Toutes les références GPIO utilisent exclusivement `board_config.h`
  - Élimination de l'ambiguïté entre `DEFAULT_NEOPIXEL_PIN` et `NEOPIXEL_PIN`
  - Séparation correcte de `PWM_PIN` et `BUZZER_PIN` (ce sont des broches distinctes)
  - Amélioration des commentaires des variables de broches runtime pour référencer `board_config.h` comme source

### Technique
- **Rétrocompatibilité** : ✅ Entièrement compatible avec v3.22.1
  - Aucun changement matériel requis
  - L'UI Web affiche désormais correctement les broches PWM et Buzzer
  - Toutes les fonctionnalités existantes préservées

### Documentation
- Nouveau guide développeur expliquant l'architecture du mapping GPIO
- Clarifie la différence entre `PIN_*` (fixe) et `DEFAULT_*` (configurable à l'exécution)
- Fournit des exemples étape par étape pour ajouter de nouveaux capteurs
- Disponible en anglais et français

---

## [Version 3.22.1] - 2025-12-12

### Corrigé — Doublons de mapping (ESP32 Classic)
1. Boutons corrigés: `BTN1` passé de 32 → 2, `BTN2` de 33 → 5 (pull-up interne)
2. LED RGB alignées: `R=13`, `V=26` (au lieu de 14), `B=33` (au lieu de 25)
3. Capteurs harmonisés:
   - `DHT` 32 → 15
   - `HC-SR04` TRIG 27 → 12, ECHO 33 → 35
   - `PWM` sur 4 ; `Buzzer` sur 19

Référence: `include/board_config.h` est la source unique de vérité pour le mapping des broches.

### Changé - Pin Mapping ESP32-S3 (v3.22.0)
- **LED RGB repositionnées** : LED verte de GPIO 45 → 41, LED bleue de GPIO 47 → 42
- **Justification** : Libération des broches de strapping GPIO 45/47 pour éviter les conflits au démarrage
- **LED rouge inchangée** : GPIO 21 maintenu (broche sécurisée)
- **Impact ESP32-S3** : Recâblage matériel requis pour LED verte et bleue
- **ESP32 Classic** : Aucun changement (configuration optimisée en v3.21.0)

### Ajouté - Documentation
- Avertissement de remapping ajouté à tous les documents utilisateur
- Tables de pin mapping mises à jour dans PIN_MAPPING_FR.md et PIN_MAPPING.md
- Notes de version v3.22.0 créées (EN/FR)

## [Version 3.21.1] - 2025-12-11

### Ajouté
- **Indicateur d'état Wi-Fi NeoPixel** : Rétroaction visuelle en temps réel sur LED RGB NeoPixel/WS2812
  - **Initialisation automatique** lors de la séquence de démarrage
  - **Jaune (50, 50, 0)** : Tentative de connexion en cours lors du démarrage
  - **Battement vert** (0, 50, 0) / (0, 10, 0) : Connecté avec succès au Wi-Fi
  - **Battement rouge** (50, 0, 0) / (10, 0, 0) : Wi-Fi déconnecté
  - **Fréquence de battement** : 1 Hz (alterne tous les 1 seconde)
  - **Fonctionnement non bloquant** : Géré dans la boucle principale, aucun impact sur la réactivité

- **Confirmation de redémarrage du bouton BOOT** : Flash LED violet lors de l'appui long
  - **Flash violet (255, 0, 255)** s'affiche immédiatement lors du maintien du BOOT pendant 2+ secondes
  - Fournit une confirmation visuelle claire de la demande de redémarrage
  - Synchronisé avec la barre de progression TFT

- **Isolation des tests** : Le statut Wi-Fi NeoPixel se met en pause lors des tests matériels
  - Le battement se met automatiquement en pause lors de l'exécution de `/api/neopixel-test`
  - Le battement se met automatiquement en pause lors de l'exécution de `/api/neopixel-pattern`
  - Le battement se met automatiquement en pause lors des changements de couleur via `/api/neopixel-color`
  - L'état se restaure automatiquement à la fin du test/motif

### Modifié
- **main.cpp** : Ajout des fonctions de gestion d'état Wi-Fi NeoPixel
  - Nouvelles fonctions : `updateNeoPixelWifiStatus()`, `neopixelSetWifiState()`, `neopixelShowConnecting()`, `neopixelPauseStatus()`, `neopixelResumeStatus()`, `neopixelRestoreWifiStatus()`, `neopixelShowRebootFlash()`
  - Modifié `setup()` : Initialiser NeoPixel avant la connexion Wi-Fi
  - Modifié `loop()` : Ajouter l'appel de mise à jour du battement
  - Modifié `onButtonBootLongPress()` : Ajouter la confirmation du flash de redémarrage
  - Modifié les gestionnaires de test NeoPixel : Ajouter pause/reprendre autour des tests

### Technique
- **GPIO** : Aucun changement - utilise la configuration GPIO NeoPixel existante (GPIO 48 ESP32-S3, GPIO 2 ESP32 Classic)
- **Timing** : Intervalle de mise à jour du battement de 1 Hz, implémentation non bloquante
- **Mémoire** : Surcharge minimale - 7 variables d'état global, aucune allocation dynamique dans le chemin du battement

### Compatibilité rétroactive
- **✅ Entièrement compatible** avec v3.21.0 - aucune modification matérielle requise
- **✅ Pas de changements cassants** - fonctionnalité purement additive
- **✅ Pas de changements de configuration** - GPIO NeoPixel inchangé depuis v3.21.0

---

## [Version 3.21.0] - 2025-12-09

### ⚠️ BREAKING CHANGE - Migration matérielle requise pour ESP32 Classic
- **Révision complète du pin mapping ESP32 Classic** : 11 modifications pour résoudre les problèmes de boot et de communication USB
  - **Problèmes de boot résolus** : Élimination des LED et périphériques sur les broches de strapping GPIO 4, 12, 15
  - **Stabilité USB-UART** : Protection des GPIO 1 (TX0) et GPIO 3 (RX0) contre les interférences lors du flashing
  - **Boutons améliorés** : Migration vers GPIO 32/33 avec pull-up interne (au lieu de 34/35 input-only sans pull-up)

### Détails des 11 modifications (ESP32 Classic uniquement)
1. **GPS PPS** : GPIO 4 → GPIO 36 (GPIO4 = strapping SDIO boot)
2. **TFT CS** : GPIO 19 → GPIO 27 (éviter interférences USB-UART)
3. **TFT DC** : GPIO 27 → GPIO 14 (réorganisation câblage)
4. **TFT RST** : GPIO 26 → GPIO 25 (meilleur groupement physique)
5. **TFT BL** : GPIO 13 → GPIO 32 (éviter conflit LED interne)
6. **LED RGB Rouge** : GPIO 12 → GPIO 13 (GPIO12 = strapping tension flash)
7. **LED RGB Bleue** : GPIO 15 → GPIO 25 (GPIO15 = strapping JTAG debug)
8. **Bouton 1** : GPIO 34 → GPIO 32 (GPIO34 = input-only, pas de pull-up)
9. **Distance TRIG** : GPIO 32 → GPIO 27 (réaffectation GPIO32)
10. **DHT** : GPIO 25 → GPIO 32 (réaffectation GPIO25)
11. **Capteur Mouvement** : GPIO 36 supprimé (réaffecté au GPS PPS)

### Ajouté
- **Section de sécurité** dans `board_config.h` : Rappels détaillés sur les tensions (3.3V), broches de strapping, GPIO input-only (34/35/36/39), protection UART0, limites de courant (≤12 mA), recommandations pull-up I2C (4.7 kΩ)
- **Documentation complète** : Nouveau fichier `docs/PIN_MAPPING_CHANGES_FR.md` détaillant chaque modification avec numérotation, raisons techniques et tableau récapitulatif

### Modifié
- **Tous les pins ESP32 Classic** mis à jour dans `board_config.h`, `PIN_MAPPING_FR.md`, `PIN_MAPPING.md`
- **Documentation utilisateurs** : CONFIG, FEATURE_MATRIX, README actualisés avec les nouveaux pins
- **Historique des versions** : Entrée v3.21.0 ajoutée à tous les documents de référence

### Impact utilisateurs
- **ESP32-S3** : Aucun changement, mapping inchangé
- **ESP32 Classic** : Recâblage matériel obligatoire selon le nouveau mapping (voir tableau dans `docs/PIN_MAPPING_CHANGES_FR.md`)
- **Configuration dynamique** : L'interface web permet toujours de modifier certains pins de capteurs sans recompiler

---

## [Version 3.20.4] - 2025-12-08

### Modifié
- **Refactorisation du nom de projet :** Élimination des chaînes de nom de projet codées en dur dans tout le code
- Toutes les occurrences de "ESP32 Diagnostic" remplacées par la macro `PROJECT_NAME` de platformio.ini
- Fichiers affectés : `main.cpp` (5 emplacements : affichage TFT, enregistrement service mDNS, affichage OLED, pied de page HTML, préambule JavaScript), `languages.h` (titre), `web_interface.h` (log console JavaScript)
- Le nom du projet est maintenant défini de manière centralisée dans la configuration de build pour une meilleure maintenabilité
- Permet une personnalisation facile du projet via une seule modification de flag de build

### Technique
- Aucun changement de fonctionnalité - toutes les fonctionnalités et comportements restent identiques
- Amélioration de la maintenabilité du code grâce à la configuration centralisée
- Réduction de la dette technique due aux littéraux de chaînes dispersés dans plusieurs fichiers

## [Version 3.20.3] - 2025-12-08

### Modifié
- **Optimisation du Code :** Application de 9 optimisations systématiques ([OPT-001] à [OPT-009]) pour l'efficacité mémoire
- Élimination de 90+ allocations de chaînes via des approches unifiées basées sur des buffers
- Formatage de chaîne de version : basé sur snprintf (1 vs 11 allocations)
- Formatage du temps de fonctionnement : approche buffer (1 vs 4-6 allocations)
- Construction de liste GPIO : optimisée à O(1) allocations
- Fonctionnalités du chip : opérations de sous-chaînes éliminées
- Création de constantes réutilisables : DEFAULT_TEST_RESULT_STR, OK_STR, FAIL_STR
- Conversion de 13 messages de debug/statut en formatage buffer snprintf
- Optimisation de l'utilisation de TextField dans formatUptime (3 appels directs .str().c_str())
- Toutes les affectations de résultats de tests utilisent des constantes pré-allouées (30+ emplacements)

### Technique
- Aucun changement de fonctionnalité - pins, tests et fonctionnalités restent identiques
- Compilé avec succès sur ESP32-S3 (esp32s3_n16r8) et ESP32 CLASSIC (esp32devkitc)
- Efficacité mémoire améliorée à l'exécution grâce à la réduction des allocations heap

## [Version 3.20.2] - 2025-12-07

### Modifié
- **Références des Pins dans l'Interface Web :** Les valeurs GPIO codées en dur dans l'interface web sont remplacées par des références dynamiques depuis `board_config.h`
   - Les pins LED RGB (R/G/B) référencent désormais `RGB_LED_PIN_R`, `RGB_LED_PIN_G`, `RGB_LED_PIN_B` au lieu des valeurs codées en dur
   - Le pin du capteur DHT référence désormais `DHT_PIN` au lieu d'une valeur codée en dur
   - Le pin du capteur de lumière référence désormais `LIGHT_SENSOR_PIN` au lieu d'une valeur codée en dur
   - Les pins de détente/écho du capteur de distance référencent désormais `DISTANCE_TRIG_PIN` / `DISTANCE_ECHO_PIN` au lieu de valeurs codées en dur
   - Le pin du capteur de mouvement référence désormais `MOTION_SENSOR_PIN` au lieu d'une valeur codée en dur
   - Le pin du buzzer/PWM référence désormais `PWM_PIN` au lieu d'une valeur codée en dur
- Les valeurs de pin sont injectées comme constantes JavaScript au chargement de la page, garantissant que l'interface affiche toujours les pins compilés corrects pour la cible
- **Version augmentée :** `PROJECT_VERSION` positionné à `3.20.2` dans `platformio.ini`

### Corrigé
- L'interface web affiche désormais correctement les pins GPIO réels en fonction de la cible compilée (ESP32-S3 vs ESP32 CLASSIC)

## [Version 3.20.1] - 2025-12-07

### Corrigé
- **Stabilité USB/OTG (ESP32-S3)** : libération des lignes D-/D+ (GPIO 19/20) en déplaçant l'I2C par défaut sur SDA=15 / SCL=16 et la LED RGB Rouge sur GPIO 21 ; supprime les perturbations du bus USB causées par l'ancien câblage I2C/RGB sur 19/20.

### Modifié
- **Pin mapping ESP32-S3** :
   - I2C : SDA=15, SCL=16 (au lieu de 21/20)
   - RGB : Rouge=21, Vert=45, Bleu=47 (Rouge quitte 19 pour libérer l'USB)
   - Rappel sur la broche de strapping GPIO45
- **Version augmentée** : `PROJECT_VERSION` positionné à `3.20.1` dans `platformio.ini`.

### Documentation
- Guides de mapping (EN/FR), README (EN/FR) et nouvelles release notes alignés sur la résolution OTG et les nouvelles broches.

## [Version 3.20.0] - 2025-12-07

### Ajouté
- **Gestion Avancée des Boutons :** Contrôles de boutons interactifs avec retour visuel
  - Bouton BOOT (GPIO 0) : Appui long (2s) déclenche un redémarrage système avec barre de progression TFT ; relâcher avant 2s efface l'écran
  - Bouton 1 (GPIO 38) : Appui court cycle les couleurs LED RGB (Éteint → Rouge → Vert → Bleu → Blanc)
  - Bouton 2 (GPIO 39) : Appui court déclenche un bip de confirmation
  - Visualisation de progression en temps réel durant les opérations d'appui long sur TFT
  - Antirebond amélioré et détection d'appui long pour une interaction bouton fiable

### Modifié
- **Version augmentée :** `PROJECT_VERSION` positionné à `3.20.0` dans `platformio.ini`
- Gestion des boutons refactorisée avec fonctions séparées pour actions appui long et appui court
- Système de retour visuel amélioré utilisant l'écran TFT pour les interactions utilisateur

### Documentation
- README/README_FR mis à jour avec descriptions des fonctionnalités boutons et exemples d'utilisation

## [Version 3.19.0] - 2025-12-07

### Modifié
- **Pin mapping isolé :** Les définitions GPIO spécifiques carte passent dans `include/board_config.h` ; `config.h` conserve les options communes/runtime. Les boutons ESP32-S3 restent sur GPIO 38/39 pour éviter les conflits d'upload/reset (aucune autre valeur de broche ne change).
- **Renommage des secrets :** Le fichier d'identifiants Wi-Fi devient `secrets.h` (avec `secrets-example.h`) ; les anciens en-têtes `wifi-config` déclenchent désormais une erreur de compilation.
- **Version augmentée :** `PROJECT_VERSION` positionné à `3.19.0` dans `platformio.ini` ; `.gitignore` protège explicitement `include/secrets.h`.

### Documentation
- README/README_FR, guides CONFIG, références de mapping, checklists installation/build, FAQ, dépannage, sécurité, schéma d'architecture et guide d'usage mis à jour pour refléter `board_config.h`, les nouveaux boutons et `secrets.h`.

## [Version 3.18.3] - 2025-12-06

### Corrigé
- **Conflit GPIO 48 ESP32-S3** : Résolution du conflit matériel entre la LED NeoPixel et la LED RGB
  - NeoPixel utilise maintenant exclusivement GPIO 48 (activé, était précédemment désactivé)
  - LED RGB Vert déplacée du GPIO 48 vers GPIO 47
  - LED RGB Bleu reste sur GPIO 45 (inchangé)
  - LED RGB Rouge reste sur GPIO 19 (inchangé)

### Modifié
- **Refactorisation Pin Mapping ESP32-S3** : Réorganisation complète des broches capteurs pour résoudre les conflits
  - Capteur Mouvement (PIR) : GPIO 6 → GPIO 46
  - Capteur Lumière : GPIO 19 → GPIO 4
  - HC-SR04 ECHO : GPIO 19 → GPIO 6
  - NeoPixel : Activé sur GPIO 48, nombre changé de désactivé à 1 LED
  - Inchangés : I2C (SDA=21, SCL=20), Boutons (BTN1=1, BTN2=2), GPS (RX=18, TX=17, PPS=8), broches écran TFT, PWM/Buzzer (14), DHT (5), HC-SR04 TRIG (3)

### Documentation
- Mise à jour de `README.md` et `README_FR.md` avec version 3.18.3 et nouveau résumé des pins
- Mise à jour de `docs/PIN_MAPPING.md` et `docs/PIN_MAPPING_FR.md` avec tableaux complets ESP32-S3
- Synchronisation de `include/config-example.h` avec configuration ESP32-S3 définitive
- Création de `docs/RELEASE_NOTES_3.18.3.md` et `docs/RELEASE_NOTES_3.18.3_FR.md`

## [Version 3.18.2] - 2025-12-06

### Corrigé
- **Clés de traduction manquantes** : Ajout de 4 clés de traduction manquantes pour l’affichage GPS et capteurs environnementaux
  - `gps_status` : Indicateur de statut GPS dans l'interface
  - `temperature_avg` : Label de température moyenne pour les capteurs combinés
  - `pressure_hpa` : Label de mesure de pression avec unité
  - `altitude_calculated` : Altitude calculée depuis la pression barométrique

## [Version 3.18.1] - 2025-12-06

### Corrigé
- **Lecture des données du capteur AHT20** : Correction de l'algorithme d'extraction de bits pour les valeurs d'humidité et de température (les valeurs 20 bits n'étaient pas correctement extraites de la réponse 6 octets)
- **API des capteurs environnementaux** : Correction de la structure JSON pour utiliser un format plat au lieu d'objets imbriqués pour une meilleure compatibilité avec l'interface web
- **Rapport de statut des capteurs** : Amélioration des messages de statut pour indiquer clairement "OK", "Erreur de lecture" ou "Non détecté" pour chaque capteur

### Ajouté
- **Clés de traduction manquantes** : Ajout des clés de traduction FR/EN manquantes pour les éléments d'interface GPS et capteurs environnementaux
  - `refresh_gps`, `gps_module`, `gps_module_desc`
  - `refresh_env_sensors`, `test_env_sensors`

## [Version 3.18.0] - 2025-12-05

### Nouvelles fonctionnalités
1. **Support du module GPS** : Intégration complète du récepteur GPS (NEO-6M/NEO-8M/NEO-M).
   - Parsing de phrases NMEA (RMC, GGA, GSA, GSV)
   - Suivi de la latitude, longitude, altitude, vitesse, cap
   - Nombre de satellites et qualité du signal (HDOP, VDOP, PDOP)
   - Détection du signal PPS (Pulse Per Second) prêt
   - Mises à jour en temps réel et mode test de diagnostic
   - Utilise UART1 avec broches configurables dans `config.h` (ESP32-S3 : RX=18/TX=17/PPS=8)
   - Point d'API `/api/gps` pour diffusion de données en direct
   - Point d'API `/api/gps-test` pour test de diagnostic

2. **Support des capteurs environnementaux** : Intégration complète AHT20 (Temp/Humidité) + BMP280 (Pression).
   - AHT20 : Lectures de température (±0,5°C) et d'humidité (±3% RH)
   - BMP280 : Pression atmosphérique (±1 hPa) avec capteur de température intégré
   - Calcul d'altitude à partir des mesures de pression
   - Détection automatique des capteurs et moyenne double-capteur
   - Utilise l'interface I2C avec broches configurables dans `config.h` (SDA/SCL)
   - Support des deux adresses AHT20 (0x38) et BMP280 (0x76/0x77)
   - Point d'API `/api/environmental-sensors` pour diffusion de données en direct
   - Point d'API `/api/environmental-test` pour test de diagnostic

3. **Mises à jour de l'interface Web** :
   - Cartouche de données GPS dans la section sans fil avec coordonnées actuelles, altitude, satellites, qualité fix
   - Cartouche de capteurs environnementaux sous la section capteur DHT existante
   - Actualisation des données en temps réel avec mises à jour de statut automatiques
   - Gestion complète des erreurs et détection de disponibilité des capteurs

### Améliorations
4. Architecture de module de capteur améliorée pour faciliter l'ajout de futurs capteurs
5. Implémentations de pilotes I2C et UART complètes avec gestion des erreurs
6. Ajout de 24 nouvelles clés de traduction pour les éléments d'interface GPS et capteurs environnementaux (FR/EN)
7. Amélioration de la détection des périphériques et de la génération de rapports de capacités

### Détails techniques
8. Nouveaux fichiers d'en-tête : `gps_module.h`, `environmental_sensors.h`
9. Nouveaux fichiers d'implémentation : `gps_module.cpp`, `environmental_sensors.cpp`
10. Nouveaux points d'API dans `main.cpp` pour données GPS et capteurs environnementaux
11. Dictionnaire de traduction étendu dans `languages.h` avec étiquettes GPS et capteurs environnementaux

### Compatibilité
- Totalement compatible avec ESP32-S3 DevKitC-1 N16R8 et cartes ESP32 Classic
- Aucun changement au pin mapping ou configuration existants
- Compatible rétroactivement avec les diagnostics et fonctionnalités existants

## [Version 3.18.0] - 2025-12-06

### Nouvelles fonctionnalités
1. **Module GPS NEO-6M/NEO-8M** : Support complet pour modules GPS via UART1 avec parsing NMEA (RMC, GGA, GSA, GSV).
   - Lecture latitude, longitude, altitude, vitesse, cap
   - Qualité du signal (HDOP, PDOP, VDOP), nombre de satellites
   - Support optionnel signal PPS (Pulse Per Second)
   - Broches configurées dans `config.h` : RX=18/TX=17/PPS=8 (ESP32-S3), RX=16/TX=17/PPS=4 (ESP32 Classic)
2. **Capteurs environnementaux AHT20 + BMP280** : Support I2C pour température, humidité et pression atmosphérique.
   - AHT20 (adresse 0x38) : température et humidité
   - BMP280 (adresse 0x76/0x77) : température, pression et altitude calculée
   - Température moyenne des deux capteurs pour plus de précision
   - API endpoints `/api/gps`, `/api/gps-test`, `/api/environmental-sensors`, `/api/environmental-test`
3. **Interface Web améliorée** : Cartouches GPS dans la page Wireless et capteurs environnementaux dans la page Sensors.
4. **Traductions** : Ajout de 28 nouvelles clés de traduction FR/EN pour GPS et capteurs environnementaux.

### Technique
5. Nouveaux fichiers : `gps_module.h/.cpp`, `environmental_sensors.h/.cpp` dans architecture modulaire
6. Initialisation automatique des modules GPS et environnementaux au démarrage
7. Parsing NMEA optimisé sans bibliothèques externes
8. Calibration BMP280 avec compensation température et pression

### Impact
- Release mineure (3.17.1 → 3.18.0) ; nouvelles fonctionnalités majeures ajoutées tout en préservant la compatibilité.

## [Version 3.17.1] - 2025-12-05

### Changements
1. **Pin mapping ESP32-S3 rafraîchi :** GPS RX=18/TX=17/PPS=8 ; TFT MOSI=11/SCLK=12/CS=10/DC=9/RST=13/BL=7 ; LED RGB R=47/G=48/B=45 ; capteurs mis à jour (PWM/Buzzer=14, DHT=5, Mouvement=4, Lumière=19, HC-SR04 TRIG=3/ECHO=6) ; boutons inchangés BTN1=1/BTN2=2.
2. **Pin mapping ESP32 Classic rafraîchi :** GPS RX=16/TX=17/PPS=4 ; TFT MOSI=23/SCLK=18/CS=19/DC=27/RST=26/BL=13 ; LED RGB R=12/G=14/B=15 ; capteurs mis à jour (PWM/Buzzer=5, DHT=25, Mouvement=36, Lumière=2, HC-SR04 TRIG=32/ECHO=33) ; boutons BTN1=34/BTN2=35.
3. **Docs & build :** README/README_FR, guides de mapping, matrices de fonctionnalités, guides d'usage et de build mis à jour ; `PROJECT_VERSION` passé à `3.17.1` dans `platformio.ini`.

### Impact
- Release patch (3.17.0 → 3.17.1) ; périmètre fonctionnel inchangé en dehors des nouvelles affectations par défaut et de la documentation alignée.

## [Version 3.17.0] - 2025-12-01
1. Fonctionnalité : Prise en charge basique des boutons matériels (BTN1/BTN2) activée via `ENABLE_BUTTONS` sans modifier le pin mapping.
   - BTN1 : appui court → bip bref du buzzer (feedback).
   - BTN2 : appui court → cycle des couleurs de la LED RGB (rouge → vert → bleu → blanc).
2. Documentation : Mise à jour des références de version et description du comportement des boutons (FR/EN) dans les documents utilisateur.
3. Build : Passage de `PROJECT_VERSION` à `3.17.0` dans `platformio.ini`.
4. Pin mapping : Aucun changement ; respect des `PIN_BUTTON_1`/`PIN_BUTTON_2` selon la cible.

## [Version 3.16.0] - 2025-11-28

### Nouvelles fonctionnalités
1. **Journalisation des IP des Clients Connectés** : Ajout de la journalisation automatique des adresses IP des clients connectés dans le moniteur série pour une meilleure surveillance réseau et diagnostics.
2. **Configuration de la Résolution OLED** : Ajout de la possibilité de configurer la résolution de l'écran OLED (largeur × hauteur) dynamiquement via l'interface web.
3. **Interface de Configuration TFT** : Ajout d'une configuration complète de l'écran TFT via l'interface web incluant :
   - Configuration du pin mapping (MOSI, SCLK, CS, DC, RST, BL)
   - Configuration de la résolution d'affichage (largeur × hauteur)
   - Paramètres de rotation
4. **Point d'API `/api/tft-config`** : Nouveau endpoint pour la configuration TFT avec validation et mises à jour en temps réel.
5. **API Info Écrans Améliorée** : Mise à jour de `/api/screens-info` pour inclure les détails de résolution et de broches pour les écrans OLED et TFT.

### Améliorations
6. **Surveillance Réseau** : Les connexions clients sont maintenant enregistrées au format `[Client] <endpoint> connected from IP: <address>` pour un débogage plus facile.
7. **Configuration Dynamique** : Tous les paramètres d'affichage (OLED/TFT) peuvent maintenant être modifiés sans recompilation du code.

### Détails Techniques
8. Ajout de variables globales pour la configuration runtime : `oledWidth`, `oledHeight`, `tftMOSI`, `tftSCLK`, `tftCS`, `tftDC`, `tftRST`, `tftBL`, `tftWidth`, `tftHeight`, `tftRotation`.
9. Amélioration de `handleOLEDConfig()` pour supporter les paramètres de résolution (largeur, hauteur).
10. Implémentation de la fonction helper `logClientConnection()` pour une journalisation cohérente des IP.
11. Version : Passage de 3.15.1 à 3.16.0 suivant le versioning sémantique (nouvelles fonctionnalités mineures).

### Améliorations de l'Interface Web
12. **Moniteur Réseau Sans Fil** : Ajout de l'affichage en temps réel du statut de connexion dans l'onglet WiFi montrant l'adresse IP actuelle, le SSID, la passerelle, le serveur DNS et la force du signal dans une grille d'information dédiée avant le scanner WiFi.

## [Version 3.15.1] - 2025-11-27

### Corrections de bugs
1. **Correction mémoire critique pour ESP32 Classic** : Correction de l'échec de chargement de l'interface web sur l'environnement `esp32devkitc` causé par épuisement du heap lors du service de gros fichiers JavaScript.
2. **Streaming PROGMEM** : Implémentation du transfert par morceaux (blocs de 1Ko) pour le contenu JavaScript servi depuis PROGMEM, éliminant les grosses allocations String qui causaient des plantages sur cartes sans PSRAM.
3. **Amélioration universelle** : Optimisation mémoire bénéficiant à tous les environnements (esp32s3_n16r8, esp32s3_n8r8, esp32devkitc) avec fragmentation du heap réduite pendant le service des pages web.

### Détails techniques
4. Modification de `handleJavaScriptRoute()` dans `src/main.cpp` pour streamer le contenu `DIAGNOSTIC_JS_STATIC` en utilisant `memcpy_P()` et `sendContent()` par petits morceaux.
5. Remplacement de l'unique grosse allocation `String(FPSTR(DIAGNOSTIC_JS_STATIC))` par transfert itératif par morceaux.
6. Aucun changement de fonctionnalité UI ou expérience utilisateur – optimisation purement interne.

### Impact
7. **ESP32 Classic (esp32devkitc)** : L'interface web se charge désormais de manière fiable sur configurations 4Mo Flash / sans PSRAM.
8. **Variantes ESP32-S3** : Efficacité mémoire améliorée sans régression.
9. Version : Release patch suivant le versioning sémantique (3.15.0 → 3.15.1).

## [Version 3.15.0] - 2025-11-27

### Nouvelles fonctionnalités
1. **Support Multi-Environnements** : Ajout de trois environnements de build distincts dans `platformio.ini` :
   - `esp32s3_n16r8` (par défaut) : ESP32-S3 avec 16Mo Flash + 8Mo PSRAM (QSPI/OPI)
   - `esp32s3_n8r8` : ESP32-S3 avec 8Mo Flash + 8Mo PSRAM
   - `esp32devkitc` : ESP32 Classic avec 4Mo Flash (sans PSRAM)
2. **Pin Mapping Spécifique Matériel** : Configurations de broches dédiées dans `config.h` pour chaque cible via compilation conditionnelle (`TARGET_ESP32_S3` / `TARGET_ESP32_CLASSIC`).
3. **Pin Mapping Partagé** : ESP32-S3 N8R8 et ESP32 Classic utilisent des affectations de broches communes là où le matériel le permet.

### Modifications de Configuration
4. **Pin Mapping ESP32-S3** (N16R8 / N8R8) :
   - I2C : SDA=21, SCL=20
   - LED RGB : R=14, G=13, B=18
   - Capteurs : DHT=19, Lumière=4, Distance TRIG=16/ECHO=17, Mouvement=39, Buzzer=3
   - TFT ST7789 : MOSI=11, SCLK=12, CS=10, DC=9, RST=7, BL=15
   - GPS : RXD=8, TXD=5, PPS=38

5. **Pin Mapping ESP32 Classic** (DevKitC) :
   - I2C : SDA=21, SCL=22
   - LED RGB : R=25, G=26, B=27
   - Capteurs : DHT=4, Lumière=34, Distance TRIG=5/ECHO=18, Mouvement=36, Buzzer=13
   - TFT ST7789 : MOSI=23, SCLK=18, CS=15, DC=2, RST=4, BL=32
   - GPS : RXD=16, TX=17, PPS=39
   - Boutons : BTN1=0 (BOOT), BTN2=35

### Documentation
6. Référence complète du pin mapping documentée dans `config.h` avec séparation claire par cible.
7. Guide de sélection d'environnement de build ajouté à la documentation.

### Technique
8. Version : Passage de 3.14.1 à 3.15.0 (nouvelle fonctionnalité mineure : support multi-environnements).
9. Compilation : Validée sur les trois environnements avec defines spécifiques à chaque cible.

## [Version 3.14.0] - 2025-11-27

### Nouvelles fonctionnalités
1. **Interface web TFT** : Ajout d'un cartouche complet pour tester l'affichage TFT ST7789 (240x240) via l'interface web.
2. **Tests TFT** : 8 tests individuels disponibles : écran de démarrage, couleurs, formes géométriques, rendu de texte, motifs de lignes, animation, barre de progression, message final.
3. **Bouton de retour écran de démarrage** : Nouveau bouton pour restaurer l'affichage de démarrage sur OLED et TFT.
4. **API REST TFT** : 3 nouveaux endpoints : `/api/tft-test` (test complet), `/api/tft-step?step=<id>` (test individuel), `/api/tft-boot` (retour écran démarrage).
5. **API REST OLED** : Nouvel endpoint `/api/oled-boot` pour restaurer l'écran de démarrage OLED.

### Améliorations
6. Interface web : Cartouche TFT avec structure similaire à OLED pour cohérence.
7. Traductions : 13 nouvelles clés bilangues (EN/FR) pour l'interface TFT.
8. Architecture : Tests TFT suivent le même modèle que les tests OLED pour maintenabilité.

### Technique
9. Version : Passage de 3.13.1 à 3.14.0 (nouvelle fonctionnalité mineure).
10. Compilation : Validée sur les trois environnements `esp32s3_n16r8`, `esp32s3_n8r8`, `esp32devkitc`.

## [Version 3.13.1] - 2025-11-26

### Mises à jour
1. Dépendances : passage des bibliothèques Adafruit dans `platformio.ini` aux intervalles avec chapeau (`^`) pour autoriser les mises à jour mineures/patch sûres.
2. Build : compilation validée sans erreur sur les trois environnements : `esp32s3_n16r8`, `esp32s3_n8r8`, `esp32devkitc`.
3. Portée : maintenance documentaire et de configuration de build ; aucun changement fonctionnel du firmware.

## [Version 3.13.0] - 2025-11-26

### Changements
- Synchronisation de la documentation pour les environnements multi-cartes : `esp32s3_n16r8`, `esp32s3_n8r8` et `esp32devkitc`.
- Suppression des références aux cartes non supportées (ESP32-S2/C3/C6/H2) dans les guides et matrices.
- Corrections des valeurs par défaut I2C (SCL=20) et maintien des défauts HC‑SR04 (TRIG=16, ECHO=17).
- Mise à jour de `PROJECT_VERSION` vers 3.13.0 dans `platformio.ini`.

### Notes
- Builds validés pour esp32s3_n16r8 et esp32s3_n8r8. Compilation `esp32devkitc` présente mais non testée faute de matériel.

# Changelog (FR)

Toutes les évolutions notables d'ESP32 Diagnostic Suite sont documentées ici. Ce projet suit [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Version 3.12.3] - 2025-11-26
### Modifications
- Pins par défaut du **HC‑SR04** définies à `TRIG=16`, `ECHO=17` dans `config.h` et valeurs par défaut alignées dans l'interface web.

### Ajouts
- Nouveau mémo de référence rapide : `docs/PIN_MAPPING_FR.md` (EN : `docs/PIN_MAPPING.md`).

### Changements Techniques
- Incrément de `PROJECT_VERSION` vers 3.12.3 dans `platformio.ini`.
- Aucun autre changement fonctionnel.

## [Version 3.12.2] - 2025-11-26
### Corrections
- **HC‑SR04** : Séquence de mesure durcie en attendant que ECHO repasse à LOW avant déclenchement et en utilisant `pulseInLong()` avec timeout étendu. Évite les faux résultats « No echo ».

### Changements Techniques
- Aucun changement de pin mapping. Ajustements limités à la séquence de mesure et à la gestion du timeout.

## [Version 3.12.1] - 2025-11-26
### Corrections
- **Activation PSRAM** : Activation garantie de la PSRAM sur ESP32-S3 DevKitC-1 N16R8 sous PlatformIO via `board_build.psram = enabled` et définition de `BOARD_HAS_PSRAM`. Corrige les échecs des tests mémoire utilisant la PSRAM externe.

### Améliorations
- Configuration PlatformIO affinée pour une détection et une utilisation cohérentes de la PSRAM avec Arduino-ESP32 3.3.x.
- Mise à jour des documentations FR/EN et des références de version vers 3.12.1.

### Changements Techniques
- Incrément de `PROJECT_VERSION` vers 3.12.1 dans `platformio.ini`.
- Aucun changement de pin mapping (config.h inchangé).

## [Version 3.12.0] - 2025-11-26
### Corrections
- **CONFIG** : Validation et confirmation de la configuration du pin backlight TFT (GPIO 15).
- Résolution des déclarations TFT dupliquées causant des avertissements de compilation.
- Correction de la documentation du mapping des pins dans config.h pour ESP32-S3 DevKitC-1 N16R8.

### Améliorations
- **DOCUMENTATION** : Suppression des fichiers de développement et débogage obsolètes pour une structure de dépôt plus claire.
- Suppression des fichiers temporaires : CORRECTIFS_APPLIQUES.md, CORRECTIF_v3.11.1.md, DEBUGGING_WEB_UI.md, PATCH_WEB_UI.cpp, RESUME_FINAL.md, RESUME_v3.11.1_FINAL.md.
- Consolidation des commentaires et de l'organisation du mapping des pins dans config.h.
- Amélioration de la configuration PlatformIO avec paramètres PSRAM optimisés.

### Changements Techniques
- Finalisation de la configuration des pins TFT pour utilisation en production sur ESP32-S3 N16R8.
- Nettoyage du dépôt des artefacts de développement et notes de maintenance.
- Standardisation de la structure et des commentaires du fichier de configuration.
- Mise à jour de la version vers 3.12.0 dans platformio.ini.

## [Version 3.11.4] - 2025-11-25
### Améliorations
- **MAINTENANCE** : Améliorations de la qualité du code et nettoyage.
- Suppression de l'historique obsolète des versions de développement dans les en-têtes du code source.
- Suppression de la fonction `handleJavaScript()` inutilisée (élimination du code mort).
- Simplification et standardisation du style de commentaires dans toute la base de code.
- Correction de la faute de frappe en français : "defaut" → "défaut" dans les messages de configuration.

### Changements Techniques
- Nettoyage des commentaires de versions obsolètes (v3.8.x-dev à v3.10.3).
- Suppression de la fonction `handleJavaScript()` jamais référencée dans le routage.
- Normalisation des délimiteurs de commentaires et suppression des annotations redondantes.
- Mise à jour de la version vers 3.11.4 dans platformio.ini.

## [Version 3.11.3] - 2025-11-25
### Corrections
- **CONFIG** : Correction du pin backlight TFT de GPIO 48 à GPIO 15 pour résoudre le conflit avec NeoPixel.
- Le rétro-éclairage du TFT utilise maintenant le GPIO 15 dédié au lieu du GPIO 48 (conflit NeoPixel).

### Changements Techniques
- Mise à jour de la définition `TFT_BL` dans `config.h` du pin 48 au pin 15.
- Assure le bon fonctionnement du rétro-éclairage TFT sans conflit matériel avec NeoPixel.

## [Version 3.11.2] - 2025-11-25
### Corrections
- **BUILD** : Correction de l'erreur de typage FPSTR() empêchant la compilation.
- Correction de la gestion des types de pointeurs pour les chaînes PROGMEM dans l'implémentation du transfert par morceaux.
- Changement de `const char* staticJs = FPSTR(...)` vers un modèle d'accès PROGMEM approprié pour ESP32.
- Résolution de l'erreur de compilation `cannot convert 'const __FlashStringHelper*' to 'const char*'`.

### Changements Techniques
- Mise à jour de `handleJavaScriptRoute()` pour utiliser l'accès direct au pointeur PROGMEM pour la vérification.
- Maintien de l'implémentation du transfert par morceaux tout en corrigeant la compatibilité des types.
- Ajout de commentaires expliquant le mapping mémoire PROGMEM spécifique à ESP32.

## [Version 3.11.1] - 2025-11-25
### Corrections
- **CRITIQUE** : Correction du chargement JavaScript de l'interface web utilisant l'encodage de transfert par morceaux.
- Remplacement de la génération JavaScript monolithique par un streaming mémoire-efficient par morceaux.
- Amélioration des logs de débogage pour afficher la répartition de la taille JavaScript (préambule, traductions, code statique).
- Correction des problèmes de dépassement mémoire quand le JavaScript dépassait la taille du tampon alloué.

### Changements Techniques
- Implémentation de l'encodage de transfert par morceaux dans `handleJavaScriptRoute()`.
- Séparation de la génération JavaScript en trois parties : préambule, traductions, code statique.
- Ajout de vérification PROGMEM pour détecter les fonctions manquantes avant envoi.
- Réduction de la pression mémoire en streamant le contenu JavaScript au lieu de le mettre en tampon.

## [Version 3.11.0] - 2025-11-25
### Ajouts
- **NOUVELLE FONCTIONNALITÉ** : Support de l'écran TFT ST7789 avec résolution 240x240.
- Écran de démarrage sur l'affichage TFT montrant l'initialisation du système.
- Visualisation en temps réel de l'état de connexion WiFi sur le TFT.
- Affichage de l'adresse IP sur le TFT une fois connecté.
- Pins TFT configurables (MOSI, SCLK, CS, DC, RST, Backlight) dans config.h.
- Nouveau fichier d'en-tête tft_display.h pour la gestion de l'affichage TFT.

### Corrections
- Améliorations de l'initialisation de l'interface web et du chargement des onglets.
- Amélioration de la gestion des erreurs JavaScript pour une meilleure réactivité de l'interface.

### Améliorations
- Meilleur retour visuel pendant le processus de démarrage avec l'écran TFT.
- Support de double affichage (OLED + TFT) pour des diagnostics améliorés.

## [Version 3.10.3] - 2025-11-25
### Ajouts
- Aucun.

### Corrections
- **CRITIQUE** : Correction de l'erreur de compilation `portGET_ARGUMENT_COUNT()` dans les macros FreeRTOS.
- Changement de la plateforme depuis la version git instable vers la version stable `espressif32@6.5.0`.
- Ajout du build flag `-DCONFIG_FREERTOS_ASSERT_ON_UNTESTED_FUNCTION=0` pour éviter les conflits de macros FreeRTOS.

### Améliorations
- Stabilité de compilation améliorée en utilisant une version stable de la plateforme au lieu du dépôt git.
- Compatibilité améliorée avec FreeRTOS et le framework Arduino-ESP32.

## [Version 3.10.2] - 2025-11-25
### Ajouts
- Aucun.

### Corrections
- **CRITIQUE** : Correction des flags de compilation C++17 causant un crash au démarrage.
- Ajout de `build_unflags = -std=gnu++11` pour forcer le remplacement du standard C++ par défaut.
- Changement de `-std=gnu++17` vers `-std=c++17` pour une conformité C++17 stricte.
- Résolution des problèmes d'initialisation des variables inline empêchant le fonctionnement de l'interface web et de l'écran OLED.

### Améliorations
- Nettoyage des flags de compilation dupliqués dans `platformio.ini`.
- Configuration du système de build améliorée pour un meilleur support C++17.

## [Version 3.9.0] - 2025-11-11
### Ajouts
- Mise à jour de la documentation du projet et des références du dépôt pour le déploiement basé sur PlatformIO.
- Migration d'Arduino IDE vers PlatformIO pour une meilleure cohérence de compilation et gestion des dépendances.

### Corrections
- Aucune.

### Améliorations
- Mise à jour de toute la documentation pour refléter la toolchain PlatformIO et la nouvelle URL du dépôt.
- Standardisation des références de version dans toute la documentation du projet.
- Structure de dépôt améliorée pour un flux de travail de développement professionnel.

## [Version 3.8.14] - 2025-11-11
### Ajouts
- Aucun.

### Corrections
- **Critique** : Ajout de la déclaration manquante de la variable `runtimeBLE` qui causait des erreurs de compilation sur les cibles ESP32-S2/S3/C3/C6/H2.
- Suppression du tableau `DIAGNOSTIC_VERSION_HISTORY` inutilisé pour réduire l'encombrement du code.

### Améliorations
- Nettoyage des initialisations redondantes de `String` (`String foo = ""` → `String foo`) sur 7 instances.
- Standardisation de l'espacement des boucles `for` (`for(` → `for `) sur 23 instances pour améliorer la lisibilité.
- Suppression des commentaires superflus pour clarifier le code.
- Mise à jour des références de version vers 3.8.14.

## [Version 3.8.0] - 2025-11-08
### Ajouts
- Version de production basée sur 3.7.30 avec mise à jour de version pour le déploiement.

### Corrections
- Aucune.

### Améliorations
- Standardisation des références de version dans toute la documentation et les constantes firmware.

## [Version 3.7.30] - 2025-11-08
### Ajouts
- Les cartes équipées d'un écran OLED affichent désormais les phases de connexion Wi-Fi (association, DHCP, réussite) dès le démarrage, sans passer par le moniteur série.

### Corrections
- Protection des descripteurs NimBLE optionnels et alignement des callbacks BLE pour supprimer les erreurs d'inclusion manquante et `override` sur ESP32 et ESP32-S3.
- Validation du statut `start()` de NimBLE avant de copier les périphériques annoncés afin d'éviter toute conversion invalide.

### Améliorations
- Réutilisation sûre des tampons de scan NimBLE, reprise de la diffusion après un échec et réponses HTTP cohérentes lorsque le scan n'est pas disponible.
- Documentation FR/EN actualisée pour détailler l'écran Wi-Fi de démarrage et les consignes de compatibilité BLE renforcées.

## [Version 3.5.1] - 2025-10-30
### Ajouts
- Extension des liaisons de traduction en direct sur les panneaux de diagnostics afin que les widgets sans fil, pager, GPIO et exports réutilisent le catalogue commun sans doublons.

### Corrections
- Résolution des métadonnées carte non traduites (cœurs CPU, adresses MAC, raisons de reset, métriques mémoire) pour que le changement de langue reste synchronisé sans rechargement.

### Améliorations
- Raffinement du rafraîchisseur de traduction côté client pour réappliquer instantanément préfixes, suffixes et placeholders lors d'une bascule de langue.

---

## [Version 3.4.0] - 2025-10-29
### Ajouts
- Mutualisation des prototypes des helpers JSON afin d'exposer des signatures cohérentes entre modules de diagnostic.

### Corrections
- Résolution des échecs de compilation causés par des déclarations de helpers JSON divergentes selon les modules.
- Rétablissement des conversions `String` sûres dans la chaîne de traduction après l'optimisation du catalogue.

### Améliorations
- Unification des builders de réponses JSON HTTP et des handlers périphériques pour réduire les duplications dans les exports.
- Harmonisation des libellés bilingues et retrait du diagnostic touchpad historique de la séquence par défaut.
- Mise à jour complète de la documentation FR/EN afin de refléter la stabilisation de la maintenance 3.3.x.

---

## [Version 3.3.0] - 2025-10-27
### Ajouts
- Néant.

### Corrections
- Suppression des bannières de commentaires « NEW FEATURE » devenues trompeuses pour les passages de maintenance.

### Améliorations
- Consolidation des notes historiques « Version de dev » dans une constante firmware afin de préserver la mémoire du projet sans dupliquer les bannières.
- Synchronisation du commentaire d'en-tête et de la macro `DIAGNOSTIC_VERSION` sur 3.3.0 avec une documentation mise à jour autour de cette maintenance.

---

## [Version 3.2.0] - 2025-10-29
### Ajouts
- Documentation de la requête `/api/memory-details` et des alertes de fragmentation issues de la dernière campagne de tests.
- Intégration, dans les guides d'utilisation et de dépannage, de la checklist Bluetooth® et Wi-Fi validée lors du débogage post-release.

### Corrections
- Remplacement des mentions 3.1.19 restantes dans les README et guides afin d'aligner bannière, documentation et changelog bilingue sur 3.2.0.
- Clarification des consignes de changement de langue pour éviter les instructions obsolètes lors de la vérification de `/api/set-language`.

### Améliorations
- Guides d'installation, de configuration et d'utilisation étoffés avec des étapes de vérification concrètes (exports mémoire, widget BLE, métadonnées de scan Wi-Fi).
- Guide de contribution renforcé pour rappeler la synchronisation systématique des contenus FR/EN lors des maintenances.

---

## [Version 3.1.19] - 2025-10-28
### Ajouts
- Néant.

### Corrections
- Mise à jour de l'ensemble des guides pour référencer le duo bilingue (`CHANGELOG.md` et `CHANGELOG_FR.md`) et conserver la cohérence FR/EN.

### Améliorations
- Scission du journal des modifications en fichiers dédiés anglais et français afin de faciliter les maintenances ciblées.
- Rafraîchissement du bandeau de version, de `DIAGNOSTIC_VERSION` et des encarts de version 3.1.19 dans toute la documentation.

---

## [Version 3.1.18] - 2025-10-27
### Ajouts
- Néant.

### Corrections
- Validation de `/api/set-language` pour renvoyer immédiatement HTTP 200 sur `fr`/`en` et HTTP 400 sur les codes non pris en charge, supprimant les réponses ambiguës.
- Synchronisation des journaux série et exports avec les nouvelles réponses JSON du changement de langue.

### Améliorations
- Réservation anticipée du tampon de `jsonEscape` afin de réduire les allocations pendant la génération d'exports.
- Inclusion explicite de `<cstring>` pour assurer la compatibilité des fonctions de chaîne C sur les toolchains Arduino récentes.
- Documentation FR/EN alignée sur la maintenance 3.1.18 (README, guides d'installation, configuration, utilisation, architecture, contribution et dépannage).

---

## [Version 3.1.16] - 2025-10-27
### Ajouts
- Bandeau sticky unifié affichant version, statut Wi-Fi/Bluetooth et lien d'accès direct depuis le tableau de bord web.
- Commandes Bluetooth® (activation, renommage, réinitialisation) exposées dans l'interface et via les endpoints REST dédiés.

### Corrections
- Délégation d'événements repositionnée pour la navigation par onglets afin de restaurer la sélection après rechargement dynamique.
- Actualisation des traductions sur les onglets et éléments dynamiques après un changement de langue pour éviter les libellés incohérents.
- Rétablissement du libellé « Non testé » sur les diagnostics additionnels dans l'interface, l'API et les exports.

### Améliorations
- Réponses de scan Wi-Fi enrichies (BSSID, bande, largeur de canal, mode PHY) pour faciliter l'analyse radio.
- Préallocation étendue de `htmlEscape` afin de limiter les réallocations lors du streaming HTML et des exports.
- Synchronisation du commentaire d'en-tête et de `DIAGNOSTIC_VERSION` sur 3.1.16 pour aligner journaux et fichiers générés.

---

## [Version 3.1.15-maint] - 2025-10-26
### Ajouts
- Néant.

### Corrections
- Uniformisation du libellé « Non testé » pour les diagnostics additionnels (ADC, tactile, PWM, stress) dans l'interface, les exports et l'API.
- Synchronisation du commentaire d'en-tête et de `DIAGNOSTIC_VERSION` sur 3.1.15-maint afin d'afficher le bon numéro dans les journaux et fichiers exportés.

### Améliorations
- Extension de la préallocation de `htmlEscape` pour réduire les réallocations lors de l'échappement des chaînes HTML générées.
- Documentation FR/EN actualisée (README, guides d'installation, configuration, utilisation, architecture, contribution et dépannage) pour refléter la maintenance 3.1.15-maint et les vérifications associées.

---

## [Version 3.1.14-maint] - 2025-10-26
### Ajouts
- Néant.

### Corrections
- Rétablissement des libellés "Non testé" pour les diagnostics additionnels afin d'assurer une interface cohérente en français.
- Alignement du commentaire de version et de `DIAGNOSTIC_VERSION` sur la révision 3.1.14-maint pour refléter la maintenance en cours.

### Améliorations
- Augmentation de la réserve mémoire utilisée par `htmlEscape` pour limiter les réallocations lors de l'échappement des chaînes.

---

## [3.1.1] - 2025-10-26
### Corrections
- Suppression des commentaires obsolètes et marqueurs internes pour clarifier la maintenance du firmware.

### Documentation
- Mise à jour des guides FR/EN, exports et constantes pour refléter le passage en version 3.1.1.

---

## [3.1.0] - 2025-10-24
### Points forts
- Activation automatique du service Bluetooth® Low Energy avec diffusion native sur les cibles compatibles (ESP32, S3, C3, C6, H2).
- Tableau de bord web enrichi d'une cartouche BLE détaillant l'état, le nom du périphérique et les journaux d'appairage récents.
- Jeux de documents FR/EN entièrement réécrits couvrant installation, configuration, utilisation, architecture, dépannage et contribution.

### Fonctionnalités
- Service GATT par défaut avec renommage dynamique du périphérique via l'interface web ou la console série.
- Nouveaux messages d'état Wi-Fi contextualisés (association, DHCP, authentification, portail captif) affichés dans le bandeau supérieur et l'API REST.
- Prise en charge initiale assouplie pour les révisions récentes des SoC ESP32 (S3/C3/C6/H2) sur les modules BLE et Wi-Fi.

### Interface & API
- Navigation par onglets fiabilisée via délégation d'événements JavaScript, évitant les blocages après un rafraîchissement partiel.
- Menu supérieur responsive avec regroupement monoligne des indicateurs de statut pour les écrans < 768 px.
- Export TXT/JSON/CSV et aperçu imprimable synchronisés avec les nouveaux états BLE et Wi-Fi.

### Documentation
- Regroupement des guides dans `docs/` avec renvois croisés vers les versions anglaises et françaises.
- Ajout d'un guide de contribution détaillé (workflow Git, validation, formatage) et de procédures de dépannage spécifiques BLE/Wi-Fi.

### Corrections
- Harmonisation des messages BLE entre la console série, l'API `/api/status` et l'interface web.
- Nettoyage du JSON de traduction dynamique pour éviter les clés orphelines et incohérences de casse.
- Clarification de la détection des onglets actifs dans le bundle JavaScript, supprimant les doublons d'écouteurs.

---

## [2.6.0] - 2025-10-15
### Fonctionnalités
- Boutons manuels sur l'interface web et endpoints REST associés pour piloter individuellement chaque animation de diagnostic OLED.
- Possibilité de déclencher et d'arrêter les séquences d'affichage directement depuis la console série.

### Améliorations
- Simplification du flux de reconfiguration I²C OLED : sélection des broches SDA/SCL et vitesse directement depuis l'interface.
- Actualisation du pack de traductions (FR/EN) pour tous les nouveaux libellés OLED et états d'exécution.

### Corrections
- Suppression complète de la prise en charge TFT (firmware, dépendances Arduino, fragments UI), réduisant la taille du binaire et les avertissements de compilation.

---

## [2.5.1] - 2025-10-10
### Corrections
- Statuts PSRAM clarifiés dans le tableau de bord, les exports et les rapports imprimables (valeur détectée, fréquence, mode octal).
- Ajustement de l'ordre de détection PSRAM pour éviter les faux négatifs sur ESP32-WROVER.

### Améliorations
- Mise en avant des cartes compatibles PSRAM avec rappels pour activer l'option dans l'IDE Arduino et la documentation d'installation.

---

## [2.5.0] - 2025-10-08
### Fonctionnalités
- Exports TXT/JSON/CSV intégralement traduits (FR/EN) avec inclusion de l'horodatage ISO 8601 et du numéro de build.
- Prévisualisation imprimable alignée sur la langue sélectionnée et sur les nouveaux blocs de diagnostic.

### Améliorations
- Nommage des fichiers d'export enrichi avec le numéro de version, la date et la carte détectée.
- Révision du workflow d'export REST pour réduire les temps de génération et homogénéiser les intitulés de colonnes.

### Corrections
- Harmonisation des formats de date (localisation FR/EN) dans les exports et la vue imprimable.
- Ajout des traductions manquantes sur les boutons de téléchargement et sur les libellés de sections.

---

## [2.4.0] - 2025-10-07
### Fonctionnalités
- Interface multilingue complète FR/EN avec bascule à chaud sans rechargement de page.
- Endpoints `/api/set-language` et `/api/get-translations` permettant aux clients externes de piloter la langue.
- Fichier `languages.h` regroupant plus de 150 chaînes traduites pour l'interface, les exports et la console série.

### Améliorations
- Tous les textes (web, API, exports, logs) reposent sur le catalogue de traductions centralisé.
- Ajout d'un indicateur visuel de langue active et persistance de la préférence dans le stockage local du navigateur.

---

## [2.3.0] - 2025-10-06
### Fonctionnalités
- Batterie de 10 tests OLED 0,96" I²C (bascule contrastes, inversion, scroll, trames personnalisées) avec messages explicatifs.
- Reconfiguration dynamique des broches SDA/SCL via l'interface web et l'API pour faciliter le recâblage.

### Améliorations
- Détection automatique de l'écran à l'adresse 0x3C avec relance des tests après connexion.
- Ajout d'un module de calibration de contraste pour optimiser les OLED selon la tension d'alimentation.

---

## [2.2.0] - 2025-10-05
### Fonctionnalités
- Support des écrans TFT SPI (ST7789/ILI9341) avec diagnostics dédiés (test couleurs, patterns, fréquence SPI).
- Assistant de configuration TFT (pinout, rotation, inversion des axes) accessible via l'interface et la console série.

### Notes
- Cette fonctionnalité a été retirée en 2.6.0 pour alléger le firmware et se concentrer sur les OLED.

---

## [2.0.0] - 2025-10-03
### Fonctionnalités
- Réécriture complète du diagnostic : GPIO, ADC, pads tactiles, PWM, I2C, SPI, PSRAM, partitions, stress test, benchmarks et exports.
- Support multi-Wi-Fi via `WiFiMulti`, découverte mDNS `ESP32-Diagnostic.local` et interface web modernisée.
- Ajout de l'API REST (lancement tests, récupération de rapports) et des exports TXT/JSON/CSV.

### Documentation
- Première version du guide d'utilisation détaillant la configuration réseau, les tests disponibles et la collecte des journaux.

---

## [1.0.0] - 2025-10-01
### Fonctionnalités
- Première version publique avec interface web basique, rapport mémoire, tests GPIO/ADC élémentaires et console série interactive.
- Génération d'un tableau de bord unique regroupant la configuration Wi-Fi, les mesures de capteurs et les actions rapides.

### Notes
- Base de code initiale posant les fondations du système de diagnostics évolutif.
