# Verification manuelle - 27 et 29 octobre 2026

Projet source publie depuis la version SkySense-Integrated de Donia. Les 107
fichiers sources, ressources, scripts et documents de la livraison ont ete
compares par SHA-256 a la copie validee : ils sont identiques avant ajout de
cette note et des exclusions Qt Creator dans .gitignore.

## Personnel / Staff - 27 octobre

- Connexion SQL Developer verifiee avec SELECT USER FROM dual : SKYSENSE.
- Ajout depuis la page Staff : membre test atelier, STAFF_ID STF-2026-025,
  retrouve dans SS_STAFF_MEMBER avec son email et le statut Active.
- Modification depuis l'interface : passage a Inactive, confirme par SELECT
  dans SQL Developer pour le meme STAFF_ID.
- Suppression effectuee par l'utilisatrice; elle confirme ensuite l'absence du
  membre apres fermeture et relance de l'application.

Cette verification manuelle complete les tests Oracle et Qt documentes dans
VALIDATION.md. La verification finale de suppression repose sur le retour de
l'utilisatrice apres redemarrage, sans nouvelle capture SQL de zero ligne.

## Export PDF Staff - 29 octobre

Le fichier SkySense-Staff.pdf produit par l'application a ete controle par
extraction du texte et rendu de la page : une page A4 paysage avec 12 membres,
les colonnes Staff ID, Name, Email, Role, Department et Status. Le tableau est
lisible et contient les 12 lignes. Aucune correction du code d'export n'a ete
necessaire : le bouton enregistre le PDF sans l'ouvrir automatiquement.

## Publication Git

Aucun changement fonctionnel n'est introduit lors de cette publication.
La generation du Makefile par qmake Qt 6.7.3 / MinGW 64 bits a reussi depuis
le nouveau chemin du depot; les 30 fichiers references par resources.qrc sont presents.
Les controles de compilation et tests du 22 octobre restent ceux decrits dans
VALIDATION.md; ils n'ont pas ete relances uniquement pour copier ces memes sources.
La base Oracle locale n'est pas modifiee par cette publication.
