#include "connection.h"

#include <QInputDialog>
#include <QSqlQuery>
#include <QLineEdit>
#include <QCoreApplication>

Connection::Connection()
{
    // A preview also has one connection. Its authentication session stays separate
    // from the default connection used by normal module queries.
    if (QCoreApplication::instance()->property("oraclePreview").toBool())
        db = QSqlDatabase::addDatabase("QODBC", "SkySensePreviewAuthentication");
    else
        db = QSqlDatabase::addDatabase("QODBC");
}


Connection::~Connection()
{
    if (db.isOpen()) {
        db.close();
    }
}


Connection& Connection::createInstance()
{
    static Connection instance;
    return instance;
}

bool Connection::createConnection()
{
    if (db.isOpen()) {
        return true;
    }

    bool accepted = false;

    const QString password = QInputDialog::getText(
        nullptr,
        "Oracle connection",
        "Oracle password for SKYSENSE",
        QLineEdit::Password,
        QString(),
        &accepted
        );

    if (!accepted) {
        errorMessage.clear();
        return false;
    }

    return createConnection(password);
}

bool Connection::createConnection(const QString &password)
{
    errorMessage.clear();
    if (db.isOpen()) return true;
    db.setDatabaseName("SKYSENSE");
    db.setUserName("SKYSENSE");
    if (!db.open(db.userName(), password)) {
        const QString detail = db.lastError().text().section(QChar::Null, 0, 0).trimmed();
        if (detail.contains("ORA-01017")) {
            errorMessage = "ORA-01017: Oracle rejected the SKYSENSE login. Test the SKYSENSE "
                           "64-bit ODBC data source on this computer using the Oracle account "
                           "SKYSENSE and its password.";
        } else {
            errorMessage = "Could not open the SKYSENSE ODBC data source.\n" + detail;
        }
        qWarning().noquote() << errorMessage;
        return false;
    }
    QSqlQuery schema(db);
    if (!schema.exec("ALTER SESSION SET CURRENT_SCHEMA=SKYSENSE_MERIEM_STAFF") ||
        !schema.exec("SELECT STAFF_KEY FROM PERSONNEL WHERE 1=0")) {
        errorMessage = "Oracle accepted the login, but the integrated project schema is "
                       "unavailable. Check its installation and permissions.\n" +
                       schema.lastError().text().section(QChar::Null, 0, 0).trimmed();
        qWarning().noquote() << errorMessage;
        schema.finish();
        db.close();
        return false;
    }
    qDebug() << "Connexion Oracle etablie.";
    return true;
}
