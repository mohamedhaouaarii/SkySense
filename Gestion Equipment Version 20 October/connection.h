#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QString>

// Meyers Singleton, following the connection workshop.
class Connection final {
public:
    static Connection &createInstance();

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

#endif // CONNECTION_H
