#include "connection.h"

#include <QInputDialog>
#include <QLineEdit>


Connection::Connection()
{
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
        "Connexion Oracle",
        "Mot de passe Oracle du compte SKYSENSE :",
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
        qDebug() << "Connexion Oracle etablie.";
        return true;
    }

    qDebug() << "Echec de la connexion :"
             << db.lastError().text();

    return false;
}