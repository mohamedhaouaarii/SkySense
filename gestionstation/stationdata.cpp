#include "stationdata.h"

#include "connection.h"

#include <QDate>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace GestionStation {

namespace {
QSqlDatabase database(QString *error)
{
    Connection &connection = Connection::createInstance();
    if (!connection.isConnected()) {
        if (error)
            *error = QStringLiteral("Oracle is not connected. Connect through the SKYSENSE ODBC data source first.");
        return {};
    }
    return connection.database();
}

void setQueryError(QString *error, const QSqlQuery &query)
{
    if (error)
        *error = query.lastError().text();
}

QVariant nullableString(const QString &value)
{
    return value.trimmed().isEmpty()
        ? QVariant(QMetaType::fromType<QString>())
        : QVariant(value.trimmed());
}

void bindRecord(QSqlQuery &query, const StationRecord &record)
{
    const QStringList location = record.cityRegion.split('\n');
    query.bindValue(QStringLiteral(":id"), record.id.trimmed().toUpper());
    query.bindValue(QStringLiteral(":name"), record.name.trimmed());
    query.bindValue(QStringLiteral(":type"), nullableString(record.type));
    query.bindValue(QStringLiteral(":city"), nullableString(location.value(0)));
    query.bindValue(QStringLiteral(":region"), nullableString(location.value(1)));
    query.bindValue(QStringLiteral(":equipment"), record.equipment);
    query.bindValue(QStringLiteral(":availability"), record.availability);
    query.bindValue(QStringLiteral(":status"), record.status.trimmed());
}

bool hasStationTable(const QSqlDatabase &db, bool *exists, QString *error)
{
    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "SELECT COUNT(*) FROM USER_TABLES WHERE TABLE_NAME=:tableName"))) {
        setQueryError(error, query);
        return false;
    }
    query.bindValue(QStringLiteral(":tableName"), QStringLiteral("STATION"));
    if (!query.exec() || !query.next()) {
        setQueryError(error, query);
        return false;
    }
    *exists = query.value(0).toInt() > 0;
    return true;
}

bool validateStationTable(const QSqlDatabase &db, QString *error)
{
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT ID_STATION, NOM_STATION, TYPE_STATION, VILLE, REGION, "
            "NB_EQUIPEMENT, DISPONIBILITE, STATUT, DATE_DERNIERE_MAJ "
            "FROM STATION WHERE 1=0"))) {
        if (error) {
            *error = QStringLiteral(
                "The Oracle STATION table is missing columns required by SkySense. "
                "Run sql/station_schema.sql as the SKYSENSE user, then reopen the app.\n\n%1")
                         .arg(query.lastError().text());
        }
        return false;
    }
    if (error)
        error->clear();
    return true;
}
}

bool StationModel::ensureSchema(QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    bool exists = false;
    if (!hasStationTable(db, &exists, error))
        return false;

    if (!exists) {
        QSqlQuery query(db);
        if (!query.exec(QStringLiteral(
                "CREATE TABLE STATION ("
                "ID_STATION VARCHAR2(30 CHAR) NOT NULL, "
                "NOM_STATION NVARCHAR2(100) NOT NULL, "
                "TYPE_STATION NVARCHAR2(50), "
                "VILLE NVARCHAR2(100), "
                "REGION NVARCHAR2(100), "
                "NB_EQUIPEMENT NUMBER(6) DEFAULT 0 NOT NULL, "
                "DISPONIBILITE NUMBER(3) DEFAULT 0 NOT NULL, "
                "STATUT NVARCHAR2(20) DEFAULT 'Offline' NOT NULL, "
                "DATE_DERNIERE_MAJ DATE DEFAULT SYSDATE NOT NULL, "
                "CONSTRAINT PK_STATION PRIMARY KEY (ID_STATION), "
                "CONSTRAINT CK_STATION_EQUIP CHECK (NB_EQUIPEMENT >= 0), "
                "CONSTRAINT CK_STATION_AVAIL CHECK (DISPONIBILITE BETWEEN 0 AND 100), "
                "CONSTRAINT CK_STATION_STATUS CHECK (STATUT IN ('Online', 'Maintenance', 'Offline'))"
                ")"))) {
            // Another application instance may have created the table after our metadata query.
            bool createdByAnotherInstance = false;
            if (!hasStationTable(db, &createdByAnotherInstance, nullptr) || !createdByAnotherInstance) {
                setQueryError(error, query);
                return false;
            }
        }
    }

    return validateStationTable(db, error);
}

bool StationModel::afficher(QVector<StationRecord> *records, QString *error) const
{
    if (!records) {
        if (error)
            *error = QStringLiteral("The station list destination is invalid.");
        return false;
    }
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT ID_STATION, NOM_STATION, TYPE_STATION, VILLE, REGION, "
            "NB_EQUIPEMENT, DISPONIBILITE, STATUT, DATE_DERNIERE_MAJ "
            "FROM STATION ORDER BY NOM_STATION"))) {
        setQueryError(error, query);
        return false;
    }

    QVector<StationRecord> loaded;
    while (query.next()) {
        StationRecord record;
        record.id = query.value(0).toString();
        record.name = query.value(1).toString();
        record.type = query.value(2).toString();
        const QString city = query.value(3).toString().trimmed();
        const QString region = query.value(4).toString().trimmed();
        record.cityRegion = city;
        if (!region.isEmpty())
            record.cityRegion += '\n' + region;
        record.equipment = query.value(5).toInt();
        record.availability = query.value(6).toInt();
        record.status = query.value(7).toString();

        const QVariant dateValue = query.value(8);
        const QDateTime dateTime = dateValue.toDateTime();
        if (dateTime.isValid())
            record.lastUpdate = dateTime.date().toString(Qt::ISODate);
        else if (dateValue.toDate().isValid())
            record.lastUpdate = dateValue.toDate().toString(Qt::ISODate);
        else
            record.lastUpdate = dateValue.toString();

        loaded.append(record);
    }

    *records = loaded;
    if (error)
        error->clear();
    return true;
}

bool StationModel::ajouter(const StationRecord &record, QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "INSERT INTO STATION (ID_STATION, NOM_STATION, TYPE_STATION, VILLE, REGION, "
            "NB_EQUIPEMENT, DISPONIBILITE, STATUT, DATE_DERNIERE_MAJ) "
            "VALUES (:id, :name, :type, :city, :region, :equipment, :availability, :status, SYSDATE)"))) {
        setQueryError(error, query);
        return false;
    }
    bindRecord(query, record);
    if (!query.exec()) {
        setQueryError(error, query);
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool StationModel::modifier(const StationRecord &record, const QString &originalId,
                            QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "UPDATE STATION SET ID_STATION=:id, NOM_STATION=:name, TYPE_STATION=:type, "
            "VILLE=:city, REGION=:region, NB_EQUIPEMENT=:equipment, "
            "DISPONIBILITE=:availability, STATUT=:status, DATE_DERNIERE_MAJ=SYSDATE "
            "WHERE ID_STATION=:originalId"))) {
        setQueryError(error, query);
        return false;
    }
    bindRecord(query, record);
    query.bindValue(QStringLiteral(":originalId"), originalId.trimmed());
    if (!query.exec()) {
        setQueryError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The station was not found in Oracle. Refresh the list and try again.");
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool StationModel::supprimer(const QString &stationId, QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral("DELETE FROM STATION WHERE ID_STATION=:id"))) {
        setQueryError(error, query);
        return false;
    }
    query.bindValue(QStringLiteral(":id"), stationId.trimmed());
    if (!query.exec()) {
        setQueryError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The station was not found in Oracle. Refresh the list and try again.");
        return false;
    }
    if (error)
        error->clear();
    return true;
}

} // namespace GestionStation
