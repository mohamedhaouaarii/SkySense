-- =====================================================================
--  SkySense - schema Oracle conforme a SCHEMA_APPROUVE_PROFA.pdf
--  (identique a "UML diagrams (4).pdf").
--
--  A executer connecte en tant que SKYSENSE (SQL Developer ou SQL*Plus) :
--      sqlplus SKYSENSE@XE @skysense_schema_profa.sql
--  ou depuis un compte DBA :  ALTER SESSION SET CURRENT_SCHEMA = SKYSENSE;
--
--  ATTENTION : ce script SUPPRIME les tables SkySense existantes avant de recreer
--  le schema approuve. A n'executer que sur une base neuve ou a reinitialiser.
--
--  Choix a valider avec l'enseignante (voir database/NOTES_SCHEMA.md) :
--   * noms SERVICE : TYPE_SERVICE et DUREE_REELLE (diagramme) au lieu de
--     TYPE / DATE_REELLE (modele relationnel imprime) ;
--   * GERER traduit en table d'association (absente du relationnel imprime) ;
--   * REALISER cree une seule fois (imprime deux fois avec les cles inversees) ;
--   * durees DUREE_ESTIMEE / DUREE_REELLE exprimees en minutes ;
--   * types et longueurs des colonnes (non precises dans le PDF).
-- =====================================================================

SET DEFINE OFF
SET SERVEROUTPUT ON

-- ---------------------------------------------------------------------
-- 1. Suppression des anciens objets (ancien modele et reexecution)
-- ---------------------------------------------------------------------
BEGIN
    FOR obj IN (SELECT 'REALISER' AS name FROM DUAL UNION ALL SELECT 'AFFECTER' FROM DUAL
                UNION ALL SELECT 'GERER' FROM DUAL UNION ALL SELECT 'MAINTENANCE' FROM DUAL
                UNION ALL SELECT 'SERVICE' FROM DUAL UNION ALL SELECT 'EQUIPEMENT' FROM DUAL
                UNION ALL SELECT 'STATION' FROM DUAL UNION ALL SELECT 'PERSONNEL' FROM DUAL) LOOP
        BEGIN
            EXECUTE IMMEDIATE 'DROP TABLE ' || obj.name || ' CASCADE CONSTRAINTS PURGE';
            DBMS_OUTPUT.PUT_LINE('Table supprimee : ' || obj.name);
        EXCEPTION
            WHEN OTHERS THEN
                IF SQLCODE != -942 THEN RAISE; END IF; -- table inexistante
        END;
    END LOOP;
    FOR seq IN (SELECT 'SEQ_MAINTENANCE' AS name FROM DUAL UNION ALL SELECT 'SEQ_PERSONNEL' FROM DUAL
                UNION ALL SELECT 'SEQ_SERVICE' FROM DUAL) LOOP
        BEGIN
            EXECUTE IMMEDIATE 'DROP SEQUENCE ' || seq.name;
            DBMS_OUTPUT.PUT_LINE('Sequence supprimee : ' || seq.name);
        EXCEPTION
            WHEN OTHERS THEN
                IF SQLCODE != -2289 THEN RAISE; END IF; -- sequence inexistante
        END;
    END LOOP;
END;
/

-- ---------------------------------------------------------------------
-- 2. Entites
-- ---------------------------------------------------------------------
CREATE TABLE PERSONNEL (
    ID_PERSONNEL               NUMBER(10)     NOT NULL,
    CIN                        VARCHAR2(20)   NOT NULL,
    NOM                        VARCHAR2(80)   NOT NULL,
    PRENOM                     VARCHAR2(80)   NOT NULL,
    EMAIL                      VARCHAR2(254),
    TELEPHONE                  VARCHAR2(30),
    FONCTION                   VARCHAR2(80),
    STATUT                     VARCHAR2(30),
    DATE_ENTREE                DATE,
    DATE_DEBUT_INDISPONIBILITE DATE,
    DATE_FIN_INDISPONIBILITE   DATE,
    -- Compte de connexion (ajout valide avec l'equipe, voir NOTES_SCHEMA.md)
    MOT_DE_PASSE_HASH          VARCHAR2(128),
    SEL                        VARCHAR2(64),
    ROLE                       VARCHAR2(30),
    CONSTRAINT PK_PERSONNEL PRIMARY KEY (ID_PERSONNEL),
    CONSTRAINT UQ_PERSONNEL_CIN UNIQUE (CIN),
    CONSTRAINT CK_PERSONNEL_ROLE CHECK (ROLE IS NULL OR ROLE IN ('ADMIN', 'STATION', 'EQUIPEMENT', 'MAINTENANCE', 'PERSONNEL', 'SERVICE')),
    CONSTRAINT CK_PERSONNEL_COMPTE CHECK ((ROLE IS NULL AND MOT_DE_PASSE_HASH IS NULL AND SEL IS NULL)
        OR (ROLE IS NOT NULL AND MOT_DE_PASSE_HASH IS NOT NULL AND SEL IS NOT NULL AND EMAIL IS NOT NULL)),
    CONSTRAINT CK_PERSONNEL_INDISPO CHECK (DATE_FIN_INDISPONIBILITE IS NULL
        OR DATE_DEBUT_INDISPONIBILITE IS NULL OR DATE_FIN_INDISPONIBILITE >= DATE_DEBUT_INDISPONIBILITE)
);

CREATE UNIQUE INDEX UQ_PERSONNEL_EMAIL ON PERSONNEL (LOWER(EMAIL));

CREATE TABLE STATION (
    ID_STATION       VARCHAR2(30)  NOT NULL,
    NOM_STATION      VARCHAR2(100) NOT NULL,
    TYPE_STATION     VARCHAR2(50),
    VILLE            VARCHAR2(100),
    REGION           VARCHAR2(100),
    STATUT           VARCHAR2(30),
    DATE_MISE_A_JOUR TIMESTAMP,
    CONSTRAINT PK_STATION PRIMARY KEY (ID_STATION)
);

-- SUIVRE (EQUIPEMENT 1,1 - PERSONNEL 0,n) : cle etrangere obligatoire #ID_PERSONNEL.
CREATE TABLE EQUIPEMENT (
    IDENTIFIANT_EQUIPEMENT VARCHAR2(30)  NOT NULL,
    NUMERO_SERIE           VARCHAR2(80)  NOT NULL,
    TYPE_EQUIPEMENT        VARCHAR2(50),
    MODELE                 VARCHAR2(100),
    UNITE                  VARCHAR2(20),
    EMPLACEMENT            VARCHAR2(150),
    STATUT                 VARCHAR2(30),
    DATE_DERNIERE_MAJ      TIMESTAMP,
    PHOTO                  BLOB,
    ID_PERSONNEL           NUMBER(10)    NOT NULL,
    CONSTRAINT PK_EQUIPEMENT PRIMARY KEY (IDENTIFIANT_EQUIPEMENT),
    CONSTRAINT FK_EQUIPEMENT_PERSONNEL FOREIGN KEY (ID_PERSONNEL) REFERENCES PERSONNEL (ID_PERSONNEL)
);

CREATE TABLE MAINTENANCE (
    ID_MAINTENANCE NUMBER(10)    NOT NULL,
    CODE           VARCHAR2(30)  NOT NULL,
    TITRE          VARCHAR2(150) NOT NULL,
    DATE_PREVUE    TIMESTAMP,
    DATE_REALISEE  TIMESTAMP,
    PRIORITE       VARCHAR2(20),
    STATUT         VARCHAR2(30),
    DESCRIPTION    CLOB,
    COMPTE_RENDU   CLOB,
    CONSTRAINT PK_MAINTENANCE PRIMARY KEY (ID_MAINTENANCE),
    CONSTRAINT UQ_MAINTENANCE_CODE UNIQUE (CODE)
);

CREATE TABLE SERVICE (
    ID_SERVICE    NUMBER(10)    NOT NULL,
    TITRE         VARCHAR2(150) NOT NULL,
    TYPE_SERVICE  VARCHAR2(50),
    DESCRIPTION   CLOB,
    DATE_PREVUE   TIMESTAMP,
    DUREE_ESTIMEE NUMBER(6),
    DUREE_REELLE  NUMBER(6),
    PRIORITE      VARCHAR2(20),
    STATUT        VARCHAR2(30),
    PRIX          NUMBER(10,2),
    CONSTRAINT PK_SERVICE PRIMARY KEY (ID_SERVICE),
    CONSTRAINT UQ_SERVICE_TITRE UNIQUE (TITRE),
    CONSTRAINT CK_SERVICE_PRIX CHECK (PRIX IS NULL OR PRIX >= 0),
    CONSTRAINT CK_SERVICE_DUREES CHECK ((DUREE_ESTIMEE IS NULL OR DUREE_ESTIMEE > 0)
        AND (DUREE_REELLE IS NULL OR DUREE_REELLE > 0))
);

-- ---------------------------------------------------------------------
-- 3. Associations plusieurs-a-plusieurs (cle primaire composee)
-- ---------------------------------------------------------------------
CREATE TABLE GERER (
    ID_PERSONNEL NUMBER(10)   NOT NULL,
    ID_STATION   VARCHAR2(30) NOT NULL,
    CONSTRAINT PK_GERER PRIMARY KEY (ID_PERSONNEL, ID_STATION),
    CONSTRAINT FK_GERER_PERSONNEL FOREIGN KEY (ID_PERSONNEL) REFERENCES PERSONNEL (ID_PERSONNEL),
    CONSTRAINT FK_GERER_STATION FOREIGN KEY (ID_STATION) REFERENCES STATION (ID_STATION)
);

CREATE TABLE AFFECTER (
    ID_PERSONNEL NUMBER(10) NOT NULL,
    ID_SERVICE   NUMBER(10) NOT NULL,
    CONSTRAINT PK_AFFECTER PRIMARY KEY (ID_PERSONNEL, ID_SERVICE),
    CONSTRAINT FK_AFFECTER_PERSONNEL FOREIGN KEY (ID_PERSONNEL) REFERENCES PERSONNEL (ID_PERSONNEL),
    CONSTRAINT FK_AFFECTER_SERVICE FOREIGN KEY (ID_SERVICE) REFERENCES SERVICE (ID_SERVICE)
);

CREATE TABLE REALISER (
    ID_PERSONNEL   NUMBER(10) NOT NULL,
    ID_MAINTENANCE NUMBER(10) NOT NULL,
    CONSTRAINT PK_REALISER PRIMARY KEY (ID_PERSONNEL, ID_MAINTENANCE),
    CONSTRAINT FK_REALISER_PERSONNEL FOREIGN KEY (ID_PERSONNEL) REFERENCES PERSONNEL (ID_PERSONNEL),
    CONSTRAINT FK_REALISER_MAINTENANCE FOREIGN KEY (ID_MAINTENANCE) REFERENCES MAINTENANCE (ID_MAINTENANCE)
);

-- Index sur les cles etrangeres (jointures du tableau de bord).
CREATE INDEX IX_EQUIPEMENT_PERSONNEL ON EQUIPEMENT (ID_PERSONNEL);
CREATE INDEX IX_GERER_STATION ON GERER (ID_STATION);
CREATE INDEX IX_AFFECTER_SERVICE ON AFFECTER (ID_SERVICE);
CREATE INDEX IX_REALISER_MAINTENANCE ON REALISER (ID_MAINTENANCE);

-- ---------------------------------------------------------------------
-- 4. Donnees de demonstration (coherentes avec les cardinalites)
-- ---------------------------------------------------------------------
INSERT INTO PERSONNEL VALUES (1, '09876543', 'Martin', 'Alex', 'alex.martin@skysense.com', '+216 20 000 001', 'Operations Manager', 'Active', DATE '2022-01-14', NULL, NULL, NULL, NULL, NULL);
INSERT INTO PERSONNEL VALUES (2, '08765432', 'Chen', 'Sophie', 'sophie.chen@skysense.com', '+216 20 000 002', 'Station Technician', 'Active', DATE '2023-03-10', NULL, NULL, NULL, NULL, NULL);
INSERT INTO PERSONNEL VALUES (3, '07654321', 'Wilson', 'James', 'james.wilson@skysense.com', '+216 20 000 003', 'Maintenance Lead', 'Active', DATE '2021-09-05', NULL, NULL, NULL, NULL, NULL);
INSERT INTO PERSONNEL VALUES (4, '06543210', 'Davis', 'Emma', 'emma.davis@skysense.com', '+216 20 000 004', 'Data Analyst', 'Active', DATE '2024-02-14', NULL, NULL, NULL, NULL, NULL);
INSERT INTO PERSONNEL VALUES (5, '05432109', 'Kim', 'Ryan', 'ryan.kim@skysense.com', '+216 20 000 005', 'Technical Specialist', 'Active', DATE '2024-01-15', DATE '2026-11-02', DATE '2026-11-06', NULL, NULL, NULL);
INSERT INTO PERSONNEL VALUES (6, '04321098', 'Martinez', 'Olivia', 'olivia.martinez@skysense.com', '+216 20 000 006', 'Support Specialist', 'Inactive', DATE '2023-05-20', NULL, NULL, NULL, NULL, NULL);

INSERT INTO STATION VALUES ('ALP-03', 'Alpine Ridge', 'Mountain Station', 'Aspen', 'Rocky Mountains', 'Online', TIMESTAMP '2026-10-18 09:00:00');
INSERT INTO STATION VALUES ('COA-01', 'Coastal Bay', 'Coastal Station', 'Santa Cruz', 'Pacific Coast', 'Online', TIMESTAMP '2026-10-18 09:30:00');
INSERT INTO STATION VALUES ('DES-05', 'Desert View', 'Desert Station', 'Phoenix', 'Sonoran Desert', 'Maintenance', TIMESTAMP '2026-10-16 14:00:00');
INSERT INTO STATION VALUES ('PLA-08', 'Great Plains', 'Plains Station', 'Dodge City', 'Great Plains', 'Online', TIMESTAMP '2026-10-18 08:15:00');
INSERT INTO STATION VALUES ('VAL-07', 'Green Valley', 'Valley Station', 'Boise', 'Treasure Valley', 'Offline', TIMESTAMP '2026-10-12 11:20:00');
INSERT INTO STATION VALUES ('FOR-12', 'Pine Forest', 'Forest Station', 'Bend', 'Cascade Range', 'Online', TIMESTAMP '2026-10-17 16:45:00');

-- GERER : chaque membre du personnel gere au moins une station (1,n cote personnel).
INSERT INTO GERER VALUES (1, 'ALP-03');
INSERT INTO GERER VALUES (1, 'COA-01');
INSERT INTO GERER VALUES (2, 'COA-01');
INSERT INTO GERER VALUES (2, 'FOR-12');
INSERT INTO GERER VALUES (3, 'DES-05');
INSERT INTO GERER VALUES (4, 'PLA-08');
INSERT INTO GERER VALUES (5, 'VAL-07');
INSERT INTO GERER VALUES (6, 'PLA-08');

-- EQUIPEMENT (SUIVRE 1,1) : chaque equipement a exactement un responsable.
INSERT INTO EQUIPEMENT VALUES ('EQP-001', 'WS536-78421', 'Weather Station', 'Vaisala WXT536', NULL, 'North tower, rooftop', 'Online', TIMESTAMP '2026-10-18 21:45:00', NULL, 1);
INSERT INTO EQUIPEMENT VALUES ('EQP-002', 'ANE-20489', 'Anemometer', 'WindMaster Pro', 'm/s', 'Mast 2, 10 m', 'Maintenance', TIMESTAMP '2026-10-18 22:07:00', NULL, 2);
INSERT INTO EQUIPEMENT VALUES ('EQP-003', 'RG-20470', 'Rain Gauge', 'RG-200', 'mm', 'Open field, east', 'Offline', TIMESTAMP '2026-10-12 10:20:00', NULL, 5);
INSERT INTO EQUIPEMENT VALUES ('EQP-004', 'PYR-49024', 'Pyranometer', 'SMP10', 'W/m' || UNISTR('\00B2'), 'Solar platform', 'Online', TIMESTAMP '2026-10-18 10:41:00', NULL, 4);
INSERT INTO EQUIPEMENT VALUES ('EQP-005', 'BAR-11142', 'Barometer', 'PTB110', 'hPa', 'Instrument shelter', 'Maintenance', TIMESTAMP '2026-10-17 10:15:00', NULL, 3);
INSERT INTO EQUIPEMENT VALUES ('EQP-006', 'CAM-38001', 'Sky Camera', 'SkyCam 4K', NULL, 'Camera mast', 'Online', TIMESTAMP '2026-10-18 10:37:00', NULL, 2);
INSERT INTO EQUIPEMENT VALUES ('EQP-007', 'WS536-78422', 'Weather Station', 'Vaisala WXT536', NULL, 'Warehouse, shelf A', 'Online', TIMESTAMP '2026-10-18 09:32:00', NULL, 1);
INSERT INTO EQUIPEMENT VALUES ('EQP-008', 'RG-20471', 'Rain Gauge', 'RG-200', 'mm', 'Warehouse, shelf B', 'Online', TIMESTAMP '2026-10-17 11:40:00', NULL, 3);

INSERT INTO MAINTENANCE VALUES (1, 'MT-2026-001', 'Weather station calibration', TIMESTAMP '2026-10-31 19:58:00', NULL, 'High', 'Planned', 'Calibrate the sensors of the main weather station.', NULL);
INSERT INTO MAINTENANCE VALUES (2, 'MT-2026-002', 'Anemometer inspection', TIMESTAMP '2026-10-29 18:58:00', NULL, 'Medium', 'In Progress', 'Inspect bearings and cabling.', NULL);
INSERT INTO MAINTENANCE VALUES (3, 'MT-2026-003', 'Rain gauge cleaning', TIMESTAMP '2026-10-28 13:58:00', NULL, 'High', 'Planned', 'Clean the funnel and the tipping bucket.', NULL);
INSERT INTO MAINTENANCE VALUES (4, 'MT-2026-004', 'Barometer replacement check', TIMESTAMP '2026-10-20 09:00:00', TIMESTAMP '2026-10-20 11:00:00', 'Low', 'Completed', 'Check drift against the reference.', 'Drift within tolerance, no replacement needed.');
INSERT INTO MAINTENANCE VALUES (5, 'MT-2026-005', 'Camera lens cleaning', TIMESTAMP '2026-11-03 10:00:00', NULL, 'Low', 'Planned', 'Clean the dome and check focus.', NULL);

-- REALISER : chaque maintenance est realisee par au moins un membre du personnel (1,n).
INSERT INTO REALISER VALUES (3, 1);
INSERT INTO REALISER VALUES (2, 2);
INSERT INTO REALISER VALUES (4, 3);
INSERT INTO REALISER VALUES (3, 4);
INSERT INTO REALISER VALUES (5, 4);
INSERT INTO REALISER VALUES (1, 5);

INSERT INTO SERVICE VALUES (1, 'Routine Maintenance', 'Maintenance', 'Monthly routine check.', TIMESTAMP '2026-10-20 09:00:00', 180, NULL, 'High', 'In Progress', NULL);
INSERT INTO SERVICE VALUES (2, 'Sensor Calibration', 'Calibration', 'Calibrate temperature and humidity sensors.', TIMESTAMP '2026-10-21 09:00:00', 180, NULL, 'Medium', 'Scheduled', NULL);
INSERT INTO SERVICE VALUES (3, 'Camera Inspection', 'Inspection', 'Inspect the sky camera housing.', TIMESTAMP '2026-10-18 09:00:00', 180, 165, 'Low', 'Completed', NULL);
INSERT INTO SERVICE VALUES (4, 'Weather Station Repair', 'Repair', 'Repair the communication module.', TIMESTAMP '2026-10-16 09:00:00', 180, NULL, 'High', 'Overdue', NULL);
INSERT INTO SERVICE VALUES (5, 'Data Connectivity Check', 'Connectivity', 'Check the uplink and data gaps.', TIMESTAMP '2026-10-22 09:00:00', 180, NULL, 'Medium', 'Scheduled', NULL);
INSERT INTO SERVICE VALUES (6, 'Network Audit', 'Software', 'Review firmware versions.', TIMESTAMP '2026-11-04 14:00:00', 120, NULL, 'Low', 'Scheduled', NULL);

-- AFFECTER : 1,n des deux cotes -> chaque service et chaque membre du personnel a au moins une affectation.
INSERT INTO AFFECTER VALUES (3, 1);
INSERT INTO AFFECTER VALUES (2, 2);
INSERT INTO AFFECTER VALUES (1, 3);
INSERT INTO AFFECTER VALUES (4, 4);
INSERT INTO AFFECTER VALUES (5, 5);
INSERT INTO AFFECTER VALUES (6, 6);
INSERT INTO AFFECTER VALUES (2, 6);

COMMIT;

-- ---------------------------------------------------------------------
-- 5. Verification
-- ---------------------------------------------------------------------
SELECT 'STATION' AS TABLE_NAME, COUNT(*) AS LIGNES FROM STATION
UNION ALL SELECT 'PERSONNEL', COUNT(*) FROM PERSONNEL
UNION ALL SELECT 'EQUIPEMENT', COUNT(*) FROM EQUIPEMENT
UNION ALL SELECT 'MAINTENANCE', COUNT(*) FROM MAINTENANCE
UNION ALL SELECT 'SERVICE', COUNT(*) FROM SERVICE
UNION ALL SELECT 'GERER', COUNT(*) FROM GERER
UNION ALL SELECT 'AFFECTER', COUNT(*) FROM AFFECTER
UNION ALL SELECT 'REALISER', COUNT(*) FROM REALISER;

-- ---------------------------------------------------------------------
-- Compte admin : admin@gmail.com / sky123 (mot de passe hache, meme algorithme que compte.cpp)
-- ---------------------------------------------------------------------
INSERT INTO PERSONNEL (ID_PERSONNEL, CIN, NOM, PRENOM, EMAIL, FONCTION, STATUT, DATE_ENTREE, MOT_DE_PASSE_HASH, SEL, ROLE)
SELECT NVL(MAX(ID_PERSONNEL),0)+1, '00000000', 'Admin', 'SkySense', 'admin@gmail.com', 'Administrator', 'Active', SYSDATE,
       '717b98f0b5c79e4047380bce7017793b050996c7972a9bbf62e01c825e786f94',
       'a1b2c3d4e5f60718293a4b5c6d7e8f90', 'ADMIN'
FROM PERSONNEL;
COMMIT;

SELECT table_name FROM user_tables ORDER BY table_name;
SELECT EMAIL, ROLE FROM PERSONNEL WHERE ROLE IS NOT NULL;
