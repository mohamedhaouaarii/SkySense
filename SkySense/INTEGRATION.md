# SkySense — version intégrée

Cette version réunit la connexion Oracle, le tableau de bord, les gestions Stations et Equipment déjà présentes, la gestion Maintenance et la gestion Services intégrées depuis leurs modules autonomes.

## Modules présents

| Module | Contenu |
|---|---|
| `commun/` | Connexion Oracle partagée, session utilisateur et thème |
| `fenetre/` | Connexion, en-tête partagé, recherche globale et navigation |
| `dashboard/` | Tableau de bord des équipements |
| `station/` | Gestion Stations et exports PDF |
| `equipement/` | Gestion Equipment et exports |
| `maintenance/` | Gestion Maintenance, son modèle Oracle et son interface |
| `service/` | Gestion Services : modèle Oracle (`servicemodel`) et interface (`serviceui`) |
| `database/` | Schéma SkySense, scripts et notes de compatibilité |

La base principale conserve son schéma approuvé. Le module Maintenance écrit dans `MAINTENANCE` et `REALISER` avec des requêtes préparées et la connexion partagée `Connection`. Il ne crée pas de seconde connexion Oracle.

### Correspondance Maintenance–Oracle

- `CODE`, `TITRE`, `DATE_PREVUE`, `DATE_REALISEE`, `PRIORITE`, `STATUT`, `DESCRIPTION` et `COMPTE_RENDU` sont lus et écrits dans `MAINTENANCE`.
- Le personnel affecté est lu et écrit via `REALISER` vers `PERSONNEL`. Modifier le technicien principal conserve les autres affectations de l’intervention.
- L’identifiant numérique est attribué dans une transaction avec un verrou sur la table, car le schéma approuvé ne crée pas de séquence `MAINTENANCE`.
- Le schéma approuvé ne relie pas Maintenance à Equipment ou Stations et ne stocke ni fin planifiée, ni début réel, ni cases de checklist, ni durée de résolution. Ces champs sont désactivés ou indiqués comme non stockés quand Oracle est connecté.
- `SkySense_Maintenance_Standalone/sql/maintenance_schema.sql` n’est pas repris. Ne créez pas de table `MAINTENANCE_INTERVENTION` pour cette version intégrée.

## Lancer le projet

Ouvrir `SkySense.pro` dans Qt Creator avec un kit Qt 6 Desktop incluant Widgets, SQL, Multimedia et Print Support. Configurer une source ODBC `SKYSENSE` vers Oracle, puis lancer l’application. La connexion Oracle est demandée au démarrage; l’écran d’accès utilise ensuite les comptes de `PERSONNEL`.

Pour une base neuve, suivre les étapes de `database/NOTES_SCHEMA.md` et le script `database/skysense_schema_profa.sql`. Ce script réinitialise les tables SkySense : ne pas le relancer sur une base contenant des données à conserver.

## Accès

Un compte `ADMIN` ouvre les modules présents. Un rôle `MAINTENANCE` ouvre Dashboard et Maintenance, un rôle `SERVICE` ouvre Dashboard et Services. Les rôles Stations et Equipment ouvrent leurs modules correspondants.

La source Staff n'est pas incluse dans les archives fournies : cette page ne fait pas partie de cette version intégrée.

### Gestion Services (`service/`)

Le module autonome écrivait dans ses propres tables (`SKYSENSE_SERVICES`, `SKYSENSE_AFFECTER`). Dans cette version il utilise le schéma approuvé partagé, avec la connexion partagée `Connection` (aucune seconde connexion, aucune table créée ni modifiée) :

| Écran Services | Oracle |
|---|---|
| Titre, type, description, priorité, statut | `SERVICE.TITRE`, `TYPE_SERVICE`, `DESCRIPTION`, `PRIORITE`, `STATUT` |
| Date prévue + heure de début | `SERVICE.DATE_PREVUE` |
| Heure de fin prévue | `SERVICE.DUREE_ESTIMEE` (minutes depuis l'heure de début) |
| Heure de fin réelle (statut Completed) | `SERVICE.DUREE_REELLE` (minutes depuis l'heure de début) |
| Prix (DT, 2 décimales) | `SERVICE.PRIX` |
| Membres affectés | `AFFECTER` → `PERSONNEL` (la liste des membres vient de `PERSONNEL`) |

- L'identifiant `ID_SERVICE` est attribué dans une transaction avec un verrou sur la table (même méthode que Maintenance), car le schéma n'a pas de séquence `SERVICE`.
- La suppression retire les lignes `AFFECTER` dans la même transaction.
- `TITRE` est `UNIQUE` dans le schéma approuvé (`UQ_SERVICE_TITRE`) : un titre ne peut servir qu'à un seul service. L'application affiche un message clair si le titre existe déjà.
- Non enregistrés (absents du schéma approuvé, voir `database/NOTES_SCHEMA.md` points 6 et 7) : la station d'un service, la checklist et la quantité. Avec Oracle connecté, le champ Station est désactivé, la checklist n'est pas proposée et la station s'affiche « — ».
- Sans Oracle (`--skip-oracle`), la page affiche des données de démonstration modifiables en mémoire uniquement.
- `database/verify_service_queries.sql` exécute les requêtes exactes du module puis annule tout (ROLLBACK).
