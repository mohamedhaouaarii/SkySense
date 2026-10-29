#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <QString>

struct OracleConnectionParameters {
    QString driverName;
    QString host;
    QString port;
    QString service;
    QString userName;
    QString password;
};

struct OracleConnectionResult {
    bool connected = false;
    QString message;
};

class DatabaseConnection {
public:
    static OracleConnectionResult testOracleConnection(const OracleConnectionParameters &parameters);
};

#endif
