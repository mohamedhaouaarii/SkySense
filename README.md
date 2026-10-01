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
