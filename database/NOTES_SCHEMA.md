# SkySense — alignement sur le schéma approuvé (29/10/2026)

Référence : `SCHEMA_APPROUVE_PROFA.pdf`, identique à `UML diagrams (4).pdf`.

## Base Oracle (compte SKYSENSE)

- Ancien schéma sauvegardé avant suppression : `backup/SKYSENSE_ancien_schema_2026-10-29.dmp`
  (restauration : `imp "'/ as sysdba'" fromuser=SKYSENSE touser=SKYSENSE file=...`).
- Nouveau schéma : `skysense_schema_profa.sql` (exécuté via `run_as_dba.sql`, 0 erreur).
- Tables : STATION, PERSONNEL, EQUIPEMENT, MAINTENANCE, SERVICE, GERER, AFFECTER, REALISER.
- Vérification des requêtes de l'application : `verify_app_queries.sql` (CRUD de test annulé par ROLLBACK).

| Table | Clé primaire | Clés étrangères / contraintes |
|---|---|---|
| STATION | ID_STATION | — |
| PERSONNEL | ID_PERSONNEL | CIN UNIQUE NOT NULL |
| EQUIPEMENT | IDENTIFIANT_EQUIPEMENT | ID_PERSONNEL NOT NULL → PERSONNEL (SUIVRE 1,1) |
| MAINTENANCE | ID_MAINTENANCE | CODE UNIQUE NOT NULL |
| SERVICE | ID_SERVICE | — |
| GERER | (ID_PERSONNEL, ID_STATION) | → PERSONNEL, → STATION |
| AFFECTER | (ID_PERSONNEL, ID_SERVICE) | → PERSONNEL, → SERVICE |
| REALISER | (ID_PERSONNEL, ID_MAINTENANCE) | → PERSONNEL, → MAINTENANCE |

## Points à valider avec l'enseignante

1. **GERER absent du modèle relationnel imprimé** : traduit en table d'association (présent dans le MCD).
2. **REALISER imprimé deux fois** avec les clés inversées : une seule table créée.
3. **SERVICE** : `type`/`type_service` et `date_reelle`/`duree_reelle` diffèrent entre MCD et relationnel.
   Choix retenu : `TYPE_SERVICE` et `DUREE_REELLE` (MCD). Unité des durées : minutes (non précisée).
4. Typos du relationnel non reprises : `nomstation` → `NOM_STATION`, `identifiant_equipment` →
   `IDENTIFIANT_EQUIPEMENT`, `date_dernier_maj` (MCD) / `date_derniere_maj` → `DATE_DERNIERE_MAJ`, `staut` → `STATUT`.
5. **Aucun lien MAINTENANCE–ÉQUIPEMENT** : le champ « Equipment » d'une intervention n'est plus enregistré
   (il reste visible mais désactivé en mode Oracle). À confirmer.
6. **Aucun lien SERVICE–STATION** : la station d'un service n'est plus enregistrée (champ désactivé en mode Oracle).
7. **Champs SERVICE hors schéma** : quantité, tarif unitaire et checklist restent dans l'interface
   (calcul du coût estimé, cases à cocher) mais **ne sont pas enregistrés**. À garder, dériver ou retirer ?
8. **Nombre d'équipements par station** (page Stations et tableau de bord) : dérivé par le chemin
   STATION –GERER– PERSONNEL –SUIVRE– EQUIPEMENT (COUNT DISTINCT). Ce n'est pas un lien direct.
9. **Cardinalités 1,n côté PERSONNEL** dans GERER et AFFECTER (tout personnel gère ≥1 station et a ≥1 service) :
   respectées dans les données de démo, mais pas imposables à la création d'un membre du personnel
   (il faut d'abord le créer). Lecture à confirmer (possible inversion de notation).
10. Types/longueurs non précisés dans le PDF : ID_STATION et IDENTIFIANT_EQUIPEMENT en VARCHAR2(30),
    autres identifiants NUMBER. NUMERO_SERIE n'est pas UNIQUE dans le PDF : unicité contrôlée seulement par l'application.
11. CODE de maintenance généré automatiquement (`MT-AAAA-NNN`) car le formulaire n'a pas de champ Code.
12. Suppression : une station gérée, ou un membre du personnel encore lié (SUIVRE/REALISER/AFFECTER/GERER),
    n'est pas supprimé ; un message demande de retirer les liens d'abord. Les lignes d'association
    d'une maintenance ou d'un service sont supprimées avec eux (même transaction).
13. Fonction « Find Replacement » (équipement de réserve sans responsable) : avec SUIVRE 1,1 il n'existe plus
    d'équipement « non suivi » en base, donc aucun candidat n'est proposé en mode Oracle.
14. **Comptes de connexion (ajout au schéma, à faire valider)** : demandés par l'équipe, ils sont portés par
    PERSONNEL au lieu d'une nouvelle table. Colonnes ajoutées : `MOT_DE_PASSE_HASH`, `SEL`, `ROLE`
    (`ADMIN`, `STATION`, `EQUIPEMENT`, `MAINTENANCE`, `PERSONNEL`, `SERVICE`) et index unique sur `LOWER(EMAIL)`.
    Le mot de passe n'est jamais stocké en clair (SHA-256 itéré + sel). Un personnel sans rôle ne peut pas se
    connecter. Script d'ajout non destructif : `ajout_comptes_personnel.sql`. Le premier ADMIN est créé
    depuis l'écran de premier lancement de l'application.
15. **SERVICE : titre unique et attribut PRIX (ajout au schéma, à faire valider)** : contrainte
    `UQ_SERVICE_TITRE UNIQUE (TITRE)` et colonne `PRIX NUMBER(10,2)` facultative, contrôlée par
    `CK_SERVICE_PRIX` (prix positif ou nul). Script d'ajout non destructif : `ajout_prix_titre_service.sql`.
