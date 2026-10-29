-- SkySense - SERVICE module (SQL*Plus)
-- Run once:  sqlplus SKYSENSE/sky123@localhost:1521/XEPDB1
--            SQL> @service_sqlplus.sql
-- Safe to run again: the table is only created if it does not exist yet.

SET LINESIZE 220
SET PAGESIZE 100
SET SERVEROUTPUT ON

DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*) INTO n FROM USER_TABLES WHERE TABLE_NAME = 'SKYSENSE_SERVICES';
    IF n = 0 THEN
        EXECUTE IMMEDIATE '
        CREATE TABLE SKYSENSE_SERVICES (
            SERVICE_ID       VARCHAR2(30 CHAR)  NOT NULL,
            TITLE            VARCHAR2(150 CHAR) NOT NULL,
            SERVICE_TYPE     VARCHAR2(50 CHAR),
            STATION_CODE     VARCHAR2(30 CHAR),
            STATION_NAME     VARCHAR2(120 CHAR),
            ASSIGNEE         VARCHAR2(120 CHAR),
            SERVICE_DATE     DATE,
            START_TIME       VARCHAR2(8 CHAR),
            END_TIME         VARCHAR2(8 CHAR),
            PRIORITY         VARCHAR2(10 CHAR),
            STATUS           VARCHAR2(20 CHAR),
            DESCRIPTION_TEXT CLOB,
            CHECKLIST_TEXT   CLOB,
            CHECKLIST_DONE   CLOB,
            ACTUAL_END_TIME  VARCHAR2(8 CHAR),
            STATION_ID       NUMBER(10),
            PERSONNEL_ID     NUMBER(10),
            CONSTRAINT PK_SKYSENSE_SERVICES PRIMARY KEY (SERVICE_ID)
        )';
        DBMS_OUTPUT.PUT_LINE('Table SKYSENSE_SERVICES created.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Table SKYSENSE_SERVICES already exists - nothing to do.');
    END IF;
END;
/

-- Association "affecter" (teacher's schema): one service can be assigned to
-- SEVERAL people, and one person can have several services.
DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*) INTO n FROM USER_TABLES WHERE TABLE_NAME = 'SKYSENSE_AFFECTER';
    IF n = 0 THEN
        EXECUTE IMMEDIATE '
        CREATE TABLE SKYSENSE_AFFECTER (
            SERVICE_ID   VARCHAR2(30 CHAR) NOT NULL,
            PERSONNEL_ID NUMBER(10)        NOT NULL,
            CONSTRAINT PK_SKYSENSE_AFFECTER PRIMARY KEY (SERVICE_ID, PERSONNEL_ID),
            CONSTRAINT FK_AFFECTER_SERVICE FOREIGN KEY (SERVICE_ID)
                REFERENCES SKYSENSE_SERVICES (SERVICE_ID) ON DELETE CASCADE
        )';
        -- services created before this change keep their single assignee
        EXECUTE IMMEDIATE '
        INSERT INTO SKYSENSE_AFFECTER (SERVICE_ID, PERSONNEL_ID)
        SELECT SERVICE_ID, PERSONNEL_ID FROM SKYSENSE_SERVICES
        WHERE PERSONNEL_ID IS NOT NULL AND PERSONNEL_ID > 0';
        COMMIT;
        DBMS_OUTPUT.PUT_LINE('Table SKYSENSE_AFFECTER created.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Table SKYSENSE_AFFECTER already exists - nothing to do.');
    END IF;
END;
/

-- Contraintes au niveau de la BD (consigne de l'atelier) --------------------
-- Each constraint is added only if it does not exist yet. If old rows break a rule,
-- the message is printed and the script goes on (nothing is deleted).
DECLARE
    PROCEDURE add_ct(p_name VARCHAR2, p_def VARCHAR2) IS
        n NUMBER;
    BEGIN
        SELECT COUNT(*) INTO n FROM USER_CONSTRAINTS WHERE CONSTRAINT_NAME = p_name;
        IF n = 0 THEN
            EXECUTE IMMEDIATE 'ALTER TABLE SKYSENSE_SERVICES ADD CONSTRAINT ' || p_name || ' ' || p_def;
            DBMS_OUTPUT.PUT_LINE('Constraint ' || p_name || ' added.');
        END IF;
    EXCEPTION WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Constraint ' || p_name || ' NOT added: ' || SQLERRM);
    END;
BEGIN
    add_ct('CK_SERVICE_TITLE',    'CHECK (LENGTH(TRIM(TITLE)) > 0)');
    add_ct('CK_SERVICE_PRIORITY', 'CHECK (PRIORITY IN (''High'', ''Medium'', ''Low''))');
    add_ct('CK_SERVICE_STATUS',   'CHECK (STATUS IN (''Scheduled'', ''In Progress'', ''Completed'', ''Overdue''))');
END;
/

-- ===== Commands to CHECK the app from SQL*Plus (copy / paste them) =====
-- After Add / Edit / Delete in SkySense, run:
--
--   SELECT SERVICE_ID, TITLE, SERVICE_TYPE, STATION_CODE, ASSIGNEE,
--          TO_CHAR(SERVICE_DATE,'YYYY-MM-DD') AS SERVICE_DATE,
--          START_TIME, END_TIME, PRIORITY, STATUS, 
--   FROM SKYSENSE_SERVICES ORDER BY SERVICE_ID;
--
--   SELECT COUNT(*) FROM SKYSENSE_SERVICES;
--
-- Assignees of each service (one service -> several people):
--   SELECT SERVICE_ID, PERSONNEL_ID FROM SKYSENSE_AFFECTER ORDER BY SERVICE_ID, PERSONNEL_ID;
--
-- Free-text columns (CLOB) - to see the description and checklist:
--   SELECT SERVICE_ID, DBMS_LOB.SUBSTR(DESCRIPTION_TEXT, 200, 1) AS DESCRIPTION,
--          REPLACE(DBMS_LOB.SUBSTR(CHECKLIST_TEXT, 300, 1), CHR(10), ' | ') AS CHECKLIST,
--          DBMS_LOB.SUBSTR(CHECKLIST_DONE, 20, 1) AS DONE
--   FROM SKYSENSE_SERVICES ORDER BY SERVICE_ID;
--
-- Note: the app commits automatically (ODBC autocommit), so you see the change
-- immediately. If you ever INSERT/UPDATE/DELETE by hand in SQL*Plus, type COMMIT;
