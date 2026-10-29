-- SkySense - STATIONS + STAFF modules (SQL*Plus / SQL Developer)
-- Run once as SKYSENSE on XEPDB1:   sqlplus SKYSENSE/sky123@localhost:1521/XEPDB1
--                                    SQL> @stations_staff_sqlplus.sql
-- Safe to run again: tables are only created if missing, constraints only if missing.
-- Same tables as other_modules_schema.sql (SKYSENSE_STATIONS, SKYSENSE_STAFF).

SET SERVEROUTPUT ON

DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*) INTO n FROM USER_TABLES WHERE TABLE_NAME = 'SKYSENSE_STATIONS';
    IF n = 0 THEN
        EXECUTE IMMEDIATE '
        CREATE TABLE SKYSENSE_STATIONS (
            STATION_ID       VARCHAR2(20 CHAR)  NOT NULL,
            STATION_NAME     VARCHAR2(120 CHAR) NOT NULL,
            STATION_TYPE     VARCHAR2(80 CHAR),
            CITY_REGION      VARCHAR2(200 CHAR),
            EQUIPMENT_COUNT  NUMBER(5) DEFAULT 0 NOT NULL,
            AVAILABILITY_PCT NUMBER(5,2) DEFAULT 0 NOT NULL,
            STATUS           VARCHAR2(30 CHAR),
            LAST_UPDATE      DATE,
            CONSTRAINT PK_SKYSENSE_STATIONS PRIMARY KEY (STATION_ID),
            CONSTRAINT CK_SKYSENSE_STATION_AVAIL CHECK (AVAILABILITY_PCT BETWEEN 0 AND 100)
        )';
        DBMS_OUTPUT.PUT_LINE('Table SKYSENSE_STATIONS created.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Table SKYSENSE_STATIONS already exists.');
    END IF;

    SELECT COUNT(*) INTO n FROM USER_TABLES WHERE TABLE_NAME = 'SKYSENSE_STAFF';
    IF n = 0 THEN
        EXECUTE IMMEDIATE '
        CREATE TABLE SKYSENSE_STAFF (
            STAFF_ID             VARCHAR2(20 CHAR)  NOT NULL,
            FULL_NAME            VARCHAR2(150 CHAR) NOT NULL,
            EMAIL                VARCHAR2(254 CHAR),
            PROFESSIONAL_ROLE    VARCHAR2(120 CHAR),
            STATUS               VARCHAR2(30 CHAR),
            PHONE                VARCHAR2(40 CHAR),
            JOINED_DATE          DATE,
            UNAVAILABLE_FROM     DATE,
            UNAVAILABLE_TO       DATE,
            CONSTRAINT PK_SKYSENSE_STAFF PRIMARY KEY (STAFF_ID)
        )';
        DBMS_OUTPUT.PUT_LINE('Table SKYSENSE_STAFF created.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Table SKYSENSE_STAFF already exists.');
    END IF;
END;
/

-- Contraintes au niveau de la BD (consigne de l'atelier) -------------------
DECLARE
    PROCEDURE add_ct(p_table VARCHAR2, p_name VARCHAR2, p_def VARCHAR2) IS
        n NUMBER;
    BEGIN
        SELECT COUNT(*) INTO n FROM USER_CONSTRAINTS WHERE CONSTRAINT_NAME = p_name;
        IF n = 0 THEN
            EXECUTE IMMEDIATE 'ALTER TABLE ' || p_table || ' ADD CONSTRAINT ' || p_name || ' ' || p_def;
            DBMS_OUTPUT.PUT_LINE('Constraint ' || p_name || ' added.');
        END IF;
    EXCEPTION WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Constraint ' || p_name || ' NOT added: ' || SQLERRM);
    END;
BEGIN
    add_ct('SKYSENSE_STATIONS', 'CK_STATION_NAME',   'CHECK (LENGTH(TRIM(STATION_NAME)) > 0)');
    add_ct('SKYSENSE_STATIONS', 'CK_STATION_STATUS', 'CHECK (STATUS IN (''Online'', ''Maintenance'', ''Offline''))');
    add_ct('SKYSENSE_STATIONS', 'CK_STATION_EQUIP',  'CHECK (EQUIPMENT_COUNT >= 0)');

    add_ct('SKYSENSE_STAFF', 'CK_STAFF_NAME',   'CHECK (LENGTH(TRIM(FULL_NAME)) > 0)');
    add_ct('SKYSENSE_STAFF', 'CK_STAFF_STATUS', 'CHECK (STATUS IN (''Active'', ''Inactive''))');
    add_ct('SKYSENSE_STAFF', 'CK_STAFF_UNAVAIL',
           'CHECK ((UNAVAILABLE_FROM IS NULL AND UNAVAILABLE_TO IS NULL) OR (UNAVAILABLE_FROM IS NOT NULL AND UNAVAILABLE_TO >= UNAVAILABLE_FROM))');
END;
/

-- One email per staff member (case-insensitive)
DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*) INTO n FROM USER_INDEXES WHERE INDEX_NAME = 'UX_SKYSENSE_STAFF_EMAIL';
    IF n = 0 THEN
        EXECUTE IMMEDIATE 'CREATE UNIQUE INDEX UX_SKYSENSE_STAFF_EMAIL ON SKYSENSE_STAFF (LOWER(EMAIL))';
        DBMS_OUTPUT.PUT_LINE('Unique index on staff email created.');
    END IF;
EXCEPTION WHEN OTHERS THEN
    DBMS_OUTPUT.PUT_LINE('Unique email index NOT created: ' || SQLERRM);
END;
/

-- ===== Commands to CHECK the app from SQL*Plus =====
--   SELECT STATION_ID, STATION_NAME, STATION_TYPE, STATUS, AVAILABILITY_PCT FROM SKYSENSE_STATIONS ORDER BY STATION_ID;
--   SELECT STAFF_ID, FULL_NAME, EMAIL, PROFESSIONAL_ROLE, STATUS FROM SKYSENSE_STAFF ORDER BY STAFF_ID;
-- The app commits automatically (ODBC autocommit). If you INSERT/UPDATE/DELETE by hand, type COMMIT;
