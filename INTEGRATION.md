# SkySense — version intégrée

Cette version réunit la connexion Oracle, le tableau de bord, les gestions Stations et Equipment déjà présentes, et la gestion Maintenance intégrée depuis le module autonome.

## Modules présents

| Module | Contenu |
|---|---|
| `commun/` | Connexion Oracle partagée, session utilisateur et thème |
| `fenetre/` | Connexion, en-tête partagé, recherche globale et navigation |
| `dashboard/` | Tableau de bord des équipements |
| `station/` | Gestion Stations et exports PDF |
| `equipement/` | Gestion Equipment et exports |
| `maintenance/` | Gestion Maintenance, son modèle Oracle et son interface |
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

Un compte `ADMIN` ouvre les modules présents. Un rôle `MAINTENANCE` ouvre Dashboard et Maintenance. Les rôles Stations et Equipment ouvrent leurs modules correspondants.

Les sources Staff et Services ne sont pas incluses dans les deux archives fournies, donc ces pages ne font pas partie de cette version intégrée.
