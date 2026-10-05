SkySense — Staff | Qt 6.7.3

Cette version reprend l'organisation visuelle de la dernière image fournie :
navigation Stations / Equipment / Maintenance / Staff / Services, quatre cartes,
liste paginée de huit membres, Daniel Kim sélectionné, onglets, informations,
droits d'accès et activité récente.
Les photos de personnes sont retirées du tableau, de la fiche Staff et du profil
en haut à droite. Les noms sont alignés à gauche, sans avatar de remplacement.

DIMENSIONS ET STYLE
La fenêtre utilise les dimensions du dossier Equipment d'origine : 1646 × 932,
avec un minimum de 1100 × 700. Les proportions du contenu sont conservées à
toutes les tailles, avec des marges si nécessaire. Les points décoratifs rouge,
jaune et vert en haut à gauche sont retirés. Le dessin est organisé sur une grille de
1672 × 941 puis ajusté à la fenêtre via QGraphicsView.
La police est Segoe UI, comme dans equipmentui.cpp et equipment.qss d'origine.
Les différences de police, d'anticrénelage et d'échelle Windows peuvent influer
sur quelques pixels du rendu ; aucune identité pixel par pixel n'est garantie.

OUVRIR LE PROJET
1. Ouvrir le dossier staff du dépôt, ou extraire entièrement l'archive Staff.
2. Ouvrir le SkySense.pro de ce dossier dans Qt Creator : staff/SkySense.pro
   dans le dépôt, SkySense_Staff_Matched/SkySense.pro dans l'archive.
3. Choisir le kit Desktop Qt 6.7.3 MinGW 64-bit, configurer et compiler.
4. Lancer avec le bouton vert.

Ce dossier conserve la structure du projet Staff fourni. Les fichiers
Equipment à la racine du dépôt sont conservés ; chaque module possède
son propre exécutable qmake. Aucune navigation entre modules n'est ajoutée.

FICHIERS
staffui.cpp : interface, pictogrammes, tableau, détails et interactions.
styles/staff.qss : palette, bordures, boutons, champs et typographie.
skysensewindow.ui : fenêtre principale 1646 × 932.
skysensewindow.cpp : initialisation de la fenêtre et taille minimale.
resources.qrc : ressources intégrées à l'exécutable.

Les contrôles sont créés en C++, suivant la structure du projet d'origine.
Le mode Design de Qt Creator affiche donc seulement la fenêtre conteneur.
Ce n'est pas une image de l'interface placée en fond : le tableau, les textes,
les cartes, les onglets et les boutons sont rendus par Qt. La ressource
assets/reference.png fournit seulement les extraits décoratifs du logo
et des montagnes pour rester proche de la référence. Aucun portrait n'est affiché.

INTERACTIONS
Recherche, filtres rôle/statut, pagination, sélection, cases à cocher,
ajout/modification d'un membre, activation/désactivation, droits d'accès,
onglets et export PDF des résultats filtrés.
Les formulaires vérifient le nom, l'adresse email, son unicité (sans distinction
de casse), le rôle et le département. Annuler ne modifie pas le membre.
Les cases du tableau fonctionnent à la souris et au clavier. En l'absence de
résultat, les actions sur le membre sont désactivées. Les textes longs sont
abrégés visuellement, avec leur contenu complet dans une infobulle.
L'export PDF utilise une écriture atomique pour préserver un rapport existant
si l'enregistrement échoue.
Les autres sections de navigation affichent un message ; ce projet est
le module Staff autonome, à relier aux autres modules de votre application.

Les données sont de démonstration et gardées en mémoire jusqu'à la fermeture.
Les dates, tendances et invitations reproduisent la référence. Les quatre
membres de la seconde page sont ajoutés pour démontrer la pagination.
Les historiques, stations et notes sont illustratifs, sans connexion serveur.

BILAN DES EXIGENCES PÉDAGOGIQUES
Voir EXIGENCES_STAFF.md pour les utilisateurs proposés, l'entité Staff,
le CRUD et l'état réel des fonctionnalités avancées. Le CRUD est incomplet :
la suppression n'est pas implémentée. Les fonctions affichées ne constituent
pas toutes des fonctions métier avancées terminées.

CAPTURE DE L'APPLICATION
Depuis un terminal configuré pour Qt, dans le dossier de l'exécutable :
SkySense_Staff_Matched.exe --screenshot apercu.png
La capture montre le contenu Qt sans le cadre Windows, en 1646 × 932 pixels.

TESTS STAFF
tests/staff_tests.pro utilise Qt Test 6.7.3, sans changer la configuration de
l'application. Dans un terminal configuré pour le kit MinGW Qt 6.7.3, créer
un dossier de compilation séparé puis exécuter :
qmake chemin/vers/tests/staff_tests.pro
mingw32-make
release\staff_tests.exe

Les tests couvrent le rendu sans points décoratifs, le redimensionnement,
les filtres, la pagination, les cases, la validation, l'annulation, les droits,
les textes longs et la création d'un PDF.
