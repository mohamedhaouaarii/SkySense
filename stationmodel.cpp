#include "stationmodel.h"

#include <QDate>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
QSqlDatabase connectedDatabase(QString *error)
{
    // Same connection as every other module: the Singleton opened it in main().
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        if (error)
            *error = QStringLiteral("The Oracle connection is not open.");
        return {};
    }
    return db;
}

void setSqlError(QString *error, const QSqlQuery &query)
{
    if (error)
        *error = query.lastError().text();
}

// A NULL date is sent as a typed (empty) text value so QODBC/Oracle accept it.
QVariant dateText(const QString &isoDate)
{
    const QDate date = QDate::fromString(isoDate.left(10), Qt::ISODate);
    return date.isValid() ? QVariant(date.toString(QStringLiteral("yyyy-MM-dd")))
                          : QVariant::fromValue(QString());
}

void bindStation(QSqlQuery &query, const StationRecord &s)
{
    query.bindValue(QStringLiteral(":id"), s.id.trimmed());
    query.bindValue(QStringLiteral(":name"), s.name.trimmed());
    query.bindValue(QStringLiteral(":type"), s.type.trimmed());
    query.bindValue(QStringLiteral(":city"), s.cityRegion.trimmed());
    query.bindValue(QStringLiteral(":equipment"), s.equipment);
    query.bindValue(QStringLiteral(":availability"), s.availability);
    query.bindValue(QStringLiteral(":status"), s.status);
    query.bindValue(QStringLiteral(":updated"), dateText(s.lastUpdate));
}
}

bool StationModel::isConnected()
{
    const QSqlDatabase db = QSqlDatabase::database();
    return db.isValid() && db.isOpen();
}

QString StationModel::validate(const StationRecord &s)
{
    if (s.id.trimmed().isEmpty() || s.name.trimmed().isEmpty())
        return QStringLiteral("The station ID and name are required.");
    if (s.id.trimmed().size() > 20)
        return QStringLiteral("The station ID cannot exceed 20 characters.");
    if (s.name.trimmed().size() > 120)
        return QStringLiteral("The station name cannot exceed 120 characters.");
    if (s.type.trimmed().size() > 80)
        return QStringLiteral("The station type cannot exceed 80 characters.");
    if (s.cityRegion.trimmed().size() > 200)
        return QStringLiteral("City and region together cannot exceed 200 characters.");
    if (s.status != QLatin1String("Online") && s.status != QLatin1String("Maintenance")
        && s.status != QLatin1String("Offline"))
        return QStringLiteral("Invalid station status.");
    if (s.availability < 0 || s.availability > 100)
        return QStringLiteral("Availability must be between 0 and 100.");
    if (s.equipment < 0)
        return QStringLiteral("The equipment count cannot be negative.");
    return {};
}

bool StationModel::afficher(QVector<StationRecord> *stations, QString *error) const
{
    if (!stations) {
        if (error) *error = QStringLiteral("The station list destination is invalid.");
        return false;
    }
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT STATION_ID, STATION_NAME, STATION_TYPE, CITY_REGION, EQUIPMENT_COUNT, "
            "AVAILABILITY_PCT, STATUS, LAST_UPDATE FROM SKYSENSE_STATIONS ORDER BY STATION_ID"))) {
        setSqlError(error, query);
        return false;
    }

    QVector<StationRecord> loaded;
    while (query.next()) {
        StationRecord s;
        s.id = query.value(0).toString();
        s.name = query.value(1).toString();
        s.type = query.value(2).toString();
        s.cityRegion = query.value(3).toString();
        s.equipment = query.value(4).toInt();
        s.availability = query.value(5).toInt();
        s.status = query.value(6).toString();
        const QDate date = query.value(7).toDate();
        s.lastUpdate = date.isValid() ? date.toString(Qt::ISODate) : QString();
        loaded.append(s);
    }
    *stations = loaded;
    if (error) error->clear();
    return true;
}

bool StationModel::ajouter(const StationRecord &station, QString *error) const
{
    const QString problem = validate(station);
    if (!problem.isEmpty()) {
        if (error) *error = problem;
        return false;
    }
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "INSERT INTO SKYSENSE_STATIONS (STATION_ID, STATION_NAME, STATION_TYPE, CITY_REGION, "
            "EQUIPMENT_COUNT, AVAILABILITY_PCT, STATUS, LAST_UPDATE) "
            "VALUES (:id, :name, :type, :city, :equipment, :availability, :status, "
            "TO_DATE(:updated, 'YYYY-MM-DD'))"))) {
        setSqlError(error, query);
        return false;
    }
    bindStation(query, station);
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (error) error->clear();
    return true;
}

bool StationModel::modifier(const StationRecord &station, const QString &originalId,
                            QString *error) const
{
    const QString problem = validate(station);
    if (!problem.isEmpty()) {
        if (error) *error = problem;
        return false;
    }
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "UPDATE SKYSENSE_STATIONS SET STATION_ID = :id, STATION_NAME = :name, "
            "STATION_TYPE = :type, CITY_REGION = :city, EQUIPMENT_COUNT = :equipment, "
            "AVAILABILITY_PCT = :availability, STATUS = :status, "
            "LAST_UPDATE = TO_DATE(:updated, 'YYYY-MM-DD') WHERE STATION_ID = :originalId"))) {
        setSqlError(error, query);
        return false;
    }
    bindStation(query, station);
    query.bindValue(QStringLiteral(":originalId"), originalId.trimmed());
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error) *error = QStringLiteral("The station no longer exists in Oracle.");
        return false;
    }
    if (error) error->clear();
    return true;
}

bool StationModel::supprimer(const QString &id, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral("DELETE FROM SKYSENSE_STATIONS WHERE STATION_ID = :id"))) {
        setSqlError(error, query);
        return false;
    }
    query.bindValue(QStringLiteral(":id"), id.trimmed());
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error) *error = QStringLiteral("The station no longer exists in Oracle.");
        return false;
    }
    if (error) error->clear();
    return true;
}
