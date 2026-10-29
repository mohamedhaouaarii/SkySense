# Correction Staff selon les ateliers — 22 octobre 2026

Cette correction complète les recommandations relatives au Staff uniquement.
La connexion commune Meyers Singleton reste inchangée. Le SQL reste dans la classe
Personnel, avec des requêtes préparées, des paramètres liés et la validation C++.

## Utilisation

1. Ouvrir ce dossier avec `SkySense.pro` dans Qt Creator et recompiler.
2. Se connecter à Oracle puis ouvrir Staff.
3. Add Staff affiche le formulaire dans la même page. Le champ Staff ID est en
   lecture seule ; Oracle attribue son numéro lors de Save.
4. Edit Staff conserve le même identifiant. Save enregistre puis recharge la liste.
5. Delete Staff affiche le nom et l'identifiant du membre avant confirmation.
   Annuler revient à la liste sans suppression.

Le schéma SKYSENSE de ce poste est déjà préparé. Sur un autre poste, exécuter
`database/04_staff_auto_id.sql` comme SKYSENSE dans XEPDB1, après le schéma et
l'import éventuel. Le script est réexécutable : il ne remet pas la séquence à zéro.
Les anciens identifiants restent inchangés. La numérotation peut avoir des trous
après une erreur ou une transaction annulée ; c'est le comportement normal d'Oracle.

## Périmètre du code

- `personnel.h` et `personnel.cpp` : génération du Staff ID par la séquence Oracle.
- `staffui.cpp` : pages internes d'ajout, modification et confirmation de suppression ;
  correction du pointeur de statut Oracle utilisé lors du rafraîchissement après Save.
- `database/04_staff_auto_id.sql` : séquence et déclencheur propres à SS_STAFF_MEMBER.
- Tests Staff, documentation et captures de vérification.

Le tableau conserve son apparence et son fonctionnement QTableWidget. Les fenêtres
de statistiques, formation et permissions ne font pas partie de cette correction.
Le mode aperçu sans Oracle conserve son fonctionnement local. Les modules Stations,
Equipment, Maintenance et Services, la page d'entrée, les styles communs, la navigation
et la connexion ne sont pas modifiés.
