# Gestion Staff — version corrigée du 5 octobre 2026

**Étudiante : Donia. Entité principale : Staff / membre du personnel.**
Cette livraison conserve C++, Qt Widgets 6.7.3, qmake et MinGW 64-bit.
Le module est développé sur la branche individuelle `gestion-staff`.

## Utilisateurs de l'application

| Acteur métier | Usage prévu dans SkySense |
| --- | --- |
| Administrateur | Gérer les fiches Staff et leurs habilitations. |
| Responsable des opérations | Consulter l'équipe, suivre les effectifs, exporter les listes et examiner les actions Staff. |
| Techniciens terrain, maintenance, analystes et support | Utiliser les modules correspondant à leur métier ; leurs fiches et habilitations sont gérées dans Staff. |

Le module Staff est destiné aux deux premiers acteurs. Leur authentification
relève du Login commun de l'équipe, qui n'est pas modifié ici. Cette version
autonome affiche le nom de session Windows comme **opérateur local** pour
identifier les actions, sans prétendre authentifier cet opérateur.

## Une entité, un CRUD complet

L'entité Member comprend : identifiant, nom, email, téléphone, rôle,
département, statut, date d'arrivée, dernière connexion d'exemple et note.

| Opération | Réalisation |
| --- | --- |
| Create | Add Staff avec validation, email unique et identifiant automatique non réutilisé après suppression. |
| Read | Liste paginée et fiche détaillée avec sélection. |
| Update | Edit Staff ; activation/désactivation dans Actions ; modification de la note. |
| Delete | Delete Staff avec confirmation ; suppression de la fiche et de ses habilitations. |

Les changements sont sauvegardés localement en JSON et relus au lancement.
L'historique reste consultable après suppression d'une fiche.

## Cinq fonctionnalités avancées distinctes

| Fonction | Comportement livré | Pourquoi elle est distincte |
| --- | --- | --- |
| 1. Tri et recherche | Recherche multichamp, filtres rôle/statut, tri croissant/décroissant sur les en-têtes, avant pagination. | Retrouver et organiser les fiches. |
| 2. Génération PDF | Rapport contenant tous les résultats filtrés dans l'ordre choisi. | Produire un document partageable. |
| 3. Statistiques | Total, actifs, inactifs, pourcentages et répartitions par département/rôle avec barres proportionnelles. | Analyser l'ensemble du personnel. |
| 4. Gestion des habilitations | Profils prédéfinis, droits personnalisés et sauvegarde par membre ; calcul des droits effectifs selon le statut. | Gérer des autorisations par module, au-delà du simple intitulé de rôle. |
| 5. Journal d'audit local | Enregistrement automatique des créations, modifications, suppressions, changements de statut et de droits ; recherche par membre, opérateur, action ou détail. | Assurer la traçabilité des opérations, y compris sur des membres supprimés. |

L'activation/désactivation appartient au CRUD et n'est pas comptée une
seconde fois parmi les cinq fonctions avancées.

## Règles et limites explicites

- Un email est obligatoire, valide et unique sans distinction de casse.
- Les nouveaux membres n'ont aucun droit de module par défaut.
- Les droits configurés sont conservés lors d'une désactivation ; les droits
  effectifs sont tous refusés tant que le membre est inactif.
- La fonction Staff::effectiveAccess expose cette règle aux autres modules.
  Cette livraison ne prétend pas protéger leurs fenêtres : ils restent à
  raccorder au Login et aux vérifications d'accès lors de l'intégration commune.
- Les statistiques utilisent les fiches enregistrées ; aucune tendance ni
  invitation fictive n'est présentée comme un calcul réel.
- Le journal est local, associé à l'opérateur Windows et non inviolable.
- Le stockage est un fichier JSON local, pas une base SQL ni un serveur.
  Le texte des exigences fourni n'impose pas de moteur de base de données.
- Les dates Last Login des fiches initiales restent des exemples. Les horodatages
  des nouvelles opérations Staff sont réels.
- Les modules Equipment, Stations, Maintenance, Services et Login sont préservés.

## Démonstration conseillée

1. Ajouter une fiche avec un email inédit ; essayer un doublon pour voir la validation.
2. Chercher la fiche, trier par nom puis par statut, et consulter les pages.
3. Modifier son département et ouvrir Statistics : vérifier les comptes et les barres.
4. Exporter la recherche en PDF.
5. Dans Edit Access, choisir un profil puis ajuster un droit.
6. Désactiver la fiche : les droits effectifs deviennent refusés dans Role & Permissions.
7. Consulter Activity Log : vérifier date, opérateur, membre et opérations.
8. Annuler une suppression, puis la confirmer. Chercher son ID dans le journal global.
9. Fermer et relancer : vérifier la persistance des fiches, droits et événements.

## Gestion des versions — travail individuel

Branche individuelle : `gestion-staff`. Les corrections prolongent le commit
`5ebe082` et sont regroupées dans un commit consacré au module Staff.

Les changements sont limités au dossier staff/. Le ZIP contient les sources,
ressources et tests, sans .git, données personnelles, exécutables, build ni cache IDE.
L'historique Git de la branche permet de suivre les étapes du travail individuel.
