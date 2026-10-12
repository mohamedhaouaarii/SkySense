#include "databaseconnection.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>

OracleConnectionResult DatabaseConnection::testOracleConnection(const OracleConnectionParameters &parameters)
{
    if (parameters.host.trimmed().isEmpty() || parameters.port.trimmed().isEmpty() ||
        parameters.service.trimmed().isEmpty() || parameters.userName.trimmed().isEmpty() ||
        parameters.password.isEmpty()) {
        return {false, "Enter host, port, service name, username and password before testing the connection."};
    }
    if (!QSqlDatabase::drivers().contains("QODBC")) {
        return {false, "Qt QODBC driver is unavailable. Check the Qt SQL installation."};
    }

    const QString connectionName = "skysense_oracle_connection_test";
    if (QSqlDatabase::contains(connectionName)) QSqlDatabase::removeDatabase(connectionName);
    QString error;
    {
        QSqlDatabase database = QSqlDatabase::addDatabase("QODBC", connectionName);
        const QString databaseName = QString("DRIVER={%1};DBQ=%2:%3/%4;UID=%5;PWD=%6;")
            .arg(parameters.driverName.trimmed(), parameters.host.trimmed(), parameters.port.trimmed(),
                 parameters.service.trimmed(), parameters.userName.trimmed(), parameters.password);
        database.setDatabaseName(databaseName);
        if (database.open()) {
            database.close();
            error.clear();
        } else {
            error = database.lastError().text();
        }
    }
    QSqlDatabase::removeDatabase(connectionName);
    if (!error.isEmpty()) return {false, "Connection failed: " + error};
    return {true, "Connection successful. Oracle is reachable from the SkySense Equipment module."};
}
