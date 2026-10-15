SkySense — Gestion Staff | Qt 6.7.3
Évolution Oracle du 15 octobre 2026 — branche gestion-staff.

CONNEXION ORACLE
Le bouton Database teste puis utilise Oracle via Qt QODBC. ORACLE_STAFF.md
décrit les paramètres, l'import initial, les tables et les tests réels effectués.
Le mot de passe est saisi localement à chaque session. Une fois Oracle choisi,
une reconnexion est nécessaire après redémarrage. Sans ce choix, le mode JSON
décrit plus bas reste disponible.
database/oracle_staff.sql contient le schéma Oracle des neuf tables Staff.
database/MODELES_STAFF.md décrit les modèles MCD/MLD/MPD de ce module et les
éléments à valider avec l'équipe pour la base commune. Les paramètres privés,
les mots de passe et les données enregistrées sur le poste ne sont pas versionnés.

OUVRIR ET LANCER
1. Extraire entièrement le ZIP.
2. Ouvrir staff/SkySense.pro dans Qt Creator.
   Le dossier du module s'appelle staff, dans le ZIP comme dans le dépôt.
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

FONCTIONS EXISTANTES CONSERVÉES
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

NOUVEAU MÉTIER INNOVANT — BESOINS EN FORMATION
Sélectionner un membre, puis cliquer sur Skills & Training, à côté de Edit Staff.
1. Vérifier les compétences requises pour sa fonction dans Edit role requirements.
   Les huit fonctions initiales disposent de modèles modifiables par le responsable.
   Une nouvelle fonction commence sans exigences : le responsable les définit.
2. Évaluer chaque compétence : 0 non acquise, 1 bases, 2 autonomie, 3 avancé.
   Not assessed signifie « pas encore évaluée », pas « compétence absente ».
3. Les écarts confirmés produisent des suggestions de domaines de formation :
   priorité haute pour un écart de 2 ou 3 niveaux, normale pour un écart de 1.
4. Save assessment conserve les évaluations et le référentiel modifié.
   Cancel abandonne les deux. Les modifications sont inscrites dans Activity Log.
Les exigences d'une fonction sont communes à tous ses membres. Un changement
de fonction conserve les compétences acquises et recalcule les besoins.
FORMATION_STAFF.md contient les règles et un scénario de démonstration.

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
La nouvelle version lit les anciens fichiers JSON (schéma 1), sans inventer de
compétences, puis écrit le schéma 2 à la prochaine sauvegarde réussie.
Un fichier au schéma 2 nécessite cette version de l'application ; garder une
copie de staff.json avant de revenir à une ancienne version.
Pour sauvegarder vos données, fermer l'application puis copier staff.json.

PÉRIMÈTRE
Oracle XE existant peut être utilisé ; aucun Login n'est ajouté. L'opérateur affiché et enregistré
dans le journal correspond au nom de session Windows ; ce n'est pas une
identité authentifiée par SkySense. Le journal local n'est pas inviolable.
Staff::effectiveAccess fournit le calcul des droits effectifs ; les autres
modules devront l'utiliser lors de leur intégration avec le Login de l'équipe.
Last Login est une donnée initiale d'exemple, sans simulation de connexions.
La navigation vers les autres modules reste hors de cette livraison.

FICHIERS
staffui.cpp/.h : interface et interactions.
staffdata.cpp/.h : entité, statistiques, calcul des droits, validation et stockage.
stafftraining.cpp/.h : référentiels de fonction et comparaison des compétences.
stafftrainingdialog.cpp : évaluation et édition des exigences de formation.
styles/staff.qss : style ; assets/ : ressources.
EXIGENCES_STAFF.md : acteurs, CRUD, fonctions conservées et métier de formation.
FORMATION_STAFF.md : nouveau métier innovant et mode d'emploi.
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
Branche individuelle : gestion-staff ; base de l'évolution : fc3753f.
L'évolution Skills & Training est regroupée dans un commit consacré à Staff.
Les commits de ce module restent limités au dossier staff/ ; les données locales,
builds et caches IDE sont exclus du suivi Git.
