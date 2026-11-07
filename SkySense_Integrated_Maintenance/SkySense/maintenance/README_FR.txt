Gestion Maintenance intégrée
============================

La page Maintenance est accessible depuis la barre latérale SkySense. Elle
utilise la session, le thème, la recherche globale et la connexion Oracle
partagés par l'application principale.

Le CRUD en ligne utilise les tables approuvées MAINTENANCE, REALISER et
PERSONNEL. L'ajout, la modification et la suppression sont enregistrés dans
Oracle; la suppression retire aussi les affectations REALISER dans la même
transaction. La recherche, les filtres, les exports et les statistiques restent
disponibles.

Le schéma approuvé ne relie pas une intervention à un équipement ou à une
station et ne comporte pas de fin planifiée, de début réel ou de checklist.
Les champs concernés sont signalés dans l'interface et ne sont pas écrits dans
la base. Le compte rendu est conservé dans COMPTE_RENDU.

Ouvrez SkySense.pro à la racine du projet intégré dans Qt Creator. Le schéma
Oracle et les consignes de connexion sont décrits dans database/NOTES_SCHEMA.md.
