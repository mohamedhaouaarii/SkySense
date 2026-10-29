# SkySense - version integree avec Staff Oracle

La version integree utilisee et verifiee par Donia se trouve dans
[`SkySense-Integrated/`](SkySense-Integrated/).

**Projet a ouvrir dans Qt Creator :
[`SkySense-Integrated/SkySense.pro`](SkySense-Integrated/SkySense.pro).**

Cette version regroupe la navigation commune et les pages Dashboard, Stations,
Equipment, Maintenance, Staff et Services. Le module Staff utilise la classe
`Personnel` pour son CRUD Oracle et la connexion `Connection` en Meyers Singleton.

- [Installation, connexion et fonctionnement](SkySense-Integrated/README.md)
- [Scripts Oracle et identifiants Staff automatiques](SkySense-Integrated/database/)
- [Organisation du travail selon les ateliers](SkySense-Integrated/STAFF_ATELIER.md)
- [Validation technique et limites connues](SkySense-Integrated/VALIDATION.md)
- [Verification manuelle du CRUD et du PDF](SkySense-Integrated/docs/verification-20261029.md)

Compiler avec Qt 6.7.3 / MinGW 64 bits. La source ODBC et l'utilisateur Oracle sont
`SKYSENSE`; le service local est `localhost:1521/xepdb1`. Le mot de passe Oracle
est saisi au demarrage. Les scripts SQL sont fournis pour preparer la base sur
un autre poste; les fichiers internes de la base locale ne font pas partie du depot.

Les dossiers historiques restent disponibles : le projet a la racine correspond
aux anciennes instructions Equipment ci-dessous, et `staff/` a la version Staff
precedente. Pour le travail integre actuel, utiliser le projet indique ci-dessus.

---

# SkySense — interface Equipment

Projet de démonstration de l’interface de gestion des équipements météo SkySense, réalisé en C++ avec Qt Widgets.

## Ouvrir et lancer

1. Ouvrir `SkySense.pro` dans Qt Creator.
2. Sélectionner le kit **Desktop Qt 6.7.3 MinGW 64-bit**.
3. Lancer qmake, compiler, puis exécuter l’application.

Le programme s’ouvre directement sur l’écran Equipment. Le projet utilise qmake et conserve une seule interface Qt Designer : `skysensewindow.ui`.

## Contenu

- `equipmentui.cpp` / `equipmentui.h` : construction de l’écran Equipment et de la fenêtre Add Equipment.
- `skysensewindow.cpp` / `skysensewindow.h` : fenêtre principale de l’application.
- `skysensewindow.ui` : interface Qt Designer de la fenêtre principale.
- `styles/equipment.qss` : styles Qt de l’écran.
- `assets/` : logo SkySense et image d’arrière-plan.
- `resources.qrc` : ressources intégrées à l’application.
- `SkySense.pro` : configuration qmake.

## Fonctionnalités de la démonstration

La fenêtre **Add Equipment** permet de choisir un type d’équipement, une station et un statut. L’unité de mesure et l’identifiant sont proposés automatiquement. Le modèle et le numéro de série sont obligatoires ; le numéro de série doit être unique. Après validation, le nouvel équipement apparaît dans le tableau et les compteurs sont mis à jour.

## Limites

Les données affichées sont des exemples. Les équipements ajoutés restent en mémoire pendant l’exécution et disparaissent à la fermeture. Il n’y a pas de base de données, d’authentification ni de backend. La recherche, les filtres, l’export PDF et les autres commandes de navigation sont des éléments d’interface de démonstration.

## Gestion Staff

Le module Staff de Donia se trouve dans `staff/`. Ouvrir **`staff/SkySense.pro`**
dans Qt Creator avec le kit **Desktop Qt 6.7.3 MinGW 64-bit**, puis compiler
et exécuter. Le projet `SkySense.pro` à la racine continue d'ouvrir Equipment.

Staff reprend le projet fourni et ses dimensions de 1646 × 932. Il comprend
la recherche, les filtres, la pagination, l'ajout et la modification de membres,
la gestion de leurs droits dans la démonstration et l'export PDF. Les données
restent en mémoire ; les autres modules ne sont pas raccordés à cette fenêtre.
Les points décoratifs rouge, jaune et vert ont été retirés de l'écran Staff.

Voir [`staff/README.txt`](staff/README.txt) pour les fonctions et les limites,
et [`staff/VERIFICATION.txt`](staff/VERIFICATION.txt) pour la validation.
Les tests Qt Test se compilent séparément avec `staff/tests/staff_tests.pro`.
