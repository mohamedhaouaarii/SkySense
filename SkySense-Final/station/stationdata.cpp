#include "stationdata.h"

#include "connection.h"

#include <QDate>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QtMath>

// Modele Station : table STATION du schema approuve (skysense_schema_profa.sql).
//   Colonnes : ID_STATION, NOM_STATION, TYPE_STATION, VILLE, REGION, STATUT, DATE_MISE_A_JOUR.
// Le nombre d'equipements et la disponibilite ne sont pas des colonnes : ils sont calcules par le
// chemin STATION -GERER- PERSONNEL -SUIVRE- EQUIPEMENT (voir NOTES_SCHEMA.md, point 8).
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
    query.bindValue(QStringLiteral(":status"), record.status.trimmed());
}

// Nombre de liens GERER d'une station (une station geree ne peut ni changer d'identifiant ni etre supprimee).
bool managedCount(const QSqlDatabase &db, const QString &stationId, int *count, QString *error)
{
    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral("SELECT COUNT(*) FROM GERER WHERE ID_STATION=:id"))) {
        setQueryError(error, query);
        return false;
    }
    query.bindValue(QStringLiteral(":id"), stationId.trimmed());
    if (!query.exec() || !query.next()) {
        setQueryError(error, query);
        return false;
    }
    *count = query.value(0).toInt();
    return true;
}
}

bool StationModel::ensureSchema(QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    // Le schema complet est cree par skysense_schema_profa.sql : on verifie seulement qu'il est en place.
    QSqlQuery query(db);
    for (const QString &sql : {
             QStringLiteral("SELECT ID_STATION, NOM_STATION, TYPE_STATION, VILLE, REGION, STATUT, "
                            "DATE_MISE_A_JOUR FROM STATION WHERE 1=0"),
             QStringLiteral("SELECT ID_PERSONNEL, ID_STATION FROM GERER WHERE 1=0"),
             QStringLiteral("SELECT IDENTIFIANT_EQUIPEMENT, ID_PERSONNEL, STATUT FROM EQUIPEMENT WHERE 1=0")}) {
        if (!query.exec(sql)) {
            if (error) {
                *error = QStringLiteral(
                    "The Oracle schema is incomplete (STATION, GERER or EQUIPEMENT). "
                    "Run database/skysense_schema_profa.sql as the SKYSENSE user, then restart.\n\n%1")
                             .arg(query.lastError().text());
            }
            return false;
        }
    }
    if (error)
        error->clear();
    return true;
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
            "SELECT S.ID_STATION, S.NOM_STATION, S.TYPE_STATION, S.VILLE, S.REGION, S.STATUT, "
            "S.DATE_MISE_A_JOUR, "
            "(SELECT COUNT(DISTINCT E.IDENTIFIANT_EQUIPEMENT) FROM GERER G "
            "   JOIN EQUIPEMENT E ON E.ID_PERSONNEL = G.ID_PERSONNEL "
            "  WHERE G.ID_STATION = S.ID_STATION) AS NB_EQUIPEMENT, "
            "(SELECT COUNT(DISTINCT E.IDENTIFIANT_EQUIPEMENT) FROM GERER G "
            "   JOIN EQUIPEMENT E ON E.ID_PERSONNEL = G.ID_PERSONNEL "
            "  WHERE G.ID_STATION = S.ID_STATION AND LOWER(E.STATUT) = 'online') AS NB_EN_LIGNE "
            "FROM STATION S ORDER BY S.NOM_STATION"))) {
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
        record.status = query.value(5).toString().trimmed();

        const QVariant dateValue = query.value(6);
        const QDateTime dateTime = dateValue.toDateTime();
        if (dateTime.isValid())
            record.lastUpdate = dateTime.date().toString(Qt::ISODate);
        else if (dateValue.toDate().isValid())
            record.lastUpdate = dateValue.toDate().toString(Qt::ISODate);
        else
            record.lastUpdate = dateValue.toString();

        // Disponibilite = part des equipements en ligne ; sans equipement, elle suit le statut de la station.
        record.equipment = query.value(7).toInt();
        const int online = query.value(8).toInt();
        record.availability = record.equipment > 0
            ? qRound(100.0 * online / record.equipment)
            : (record.status.compare(QStringLiteral("Online"), Qt::CaseInsensitive) == 0 ? 100 : 0);

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
            "INSERT INTO STATION (ID_STATION, NOM_STATION, TYPE_STATION, VILLE, REGION, STATUT, "
            "DATE_MISE_A_JOUR) VALUES (:id, :name, :type, :city, :region, :status, SYSTIMESTAMP)"))) {
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

    if (record.id.trimmed().compare(originalId.trimmed(), Qt::CaseInsensitive) != 0) {
        int links = 0;
        if (!managedCount(db, originalId, &links, error))
            return false;
        if (links > 0) {
            if (error)
                *error = QStringLiteral("This station is managed by staff members (GERER). "
                                        "Its ID cannot change while those links exist.");
            return false;
        }
    }

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "UPDATE STATION SET ID_STATION=:id, NOM_STATION=:name, TYPE_STATION=:type, "
            "VILLE=:city, REGION=:region, STATUT=:status, DATE_MISE_A_JOUR=SYSTIMESTAMP "
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

    int links = 0;
    if (!managedCount(db, stationId, &links, error))
        return false;
    if (links > 0) {
        if (error)
            *error = QStringLiteral("This station is still managed by %1 staff member(s) (GERER). "
                                    "Remove those links first, then delete the station.").arg(links);
        return false;
    }

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
