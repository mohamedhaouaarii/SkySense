#include "connection.h"

#include <QInputDialog>
#include <QSqlQuery>
#include <QLineEdit>
#include <QCoreApplication>

namespace {
QString authenticationName() {
    return QCoreApplication::instance()->property("oraclePreview").toBool()
        ? QStringLiteral("SkySensePreviewAuthentication") : QString(QSqlDatabase::defaultConnection);
}
}
QSqlDatabase Connection::authenticationDatabase() {
    const auto name=authenticationName();
    return QSqlDatabase::contains(name)?QSqlDatabase::database(name,false):QSqlDatabase{};
}


Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC",authenticationName());
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
        return false;
    }

    return createConnection(password);
}

bool Connection::createConnection(const QString &password)
{
    if (db.isOpen()) return true;
    db.setDatabaseName("SKYSENSE");
    db.setUserName("SKYSENSE");
    db.setPassword(password);

    if (db.open()) {
        QSqlQuery schema(db);
        if (!schema.exec("ALTER SESSION SET CURRENT_SCHEMA=SKYSENSE_MERIEM_STAFF") ||
            !schema.exec("SELECT STAFF_KEY FROM PERSONNEL WHERE 1=0")) {
            qWarning() << "Integrated schema unavailable:" << schema.lastError().text();
            db.close();return false;
        }
        qDebug() << "Connexion Oracle etablie.";
        return true;
    }

    qDebug() << "Echec de la connexion :"
             << db.lastError().text();

    return false;
}
