#ifndef SKYSENSE_GESTIONSTATION_CONNECTION_H
#define SKYSENSE_GESTIONSTATION_CONNECTION_H

#include <QSqlDatabase>
#include <QString>

namespace GestionStation {

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

} // namespace GestionStation

#endif // SKYSENSE_GESTIONSTATION_CONNECTION_H
