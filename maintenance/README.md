# SkySense Maintenance

This standalone Qt Widgets module keeps the Maintenance work separate from the root Equipment project. Open `maintenance/SkySense.pro` in Qt Creator with the Qt 6.7.3 MinGW 64-bit kit.

## Oracle setup

Create the 64-bit ODBC data source `SKYSENSE` for the Oracle service `localhost:1521/XEPDB1`, then run `maintenance_schema.sql` once as the `SKYSENSE` database user. The module stores interventions in `MAINTENANCE_INTERVENTION` through Qt SQL's ODBC driver.

Set `SKYSENSE_DB_PASSWORD` in your local environment before launching the app. The password is never stored in project files or Qt settings. The data source and user default to `SKYSENSE`; override them locally with `SKYSENSE_DB_DSN` and `SKYSENSE_DB_USER` if needed. If the password environment variable is absent, the app asks for it at startup.

Launch with `--skip-oracle` to open the temporary sample data without connecting to Oracle. Changes in preview mode are kept in memory only. If Oracle connects but the schema is missing, the module explains how to install it and blocks writes until the table can be loaded.

## Maintenance features

- Search, filter, sort, and review intervention status and due dates.
- Add, edit, and delete interventions in Oracle.
- Save the work report and recommended action checklist.
- Mark an intervention complete after entering a work report; completion records actual start and end dates.
- Export filtered lists and individual intervention reports to PDF.
- Review workload statistics, scheduling priority suggestions, and preventive maintenance dates.
