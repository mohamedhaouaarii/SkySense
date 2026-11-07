-- Execute le script du schema dans le compte SKYSENSE depuis une session DBA locale
-- (sqlplus "/ as sysdba" @run_as_dba.sql), sans avoir besoin du mot de passe SKYSENSE.
WHENEVER SQLERROR EXIT SQL.SQLCODE ROLLBACK
ALTER SESSION SET CURRENT_SCHEMA = SKYSENSE;
SET ECHO OFF FEEDBACK ON PAGESIZE 100 LINESIZE 160
@@skysense_schema_profa.sql
EXIT
