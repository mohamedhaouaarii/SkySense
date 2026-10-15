# Oracle — Gestion Staff

Vérification du 15 octobre 2026 : Qt Widgets / Qt SQL 6.7.3, qmake,
MinGW 11.2 64-bit, Oracle XE 21c et pilote Oracle ODBC 64-bit sous Windows.

## Utilisation

Compiler `SkySense.pro`, lancer Staff puis ouvrir **Database**.

| Paramètre de cette installation | Valeur |
| --- | --- |
| Pilote | Oracle dans OraDB21Home1 |
| Hôte / port | localhost / 1521 |
| Service | xepdb1 |
| Utilisateur du projet | donia |
| Mot de passe | Saisi localement à chaque session |

**Test connection** ouvre Qt QODBC et exécute SELECT sur DUAL sans modifier
les données. **Use Oracle** charge les tables du compte choisi. L'option de
création initialise les objets SS_STAFF manquants. L'import local est limité
à une base vierge : révision 0, aucun membre, aucun événement. Les données
Oracle existantes ne sont pas fusionnées ou remplacées automatiquement.

Le JSON local reste intact, mais n'est pas une sauvegarde actualisée d'Oracle.
Après sélection d'Oracle dans l'application normale, le mode est mémorisé :
au redémarrage, reconnecter via Database pour charger et modifier les données.
Le mot de passe n'est pas mémorisé dans QSettings. Les tests/captures utilisent
un stockage temporaire indépendant.

## Tables

`database/oracle_staff.sql` définit neuf tables : MEMBER (personnel), ROLE,
DEPT, SKILL, REQUIREMENT (exigences de fonction), ASSESSMENT (évaluations),
ACCESS, AUDIT et META, toutes préfixées SS_STAFF_. L'historique survit à la
suppression d'une fiche. Les lectures et écritures sont transactionnelles et
prennent le même verrou sur la ligne META ; une écriture obsolète est refusée.
Les modifications SQL externes doivent être coordonnées avec l'application,
car elles ne mettent pas automatiquement à jour sa révision.

Le compte applicatif possède ses tables, CREATE SESSION, CREATE TABLE et un
quota sur USERS. Ce script couvre Staff. Le modèle commun de tous les modules
reste à rapprocher des schémas validés par les autres membres de l'équipe.

## Correction du pilote ODBC

Le diagnostic a reproduit ORA-12154 avec l'ancienne valeur
`DBQ={//localhost:1521/xepdb1}`. Le même compte se connecte avec une valeur
DBQ sans accolades. Le code utilise maintenant un descripteur Oracle explicite,
sans accolades autour de DBQ, après validation des champs. Les accolades restent
autour du nom du pilote. L'option SQL_ATTR_CONNECTION_TIMEOUT, associée à IM006,
a été retirée ; le délai de login et CONNECT_TIMEOUT sont conservés.

Les diagnostics sont coupés au terminateur NUL fourni par le pilote afin de
supprimer les caractères parasites observés. Les lectures utilisent READ
COMMITTED et le verrou commun aux sauvegardes : cela évite ORA-01466 observé
avec READ ONLY immédiatement après création du schéma.

Références : [Oracle ODBC 21c](https://docs.oracle.com/en/database/oracle/oracle-database/21/adfns/odbc-driver.html),
[ORA-12154](https://docs.oracle.com/en/error-help/db/ora-12154/),
[ORA-01466](https://docs.oracle.com/en/error-help/db/ora-01466/).

## Résultats réellement obtenus

- Compilation Release application, Qt Test et vérificateur Oracle : succès.
- Qt Test : **31 réussis, 0 échec**. Les minuteries de test sont maintenant
  annulées avec leur fenêtre, évitant un accès à une fenêtre déjà détruite.
- Connexion Qt 6.7.3 QODBC vers Oracle XE / xepdb1 : succès.
- Création du schéma puis lecture immédiate : succès.
- Création, lecture, modification et suppression d'un membre : succès.
- Nom accentué, compétences, droits et refus d'accès pour un inactif : succès.
- Refus d'une écriture obsolète depuis une seconde connexion : succès.
- Refus d'un email dupliqué : succès.
- Service inexistant : ORA-12514 sans texte parasite.

Les essais automatisés Oracle ont utilisé un compte temporaire distinct de DONIA,
supprimé ensuite avec son fichier de paramètres privé. Donia a ensuite confirmé
dans SQL Developer l'import des 12 membres depuis sa fenêtre Qt, puis l'ajout du
membre testoracle (STF-2026-013) et sa modification vers Inactive : 13 membres.
Donia a ensuite confirmé la suppression depuis sa fenêtre corrigée. Le contrôle
en lecture seule du 15 octobre confirme 12 membres et l'absence de STF-2026-013.
Les modèles correspondant au schéma sont décrits dans
[Modèles Staff](database/MODELES_STAFF.md).

## Confirmation de suppression vide sous Windows

La capture de Donia montrait une fenêtre Delete Staff vide devant une seconde
fenêtre intégrée à la scène graphique. Les messages Staff utilisent désormais
QMessageBox avec DontUseNativeDialog et la véritable fenêtre de l'application
comme parent, au lieu du canevas QGraphicsProxyWidget. Cela s'applique aussi aux
messages d'erreur de sauvegarde. Le nom et l'identifiant du membre sont affichés
en texte brut ; No reste le choix par défaut et Échap annule.

Trois nouveaux cas vérifient le texte, les boutons visibles, les clics Yes/No et
Échap aux dimensions 1100 × 700, 1646 × 932 et 1920 × 1080. Les tests de suppression
et de conservation de l'historique restent réussis. Ces essais d'interface utilisent
des données temporaires et ne suppriment aucune fiche DONIA.
Référence : [options QMessageBox](https://doc.qt.io/qt-6/qmessagebox.html#options-prop).

Le vérificateur `tests/oracle_check.pro` accepte :
`oracle_check --config <fichier-INI-prive> --setup --exercise`.
Il crée puis supprime une fiche de test et conserve ses événements d'audit dans
le compte testé. `--import <staff.json>` est réservé à une base vierge.
Ne pas versionner paramètres privés, données, builds ou caches.
