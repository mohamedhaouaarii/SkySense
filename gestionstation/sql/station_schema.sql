-- Run as the SKYSENSE Oracle user. SkySense also creates this table on first
-- connected launch when it does not already exist.
SET SERVEROUTPUT ON

DECLARE
    v_table_count NUMBER;
BEGIN
    SELECT COUNT(*)
      INTO v_table_count
      FROM USER_TABLES
     WHERE TABLE_NAME = 'STATION';

    IF v_table_count = 0 THEN
        EXECUTE IMMEDIATE q'~
            CREATE TABLE STATION (
                ID_STATION VARCHAR2(30 CHAR) NOT NULL,
                NOM_STATION NVARCHAR2(100) NOT NULL,
                TYPE_STATION NVARCHAR2(50),
                VILLE NVARCHAR2(100),
                REGION NVARCHAR2(100),
                NB_EQUIPEMENT NUMBER(6) DEFAULT 0 NOT NULL,
                DISPONIBILITE NUMBER(3) DEFAULT 0 NOT NULL,
                STATUT NVARCHAR2(20) DEFAULT 'Offline' NOT NULL,
                DATE_DERNIERE_MAJ DATE DEFAULT SYSDATE NOT NULL,
                CONSTRAINT PK_STATION PRIMARY KEY (ID_STATION),
                CONSTRAINT CK_STATION_EQUIP CHECK (NB_EQUIPEMENT >= 0),
                CONSTRAINT CK_STATION_AVAIL CHECK (DISPONIBILITE BETWEEN 0 AND 100),
                CONSTRAINT CK_STATION_STATUS
                    CHECK (STATUT IN ('Online', 'Maintenance', 'Offline'))
            )
        ~';
        DBMS_OUTPUT.PUT_LINE('Created STATION for SkySense.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('STATION already exists; no changes made.');
    END IF;
END;
/
EXIT;
