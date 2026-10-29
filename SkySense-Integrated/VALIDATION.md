# Validation de l'intégration — 22 octobre 2026

## Sources

- `staff.zip` : SHA-256 `E4876B068D9FFA754414EB66E250D713C74960317E712DFF146A7C975AA83528`
- `travaille service avec eq main.zip` : SHA-256 `8A66EE68B1139CBD9A6BAA8F8D9BC4AA741489544D486D6854683B9DD5BD751A`

Les archives d'entrée restent intactes. Le projet intégré réutilise la fenêtre,
la navigation, le thème et les modules du groupe ; sa page Staff est remplacée
par celle de `staff.zip`, avec ses classes Personnel et StaffTraining.
Les binaires et configurations personnelles de compilation des auteurs ont été exclus.

## Compilation et tests exécutés

- Windows 11, Qt 6.7.3 Widgets, qmake, MinGW 64 bits.
- Compilation de `SkySense.pro` réussie, code de sortie 0.
- Compilation de `tests/integration_tests.pro` réussie, code de sortie 0.
- Suite complète : **8 résultats PASS, 0 échec** (6 cas fonctionnels + initialisation/nettoyage).
- Après les derniers ajustements de présentation, nouveau passage du cas de navigation :
  **3 résultats PASS, 0 échec** avec initialisation/nettoyage.

Les tests utilisent réellement QODBC et Oracle XE 21c, le schéma SKYSENSE et le
service XEPDB1. Le mot de passe est lu depuis l'entrée standard du programme de test.

| Cas | Vérification |
| --- | --- |
| Singleton et schéma | Une seule connexion Qt ; utilisateur SKYSENSE ; conteneur XEPDB1 ; tables présentes |
| Staff | Ajouter, modifier, afficher et supprimer ; texte accentué ; suppression inexistante refusée |
| Equipment | Ajouter, modifier, afficher et supprimer ; aller-retour du BLOB photo |
| Services | Ajouter, modifier, afficher et supprimer ; description accentuée et checklist |
| Maintenance | Ajouter avec séquence, modifier, afficher et supprimer ; dates facultatives |
| Interface | Six pages accessibles à la session admin ; Staff Oracle à neuf colonnes ; recherche commune ; modes clair/sombre ; navigation selon le rôle du prototype |

Chaque test de modèle ouvre une transaction annulée à son terme et utilise ses
propres lignes. Les données existantes ne sont pas supprimées. Une séquence Oracle
peut avancer même si la transaction est annulée.

Les sorties originales sont dans `docs/integration-results.txt` et `docs/ui-results.txt`.
Elles contiennent encore des avertissements QPainter pendant le rendu de l'interface
du groupe (painter déjà actif / translate inactif). Les assertions de navigation passent ;
les captures des pages ont été contrôlées visuellement. Ces avertissements graphiques
ne constituent pas une validation sans avertissement de l'ensemble du code fourni.

## État Oracle vérifié

- Compte SKYSENSE créé dans XEPDB1 avec les rôles CONNECT, RESOURCE et DBA demandés.
- Source ODBC utilisateur 64 bits SKYSENSE : pilote Oracle dans OraDB21Home1,
  serveur `//localhost:1521/xepdb1`, utilisateur SKYSENSE.
- 13 tables créées dans SKYSENSE.
- 12 membres Staff copiés depuis DONIA ; DONIA conserve ses 12 membres.
- Rôles et indicateurs de formation, départements, compétences, accès, exigences
  et historique copiés ; les indicateurs des rôles correspondent à ceux de DONIA.
- EQUIPEMENT, SKYSENSE_SERVICES et MAINTENANCE_INTERVENTION restent vides après tests.
- Deuxième exécution de `01_schema.sql` réussie, sans recréer de tables ni dupliquer
  les références ; les 12 membres sont conservés.

## Périmètre et limites

La page Stations conserve le fonctionnement en mémoire de l'archive du groupe.
Le catalogue Oracle STATION utilisé par Equipment ne constitue pas une synchronisation
avec cette page. Les identifiants optionnels de station/personnel de Services restent
ceux de son modèle d'origine ; aucune relation SQL intermodule supplémentaire n'est créée.

La page d'entrée conserve une session locale de démonstration, indépendante de
l'authentification Oracle. Elle ne constitue pas une gestion d'utilisateurs sécurisée.
Les fonctions avancées Staff gardent leurs limites Oracle d'origine. Les exports PDF,
les statistiques avancées, les recommandations et l'audio n'ont pas tous été testés
fonctionnellement dans cette intégration.

Le ZIP contient les sources, ressources, tests et scripts SQL. Il ne contient ni
installation Oracle/Qt, ni sauvegarde physique de la base, ni mot de passe enregistré.

## Complément Staff selon les ateliers — même date

Corrections limitées à `personnel.h`, `personnel.cpp` et `staffui.cpp`, avec le script
`database/04_staff_auto_id.sql`, des tests Staff et cette documentation.
Les sources de la connexion, de la fenêtre commune, du thème, de l'authentification
et des autres modules ont été comparées par SHA-256 à la copie prise avant cette
correction : aucune modification.

- Compilation de l'application et des tests : code de sortie 0.
- Nouvelle sélection de tests : **7 PASS, 0 échec** (5 cas + initialisation/nettoyage).
- Cas : Singleton/schéma, CRUD Personnel existant, nouveaux identifiants automatiques,
  CRUD depuis les boutons de la page Staff, navigation commune et modes clair/sombre.
- Deux ajouts automatiques donnent des identifiants distincts ; modifier conserve
  l'identifiant ; un email invalide ou déjà présent est refusé. Un INSERT SQL sans
  STAFF_ID reçoit aussi un identifiant via le déclencheur Oracle.
- Test de l'interface : formulaire dans la page, identifiant en lecture seule,
  erreur de saisie visible, annulation sans ajout, sauvegarde vérifiée dans Oracle,
  modification vérifiée dans Oracle, annulation et confirmation de suppression.
- Lecture des 12 membres d'origine après annulation des transactions de test.
- Deuxième passage du script 04 : réussi, même valeur de séquence, 12 identifiants
  d'origine conservés, séquence et déclencheur marqués VALID.
- Captures Add Staff, Edit Staff sombre, Delete Staff et retour à la liste contrôlées.

Résultats : `docs/staff-workshop-results.txt`, `docs/staff-migration-results.txt`.
Captures : `docs/staff-workshop/`. L'avertissement ORA-00001 dans le test d'identifiant
est attendu : il vérifie le refus d'un email dupliqué. Les avertissements QPainter
déjà présents dans l'interface commune restent inchangés. Les autres modèles CRUD
n'ont pas été modifiés ni réexécutés pour cette correction Staff.
