# SkySense — partie « Connexion + Tableau de bord + Équipement »

Cette copie contient uniquement ma partie du projet d'équipe. Elle compile et fonctionne seule
(Qt 6, MinGW, Oracle XE 11.2 via ODBC, DSN `SKYSENSE`).

## Contenu

| Dossier | Rôle |
|---|---|
| `commun/` | Connexion unique à Oracle (Singleton `Connection`), session, thème clair/sombre |
| `fenetre/` | Fenêtre principale : page de connexion, en-tête, barre de navigation (`sidebar.cpp`) |
| `compte/` | Comptes de connexion (`Compte`) + création du premier administrateur |
| `dashboard/` | Tableau de bord des **équipements**, lecture seule (`Equipement::statistiques()`) |
| `equipement/` | Gestion des équipements : **CRUD complet** (`Equipement` = modèle, `equipmentui` = vue) |
| `database/` | Base **complète** : les 8 tables du schéma validé, scripts et notes |

Architecture de l'atelier CRUD : les requêtes (préparées) sont dans les classes C++ du modèle,
jamais derrière un bouton ; les vues appellent le modèle par un objet membre.

## Base de données (complète)

- `database/skysense_schema_profa.sql` : crée toute la base (8 tables + données d'exemple).
- `database/ajout_comptes_personnel.sql` : comptes dans `PERSONNEL` (ROLE, MOT_DE_PASSE_HASH, SEL).
- `database/ajout_prix_titre_service.sql` : `SERVICE.PRIX` et titre de service unique.
- `database/NOTES_SCHEMA.md` : choix et points à faire valider par l'enseignante.

Aucun mot de passe n'est écrit dans le code ni dans les scripts. Au premier lancement connecté à
Oracle, si aucun compte `ADMIN` n'existe, l'application demande de créer l'administrateur.

### Mettre la base en place (sur un autre PC)

Vérifié : `skysense_schema_profa.sql` recrée une base **strictement identique** à la base de référence
(mêmes colonnes, types, contraintes et index) et s'exécute sans erreur.

1. Créer l'utilisateur Oracle `SKYSENSE` (droits `CONNECT`, `RESOURCE`).
2. Exécuter `database/skysense_schema_profa.sql` connecté en `SKYSENSE`
   (SQL Developer : ouvrir le fichier puis F5), ou en DBA : `sqlplus "/ as sysdba" @database/run_as_dba.sql`.
   **Attention : ce script supprime les tables SkySense existantes** — ne pas le lancer sur une base qui
   contient déjà vos données.
3. Une base existante créée avant les ajouts reçoit seulement `ajout_comptes_personnel.sql`
   puis `ajout_prix_titre_service.sql` (scripts non destructifs).
4. Créer une source ODBC système nommée `SKYSENSE` (pilote « Oracle in XE »).
   SQL Developer : hôte `localhost`, port `1521`, **SID `xe`** (pas `XEPDB1` avec Oracle XE 11.2).

## Connexion

- Seuls les membres de `PERSONNEL` qui ont un `ROLE` se connectent (email + mot de passe haché avec sel).
- `ADMIN` voit tout ; un responsable voit le tableau de bord et sa gestion
  (`STATION`, `EQUIPEMENT`, `MAINTENANCE`, `PERSONNEL`, `SERVICE`).
- Donner un accès à un responsable se fait dans la gestion **Staff** (section « Access »), qui n'est pas
  dans cette copie. API à utiliser : `Compte().enregistrerAcces(idPersonnel, role, motDePasse, &erreur)`.

## Intégrer une autre gestion (ex. Services)

1. Copier le dossier de la gestion (ex. `service/`) à la racine du projet.
2. `SkySense.pro` : ajouter le dossier à `INCLUDEPATH`, ses `.cpp` à `SOURCES` et ses `.h` à `HEADERS`.
3. `fenetre/skysensewindow.cpp`, bloc « Cette version ne contient que… » :
   - inclure l'en-tête de la vue (ex. `#include "serviceui.h"`) ;
   - créer la page (ex. `auto *servicesPage = new QWidget; setupServiceContent(servicesPage);`) ;
   - ajouter son nom à `moduleNames` (ex. `"Services"`) et la page dans le `if` de la boucle ;
   - ajouter sa recherche dans `moduleSearches` et son texte dans `searchPrompts`, **dans le même ordre**.
   La barre de navigation se construit à partir de `moduleNames` : rien d'autre à faire.
4. Le tableau de bord de cette version ne montre que les équipements. Pour y ajouter une gestion,
   lire ses données par sa méthode `statistiques()` dans `dashboard/dashboarddata.cpp` et ajouter
   sa carte dans `dashboard/dashboardui.cpp` (le projet complet contient la version à 5 gestions).
5. Si le projet de la gestion contient sa propre barre de navigation (`createSkySenseSidebar` dans
   `serviceui.cpp`), la supprimer : celle de `fenetre/sidebar.cpp` la remplace.
