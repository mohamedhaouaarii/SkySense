SkySense — Gestion Staff | Qt 6.7.3
Version corrigée du 5 octobre 2026 — branche gestion-staff.

OUVRIR ET LANCER
1. Extraire entièrement le ZIP.
2. Ouvrir SkySense_Staff_Matched/SkySense.pro dans Qt Creator.
   Dans la copie du dépôt, le projet est staff/SkySense.pro.
3. Choisir Desktop Qt 6.7.3 MinGW 64-bit.
4. Exécuter qmake, puis Rebuild Project et Run.
Le projet Equipment à la racine du dépôt reste indépendant et inchangé.

INTERFACE
Dimensions : 1646 × 932 ; minimum 1100 × 700 ; proportions conservées.
Police Segoe UI, palette SkySense, aucun portrait et aucun point macOS.
Les contrôles sont créés en C++. Le fichier .ui contient la fenêtre conteneur.
Le logo et les montagnes utilisent les extraits décoratifs de reference.png.

CRUD STAFF
- Add Staff : nom, email unique, rôle et département obligatoires.
- Liste et fiche : consultation, pagination, cases et détails.
- Edit Staff : coordonnées, rôle, département, statut et note.
- Delete Staff : bouton sous la fiche ou menu Actions, avec confirmation.
  Les habilitations du membre sont retirées ; son historique est conservé.
- L'identifiant est généré automatiquement et n'est pas réutilisé après suppression.

CINQ FONCTIONNALITÉS AVANCÉES
1. Recherche et tri : recherche par ID, nom, email, rôle ou département ;
   filtres rôle/statut ; clic sur les en-têtes ID à Last Login pour alterner
   croissant/décroissant. Le tri porte sur tous les résultats, avant pagination.
2. Export PDF : tous les résultats filtrés, dans leur ordre de tri.
3. Statistics : effectifs et pourcentages calculés ; répartition graphique
   par département ou rôle. Les fausses tendances et invitations sont retirées.
4. Edit Access : profils d'habilitation (administrateur, terrain, maintenance,
   aucun accès) ou droits personnalisés par module. Un membre inactif a
   toujours des droits effectifs refusés, même si ses droits configurés sont conservés.
5. Activity Log : actions réellement réalisées, avec date, opérateur local,
   membre et détail. Recherche possible ; l'historique conserve les membres supprimés.

SAUVEGARDE LOCALE
Les membres, habilitations, identifiants et événements sont enregistrés en JSON
dans le dossier AppLocalDataLocation de Qt, normalement :
%LOCALAPPDATA%/CoreVision/SkySense Staff/staff.json
L'infobulle de « Saved on this device » indique le chemin exact.
Les 12 fiches d'exemple sont créées uniquement au premier lancement.
Une liste vide reste vide après redémarrage.
Les écritures sont atomiques ; un verrou et un numéro de révision évitent
l'écrasement des changements d'une autre instance. En cas d'erreur, la
modification est annulée dans l'interface et un message explique le problème.
Un fichier invalide n'est jamais remplacé automatiquement.
Pour sauvegarder vos données, fermer l'application puis copier staff.json.

PÉRIMÈTRE
Il n'y a ni serveur ni module Login ajouté. L'opérateur affiché et enregistré
dans le journal correspond au nom de session Windows ; ce n'est pas une
identité authentifiée par SkySense. Le journal local n'est pas inviolable.
Staff::effectiveAccess fournit le calcul des droits effectifs ; les autres
modules devront l'utiliser lors de leur intégration avec le Login de l'équipe.
Last Login est une donnée initiale d'exemple, sans simulation de connexions.
La navigation vers les autres modules reste hors de cette livraison.

FICHIERS
staffui.cpp/.h : interface et interactions.
staffdata.cpp/.h : entité, statistiques, calcul des droits, validation et stockage.
styles/staff.qss : style ; assets/ : ressources.
EXIGENCES_STAFF.md : acteurs, CRUD, cinq fonctions et démonstration.
VERIFICATION.txt : résultats de compilation et de tests.

TESTS
Compiler tests/staff_tests.pro dans un dossier de build séparé :
qmake chemin/vers/tests/staff_tests.pro
mingw32-make
release\staff_tests.exe
Chaque test utilise un dossier temporaire ; les vraies données ne sont pas modifiées.

CAPTURE
SkySense_Staff_Matched.exe --screenshot apercu.png
Ce mode utilise des données temporaires et ne modifie pas votre fichier staff.json.

GESTION DES VERSIONS
Branche individuelle : gestion-staff.
Les corrections Staff prolongent le commit 5ebe082.
Les commits de ce module restent limités au dossier staff/ ; les données locales,
builds et caches IDE sont exclus du suivi Git.
