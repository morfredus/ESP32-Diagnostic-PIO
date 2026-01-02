
# ESP32 Suite de Diagnostic – Référence API REST (v3.33.2)

> **NOUVEAUTÉ v3.33.2** : Contrôle de luminosité PWM du rétroéclairage TFT via l'endpoint `/api/tft-brightness`. Voir [CHANGELOG_FR.md](../CHANGELOG_FR.md) pour les détails.

> **NOUVEAUTÉ v3.33.0** : Barre de progression TFT fluide, NeoPixel violet, logique BOOT robuste, et sélection dynamique du contrôleur TFT (ILI9341 ou ST7789). Voir [RELEASE_NOTES_3.33.0_FR.md](RELEASE_NOTES_3.33.0_FR.md).

> **AVERTISSEMENT** : Ce document reflète le firmware **v3.33.2** avec mappings de broches EXACTS depuis `include/board_config.h`. Toutes les assignations GPIO ont été vérifiées et synchronisées avec le code. Lisez [docs/PIN_MAPPING_FR.md](docs/PIN_MAPPING_FR.md) avant de flasher.

La REST API reflète les commandes du tableau de bord et expose les données de diagnostic pour l'automatisation. Tous les endpoints
sont servis en HTTP sur le même port que l'interface web. L'authentification est stateless ; sécurisez l'accès au niveau réseau
dans les environnements sensibles.

## URL de base
- Mode station : `http://<ip-appareil>/api`
- Mode point d'accès : `http://192.168.4.1/api`
- mDNS (si supporté) : `http://esp32-diagnostic.local/api`

## Conventions
- Réponses encodées en JSON UTF-8.
- Horodatages au format ISO 8601 (`YYYY-MM-DDThh:mm:ssZ`).
- Les erreurs incluent un objet `error` avec `code`, `message` et un tableau `details` optionnel.

## Endpoints
### `GET /api/version`
Retourne les métadonnées firmware et hash de build.
```json
{
  "version": "3.30.0",
  "git_commit": "abc1234",
  "build_time": "2024-05-10T09:15:00Z"
}
```

### `POST /api/run`
Déclenche un cycle de diagnostic.
- Payload : `{ "mode": "full" | "quick" }`
- Réponse : `{ "status": "queued", "run_id": "20240510-091500" }`

### `POST /api/stop`
Arrête le diagnostic en cours.
- Réponse : `{ "status": "stopped" }`

### `GET /api/status`
Fournit la progression en temps réel.
- Réponse :
```json
{
  "run_id": "20240510-091500",
  "stage": "memory",
  "progress": 58,
  "modules": [
    { "id": "env", "status": "PASS", "duration_ms": 520 },
    { "id": "connectivity", "status": "WARN", "duration_ms": 2180 }
  ]
}
```

### `GET /api/report`
Retourne les résultats du dernier cycle terminé.
- Paramètres :
  - `format` (optionnel) : `json` (défaut), `csv`, `txt`.
  - `module` (optionnel) : filtre sur un module spécifique.
- Les réponses streament le format demandé avec l'en-tête `Content-Type` adapté.

### `GET /api/report/<run_id>`
Récupère un cycle archivé par identifiant. Retourne `404` si le cache a été purgé.

### `GET /api/modules`
Liste les modules avec métadonnées et flags d'activation.
- Exemple :
```json
[
  {
    "id": "memory",
    "name": "Mémoire & stockage",
    "enabled": true,
    "requires": ["psram"],
    "export_keys": ["memory.heap_max", "storage.flash_crc"]
  }
]
```

### `POST /api/modules/<id>/toggle`
Active ou désactive un module optionnel.
- Payload : `{ "enabled": true }`
- Réponse : `{ "id": "neopixel", "enabled": true }`

### `POST /api/notes`
Associe des notes opérateur au dernier rapport.
- Payload : `{ "author": "QA-Team", "message": "Antenne remplacée, test relancé." }`
- Réponse : `{ "status": "stored" }`

## Limitation de débit
- Le firmware exécute un seul cycle à la fois.
- Les requêtes concurrentes sont mises en file ; le polling `/api/status` est limité à 1 requête/s.
- Réutiliser les exports mis en cache plutôt que de relancer un cycle complet.

## Codes d'erreur
| Code | Signification | Résolution |
|------|---------------|------------|
| `400` | Payload invalide | Valider le schéma JSON et les valeurs autorisées. |
| `401` | Accès refusé | Restreindre l'accès réseau ou ajouter une authentification proxy. |
| `404` | Ressource introuvable | Vérifier le `run_id` ou l'activation du module. |
| `409` | Cycle déjà actif | Attendre la fin du cycle en cours avant d'en déclencher un autre. |
| `503` | Matériel occupé | Le module initialise le matériel ; réessayer après quelques secondes. |

## Automatisation
- Utiliser `GET /api/version` en CI pour vérifier que l'unité déployée tourne bien en 3.30.0.
- Combiner `/api/run` et `/api/status` pour surveiller les longs tests, puis télécharger le rapport JSON pour analyse.
- Enregistrer les notes opérateur via `/api/notes` afin de conserver une trace d'audit dans les exports.

## Ressources associées
- [Guide d'utilisation](USAGE_FR.md) – workflows UI équivalents.
- [Modules de diagnostic](DIAGNOSTIC_MODULES_FR.md) – liste des IDs utilisés par l'API.
- [Directives de sécurité](SECURITY_FR.md) – recommandations pour sécuriser les endpoints HTTP.
