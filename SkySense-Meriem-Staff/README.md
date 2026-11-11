# SkySense · Meriem + Staff

Ouvrir **SkySense.pro** dans Qt Creator, sélectionner **Desktop Qt 6.7.3 MinGW 64-bit** et cliquer sur **Exécuter**. Sur le poste de Donia, le projet et son schéma Oracle sont déjà installés.

La connexion utilise la source ODBC **SKYSENSE** et le mot de passe Oracle habituel. Se connecter ensuite avec le compte applicatif Staff existant. Sous **Staff**, sélectionner une fiche puis **Access** pour administrer les comptes. Le menu du profil contient aussi **Manage manager accounts**.

## Sources

Les modules Stations, Équipement, Maintenance, Services et Dashboard proviennent de `meriem-integfinal-7/11/26` au commit `350310f9a53537f66bba53480715471e1de78eb9`. Le dossier `staff/` contient les fonctionnalités de l'application locale SkySense-Integrated. Les six modules partagent une fenêtre, un menu, une session et une connexion Oracle.

## Oracle

L'application lit et écrit dans le schéma **SKYSENSE_MERIEM_STAFF**. Il contient une copie des fiches et des comptes Staff pris à l'installation ainsi que les données de démonstration de Meriem. Chaque personne conserve son identifiant Staff et possède une clé numérique pour les affectations des autres modules. Lorsqu'une adresse mail figure dans les deux jeux de données, les informations de la fiche Staff sont conservées. Les vérifications de disponibilité lisent les affectations des tables **AFFECTER** et **REALISER**.

Les deux applications évoluent indépendamment après cette copie. Pour installer sur un autre poste disposant du schéma Staff d'origine **SKYSENSE** (`PERSONNEL`, `PERSONNEL_SEQ`, `STAFF_ACCOUNT`), un DBA peut lancer `database/install_integration.sql` avec F5 sur Oracle XE 21c. Ce script lit les données source et crée un nouveau schéma sans compte de connexion. Il s'arrête si la destination existe déjà.

## Vérification

`SkySenseCheck.pro` construit le programme de vérification. Il lit le mot de passe Oracle sur son entrée standard et utilise le même schéma isolé que l'application. Pour vérifier la connexion et la navigation, définir `SKYSENSE_TEST_ADMIN_PASSWORD` dans l'environnement avec le mot de passe du compte administrateur de test. `SKYSENSE_TEST_ADMIN_EMAIL` permet de sélectionner son adresse si elle diffère de `admin@skysense.com`.

