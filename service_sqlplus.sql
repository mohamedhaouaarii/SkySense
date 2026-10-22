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
-- Free-text columns (CLOB) - to see the description and checklist:
--   SELECT SERVICE_ID, DBMS_LOB.SUBSTR(DESCRIPTION_TEXT, 200, 1) AS DESCRIPTION,
--          REPLACE(DBMS_LOB.SUBSTR(CHECKLIST_TEXT, 300, 1), CHR(10), ' | ') AS CHECKLIST,
--          DBMS_LOB.SUBSTR(CHECKLIST_DONE, 20, 1) AS DONE
--   FROM SKYSENSE_SERVICES ORDER BY SERVICE_ID;
--
-- Note: the app commits automatically (ODBC autocommit), so you see the change
-- immediately. If you ever INSERT/UPDATE/DELETE by hand in SQL*Plus, type COMMIT;
