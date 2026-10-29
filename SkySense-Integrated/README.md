# SkySense — projet intégré

Projet Qt Widgets 6.7.3, C++17, qmake, Windows 64 bits.

Cette version réunit la navigation, la connexion visuelle et les modules du ZIP
« travaille service avec eq main » avec le module fourni dans `staff.zip`.
Le projet à ouvrir dans Qt Creator est **SkySense.pro**, à la racine de ce dossier.

## Démarrer sur le poste de Donia

1. Ouvrir `SkySense.pro` dans Qt Creator.
2. Choisir **Desktop Qt 6.7.3 MinGW 64-bit**, puis compiler et exécuter.
3. Entrer le mot de passe Oracle du groupe dans la fenêtre de connexion.
4. La page d'entrée du projet de groupe s'affiche. Pour examiner tous les modules,
   utiliser l'identifiant local **admin** et un texte non vide dans son champ mot de passe.
   Cette page est la session de démonstration fournie dans le projet de groupe :
   elle ne vérifie pas de comptes applicatifs dans Oracle. La vraie connexion Oracle
   est établie séparément avant l'ouverture de cette page.
5. Utiliser la barre latérale commune : Dashboard, Stations, Equipment,
   Maintenance, Staff, Services.

## Connexion Oracle commune

| Paramètre | Valeur |
| --- | --- |
| Source ODBC utilisateur, 64 bits | SKYSENSE |
| Pilote sur ce poste | Oracle dans OraDB21Home1 |
| Serveur et service | //localhost:1521/xepdb1 |
| Utilisateur Oracle | SKYSENSE |
| Mot de passe | Saisi au démarrage, non enregistré dans les sources |

Sur ce poste, la préparation du compte et de la source ODBC est déjà faite.
Avec Oracle XE 21c, l'utilisateur local SKYSENSE est créé dans **XEPDB1** :
utiliser `//localhost:1521/xepdb1` à la place du raccourci `@XE` des captures du groupe.

`Connection` conserve le Meyers Singleton de l'atelier. Tous les modèles utilisent
la même connexion QODBC par défaut. Un échec de connexion arrête le démarrage au
lieu de présenter les données de démonstration comme des données enregistrées.

Pour un aperçu explicite sans Oracle : lancer avec `--preview` (ou `--skip-oracle`).
Les données Staff de cet aperçu sont isolées dans un dossier temporaire.

## Base de données

La base locale a été préparée dans **XEPDB1**, schéma **SKYSENSE**.
Les données Staff de DONIA ont été copiées dans ce schéma ; les originaux restent
dans DONIA. Le ZIP contient les scripts, pas une sauvegarde des fichiers internes d'Oracle.

- `database/00_user.sql` : préparation du compte par un administrateur, avec les
  rôles CONNECT, RESOURCE et DBA demandés dans les consignes fournies par le groupe.
- `database/01_schema.sql` : création des objets manquants en étant connecté comme
  SKYSENSE. Les objets et données existants sont conservés. Les références de rôles
  et départements permettent aussi de commencer avec une liste Staff vide.
- `database/02_copy_staff_from_donia.sql` : copie optionnelle de l'ancien schéma DONIA,
  à exécuter comme SYS/SYSTEM dans XEPDB1. Les clés déjà présentes sont conservées.
- `database/04_staff_auto_id.sql` : à exécuter comme SKYSENSE après la création du
  schéma et l'import éventuel. Ajoute la séquence et le déclencheur des identifiants
  Staff automatiques, sans modifier les membres existants. Déjà appliqué sur ce poste.

| Module | Stockage repris dans cette intégration |
| --- | --- |
| Staff | SS_STAFF_MEMBER et tables SS_STAFF_* ; classe Personnel |
| Equipment | EQUIPEMENT ; catalogue de noms STATION |
| Maintenance | MAINTENANCE_INTERVENTION et sa séquence |
| Services | SKYSENSE_SERVICES |
| Stations | Données de démonstration en mémoire, comme dans le ZIP de groupe |

Les listes Equipment, Maintenance et Services sont initialement vides dans le nouveau
schéma. Aucune donnée de production provenant d'un serveur distant n'a été importée.
Le catalogue STATION utilisé par Equipment est distinct de la page de démonstration Stations.
Les identifiants optionnels de station/personnel de Services sont conservés tels que
fournis ; cette fusion n'invente pas de clés étrangères entre les modèles des modules.

Le Staff intégré est celui de `staff.zip` : départements, rôles, informations détaillées,
statistiques et export PDF. Ses commandes Oracle Ajouter/Modifier/Supprimer/Afficher
sont conservées. Ses fonctions avancées de formation et de permissions gardent les
limites de persistance du fichier fourni ; elles ne sont pas transformées en nouvelle
gestion des droits applicatifs.

Les formulaires Oracle **Add Staff / Edit Staff** et la confirmation **Delete Staff**
s'affichent maintenant dans la page Staff. Le Staff ID est attribué par une séquence
Oracle lors de l'ajout et reste en lecture seule. Les erreurs de saisie restent dans
le formulaire ; Save revient à la liste et relit les données Oracle. Cancel revient
à la liste sans enregistrer. Voir `STAFF_ATELIER.md` pour le détail de cette correction.

## Compilation et ressources

Les images et feuilles de style sont embarquées dans `resources.qrc`.
Les binaires, dossiers build et préférences personnelles Qt Creator des ZIP d'origine
ne sont pas repris dans le ZIP source intégré.

Qt Multimedia est facultatif : s'il est présent, les sons utilisent QSoundEffect.
Sur le kit Windows fourni sans ce module, le même WAV est joué via l'API audio Windows.

Les anciennes notes des auteurs sont conservées dans `docs/original/` uniquement
comme références. Pour cette version, suivre ce README et l'ordre des scripts ci-dessus.

## Vérification

`tests/integration_tests.pro` compile les tests du projet intégré. Ils lisent le mot
de passe Oracle depuis l'entrée standard, jamais depuis un argument ou un fichier.
Ils testent le Singleton, les modèles Oracle et la navigation commune. Les mutations
de test sont isolées par des transactions annulées ; une valeur de séquence Oracle
peut être consommée par les tests Staff et Maintenance, même après annulation.

Les résultats de validation de cette livraison sont consignés dans `VALIDATION.md`.
