-- Comptes de connexion portes par PERSONNEL (aucune nouvelle table).
-- Un personnel sans ROLE ni mot de passe reste un simple membre du staff et ne peut pas se connecter.
-- Le mot de passe n'est jamais stocke en clair : seulement SHA-256 itere avec un sel propre a chaque compte.
-- Le premier compte ADMIN est cree depuis l'application (ecran de premier lancement).
-- Script non destructif : il ajoute des colonnes et des contraintes, il ne supprime aucune donnee.

ALTER TABLE PERSONNEL ADD (
    MOT_DE_PASSE_HASH VARCHAR2(128),
    SEL               VARCHAR2(64),
    ROLE              VARCHAR2(30)
);

ALTER TABLE PERSONNEL ADD CONSTRAINT CK_PERSONNEL_ROLE CHECK (
    ROLE IS NULL OR ROLE IN ('ADMIN', 'STATION', 'EQUIPEMENT', 'MAINTENANCE', 'PERSONNEL', 'SERVICE')
);

ALTER TABLE PERSONNEL ADD CONSTRAINT CK_PERSONNEL_COMPTE CHECK (
    (ROLE IS NULL AND MOT_DE_PASSE_HASH IS NULL AND SEL IS NULL)
    OR (ROLE IS NOT NULL AND MOT_DE_PASSE_HASH IS NOT NULL AND SEL IS NOT NULL AND EMAIL IS NOT NULL)
);

-- L'email sert d'identifiant de connexion : unique sans tenir compte des majuscules.
CREATE UNIQUE INDEX UQ_PERSONNEL_EMAIL ON PERSONNEL (LOWER(EMAIL));
