#ifndef MAINTENANCE_CONNECTION_H
#define MAINTENANCE_CONNECTION_H

#include <QSqlDatabase>
#include <QString>

class Connection final
{
public:
    static Connection &createInstance();

    static QString defaultDataSource() { return QStringLiteral("SKYSENSE"); }
    static QString defaultUserName() { return QStringLiteral("SKYSENSE"); }

    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

    bool createConnection(const QString &dataSource,
                          const QString &userName,
                          const QString &password,
                          QString *error = nullptr);
    void closeConnection();
    bool isConnected() const;
    QSqlDatabase database() const;

private:
    Connection();
    ~Connection();

    const QString connectionName;
    QSqlDatabase db;
};

#endif // MAINTENANCE_CONNECTION_H
