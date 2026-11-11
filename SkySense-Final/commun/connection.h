#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

class Connection
{
public:
    static Connection& createInstance();


    static QSqlDatabase authenticationDatabase();
    bool isConnected() const { return db.isOpen(); }
    QSqlDatabase database() const { return db; }
    bool createConnection();
    bool createConnection(const QString &password);

private:
    QSqlDatabase db;

    Connection();
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};

#endif // CONNECTION_H
