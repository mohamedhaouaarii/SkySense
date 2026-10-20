#include "connection.h"

#include <QSqlError>

Connection &Connection::createInstance()
{
    static Connection instance;
    return instance;
}

Connection::Connection()
    : connectionName(QStringLiteral("SkySenseOracle")),
      db(QSqlDatabase::addDatabase(QStringLiteral("QODBC"), connectionName))
{
}

Connection::~Connection()
{
    closeConnection();
}

bool Connection::createConnection(const QString &dataSource,
                                  const QString &userName,
                                  const QString &password,
                                  QString *error)
{
    if (db.isOpen())
        db.close();

    db.setDatabaseName(dataSource.trimmed());
    db.setUserName(userName.trimmed());
    db.setPassword(password);

    if (db.open()) {
        if (error)
            error->clear();
        return true;
    }

    if (error)
        *error = db.lastError().text();
    return false;
}

void Connection::closeConnection()
{
    if (db.isValid() && db.isOpen())
        db.close();
}

bool Connection::isConnected() const
{
    return db.isValid() && db.isOpen();
}

QSqlDatabase Connection::database() const
{
    return db;
}
