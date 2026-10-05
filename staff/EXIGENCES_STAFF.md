# Gestion Staff — bilan du 5 octobre 2026

Ce bilan concerne le module Staff de Donia, inspecté dans `staffui.cpp`.
Il distingue les interactions qui fonctionnent des éléments de démonstration.
La retouche du 5 octobre retire les photos de personnes ; elle n'ajoute pas
de nouvelles fonctions métier.

## Utilisateurs de l'application

Proposition de rôles métier, cohérente avec les rôles déjà affichés dans SkySense :

| Utilisateur | Usage prévu |
| --- | --- |
| Administrateur | Gérer les membres du personnel et leurs habilitations. |
| Responsable des opérations | Consulter l'équipe, suivre les effectifs et exporter les listes. |
| Techniciens, responsables maintenance, analystes et support | Utiliser les modules autorisés selon leur métier ; leurs fiches sont gérées dans Staff. |

Cette répartition est à valider dans le cahier des charges de l'équipe. Dans ce
prototype, les intitulés sont des données de démonstration : le profil Alex Martin
est fixe et il n'existe ni connexion utilisateur ni contrôle effectif des accès
aux autres modules. Une personne affichée dans la liste est une fiche de personnel,
pas automatiquement un compte authentifié.

## Entité et CRUD individuel

**Étudiante : Donia. Entité : Staff / membre du personnel (`Member`).**
Attributs actuels : identifiant, nom, email, rôle, département, statut,
dernière connexion, téléphone, date d'arrivée et note.

| Opération | État dans le code |
| --- | --- |
| Create — ajouter | Oui : bouton Add Staff, formulaire avec validation des champs et email unique. |
| Read — consulter | Oui : liste, pagination, recherche et fiche détaillée. |
| Update — modifier | Oui : formulaire Edit Staff, rôle, coordonnées, statut et note. |
| Delete — supprimer | **Non : aucune action de suppression de membre.** |

Le CRUD n'est donc pas complet. Désactiver un membre conserve sa fiche et
modifie son statut : cela ne remplace pas la suppression. Les modifications
restent en mémoire et sont perdues à la fermeture. Le texte de la consigne fourni
ne précise pas le stockage attendu ; la persistance reste à définir avec l'équipe.

## Cinq fonctionnalités avancées

| Exigence | Présence et limite |
| --- | --- |
| 1. Tri / recherche | Recherche fonctionnelle sur identifiant, nom, email, rôle et département ; filtres rôle/statut. **Pas de tri interactif** : la flèche Status est décorative. Si la consigne exige tri ET recherche, le tri manque. |
| 2. Génération d'un document | Export PDF fonctionnel des résultats filtrés, sur toutes les pages, avec sauvegarde du fichier. |
| 3. Statistiques | Statistiques simples fonctionnelles : total des membres, actifs et inactifs calculés depuis la liste. Les tendances mensuelles, invitations et petites barres sont fixes. Ce n'est pas une analyse historique. |
| 4. Autre fonction : habilitations | Les droits Stations / Equipment / Maintenance / Services se modifient par membre et sont conservés pendant la session. **Démonstration partielle** : ces choix ne protègent pas réellement les autres modules et ne persistent pas. |
| 5. Autre fonction non redondante | **Pas de cinquième fonction avancée complète clairement démontrée.** Activation/désactivation existe, mais correspond à une modification de statut du CRUD ; elle ne doit pas être comptée automatiquement comme une fonction avancée distincte. |

Le journal d'activité, les affectations de stations et les compteurs de notes
affichent surtout des exemples fixes. L'édition d'une note fonctionne dans le
formulaire, mais ce champ fait partie de la modification de la fiche. Ces éléments
ne prouvent pas une cinquième fonctionnalité avancée opérationnelle.

Piste à valider pour une fonction distincte : un vrai journal d'audit des actions
Staff, alimenté automatiquement par les ajouts, modifications, suppressions et
changements de droits, avec date et auteur. Cette piste **n'est pas implémentée**
dans la version actuelle et ne constitue pas une nouvelle exigence imposée.

## Gestion des versions — travail individuel

- Dépôt : https://github.com/mohamedhaouaarii/SkySense
- Branche de Donia : `gestion-staff`, issue de `equipment`.
- Module isolé dans `staff/` ; sources Equipment préservées.
- Commit initial Staff : `e6f51e6c1df33db45d79d04286901fed4758edf3`.
- Consulter les commits suivants pour la retouche sans photos et ce bilan.
- Les fichiers générés, dossiers de compilation et caches IDE sont exclus.

Pour montrer le travail individuel : présenter la branche et l'historique
`git log --oneline -- staff/`, ainsi que les différences des commits Staff.

## Conclusion sur la conformité

La version actuelle fournit une interface Staff interactive et plusieurs
fonctions utilisables, mais elle ne remplit **pas encore intégralement**
« un CRUD complet + cinq fonctionnalités avancées non redondantes ».
Il manque au minimum la suppression et une cinquième fonction distincte solide.
Les habilitations doivent être distinguées d'un contrôle d'accès réel ; le tri
et la persistance dépendent du niveau demandé dans le cahier des charges.
