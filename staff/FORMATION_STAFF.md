# Détection des besoins en formation — Gestion Staff

Évolution du 14 octobre 2026. C++, Qt Widgets 6.7.3, qmake, MinGW 64-bit.

## But métier

Aider le responsable du personnel à identifier des domaines de formation
à partir de la différence entre les compétences évaluées d'un membre et les
compétences attendues pour sa fonction. Les recommandations suivent des règles
explicites, sans service externe ni modèle d'intelligence artificielle.

## Utilisation

1. Sélectionner un membre dans Staff List.
2. Cliquer sur **Skills & Training**, à côté de **Edit Staff** (également dans Actions).
3. Consulter ou adapter **Edit role requirements**. Les changements du référentiel
   concernent tous les membres de la même fonction et restent en brouillon.
4. Choisir le niveau observé pour chaque compétence dans **Assessed level**.
5. Consulter **Recommended training**, qui se recalcule immédiatement.
6. Cliquer sur **Save assessment** pour sauvegarder le brouillon complet.
   **Cancel** abandonne les évaluations et les modifications du référentiel.

## Niveaux et priorité

| Valeur | Signification |
| --- | --- |
| Not assessed | Aucune évaluation disponible ; demander une évaluation. |
| 0 — Not acquired | Compétence évaluée comme non acquise. |
| 1 — Foundation | Bases acquises. |
| 2 — Independent | Travail en autonomie. |
| 3 — Advanced | Maîtrise avancée. |

Les exigences de fonction vont de 1 à 3. Pour chaque compétence évaluée :
`écart = niveau requis − niveau évalué`.

- Écart ≤ 0 : exigence satisfaite, pas de suggestion de formation.
- Écart = 1 : priorité normale.
- Écart = 2 ou 3 : priorité haute.
- Compétence non évaluée : à évaluer, sans conclure à un manque de compétence.

Les recommandations sont classées par écart décroissant puis par nom de compétence.
L'écran ne déclare pas un profil conforme lorsque des évaluations manquent ou
lorsque sa fonction n'a aucune exigence configurée. Le responsable valide les
domaines de formation suggérés ; aucun cours ni aucune inscription n'est créé.

## Démonstration

Utiliser la fiche d'exemple Sophia Chen, fonction **Station Technician**.
Le modèle initial attend le niveau 2 pour les trois compétences suivantes :

| Compétence | Niveau évalué pour cette démonstration | Résultat attendu |
| --- | --- | --- |
| Sensor installation | 2 | Exigence satisfaite. |
| Sensor calibration | 0 | Formation prioritaire, écart de 2. |
| Fault diagnosis | 1 | Formation de priorité normale, écart de 1. |

Enregistrer, fermer puis rouvrir la fenêtre : les évaluations sont conservées.
Passer Sensor calibration au niveau 2 : sa recommandation disparaît.
Les données initiales ne contiennent aucun niveau attribué automatiquement :
ces valeurs sont saisies uniquement pour la démonstration.

## Référentiels et données

Huit modèles modifiables correspondent aux fonctions déjà présentes : Station
Technician, Field Technician, Technical Specialist, Maintenance Lead, Data Analyst,
Operations Manager, Administrator et Support Specialist. Ce sont des exemples
de configuration à adapter, pas des normes professionnelles certifiées.

Une nouvelle fonction commence sans référentiel. Le responsable peut ajouter,
renommer ou retirer ses exigences, et modifier les niveaux minimums. Les doublons
de compétences sont refusés sans tenir compte des majuscules et des espaces répétés.

Les évaluations appartiennent au membre ; les exigences appartiennent à sa fonction.
Changer de fonction conserve les compétences et recalcule les besoins. Un membre
inactif peut être évalué pour préparer sa reprise ; cela ne modifie pas ses accès.

Le JSON utilise maintenant le schéma 2. Les fichiers du schéma 1 sont lus sans
modifier leur contenu à l'ouverture ; les anciens membres sont non évalués.
Le schéma 2 est écrit à la prochaine sauvegarde réussie. Les coordonnées,
habilitations, identifiants et historiques existants sont conservés.

La sauvegarde reste atomique et protégée contre les écritures concurrentes.
Une erreur annule le changement dans l'interface. Le journal local enregistre
l'opérateur, le membre, la date et les modifications d'évaluation/référentiel.

## Livraison

Sources dans le dossier `staff/`, projet `staff/SkySense.pro`.
Les autres modules et la structure du projet sont préservés. Branche de livraison :
`gestion-staff`, à partir du commit `fc3753f`.
