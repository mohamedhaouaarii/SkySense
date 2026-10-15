#ifndef STAFFORACLE_H
#define STAFFORACLE_H
#include "staffdata.h"
#include <QSqlDatabase>
#include <memory>
class QWidget;

namespace Staff {
struct OracleConfig {
    QString driver, host, service, user, password;
    int port=1521;
};

QStringList oracleOdbcDrivers();
QString validateOracleConfig(const OracleConfig &config);
QString oracleConnectionString(const OracleConfig &config);

class OracleStore {
public:
    OracleStore();
    ~OracleStore();
    OracleStore(const OracleStore&)=delete;
    OracleStore &operator=(const OracleStore&)=delete;
    bool open(const OracleConfig &config,QString &error);
    bool test(QString &description,QString &error) const;
    bool schemaReady(bool &ready,QString &error) const;
    bool initialize(QString &error);
    bool load(State &state,QString &error);
    bool save(State &state,QString &error);
    QString description() const { return identity; }
private:
    QString connectionName,identity;
    QSqlDatabase database;
};

// The dialog's Test connection action never writes. Use Oracle may set up Staff tables.
bool connectOracleDialog(std::unique_ptr<OracleStore> &store,State &state,
                         const State *localData,QWidget *parent);
}
#endif
