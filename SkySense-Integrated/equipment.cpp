#include "equipment.h"
#include "connection.h"

#include <QByteArray>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
QVariant nullableString(const QString &value)
{
    return value.trimmed().isEmpty() ? QVariant(QMetaType::fromType<QString>()) : QVariant(value.trimmed());
}

QVariant nullablePhoto(const QString &photoBase64)
{
    return photoBase64.isEmpty()
        ? QVariant(QMetaType::fromType<QByteArray>())
        : QVariant(QByteArray::fromBase64(photoBase64.toLatin1()));
}

QVariant updateDate(const QString &value)
{
    const QDateTime date = QDateTime::fromString(value, Qt::ISODate);
    return date.isValid() ? QVariant(date) : QVariant(QMetaType::fromType<QDateTime>());
}

QSqlDatabase database(QString *error)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        if (error) *error = QStringLiteral("Oracle is not connected. Reopen the application and connect with the SKYSENSE ODBC data source.");
        return {};
    }
    return db;
}

void setQueryError(QString *error, const QSqlQuery &query)
{
    if (error) *error = query.lastError().text();
}

void bindRecord(QSqlQuery &query, const EquipmentRecord &record)
{
    query.bindValue(QStringLiteral(":serial"), record.serialNumber.trimmed());
    query.bindValue(QStringLiteral(":identifier"), record.id.trimmed());
    query.bindValue(QStringLiteral(":type"), record.type.trimmed());
    query.bindValue(QStringLiteral(":model"), record.model.trimmed());
    query.bindValue(QStringLiteral(":unit"), record.unit.trimmed());
    query.bindValue(QStringLiteral(":location"), nullableString(record.location));
    query.bindValue(QStringLiteral(":status"), record.status.trimmed());
    query.bindValue(QStringLiteral(":updated"), updateDate(record.updated));
    query.bindValue(QStringLiteral(":photo"), nullablePhoto(record.photoBase64));
    query.bindValue(QStringLiteral(":station"), nullableString(record.station));
}

QString insertSql()
{
    return QStringLiteral(
        "INSERT INTO EQUIPEMENT (NUMERO_SERIE, IDENTIFIANT_EQUIPEMENT, TYPE_EQUIPEMENT, MODELE, UNITE, "
        "EMPLACEMENT, STATUT, DATE_DERNIERE_MAJ, PHOTO, NOM_STATION) "
        "VALUES (:serial, :identifier, :type, :model, :unit, :location, :status, :updated, :photo, :station)");
}
}

bool EquipmentModel::afficher(QVector<EquipmentRecord> *records, QString *error) const
{
    if (!records) {
        if (error) *error = QStringLiteral("The equipment list destination is invalid.");
        return false;
    }
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen()) return false;

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT IDENTIFIANT_EQUIPEMENT, TYPE_EQUIPEMENT, NOM_STATION, EMPLACEMENT, UNITE, "
            "DATE_DERNIERE_MAJ, STATUT, MODELE, NUMERO_SERIE, PHOTO "
            "FROM EQUIPEMENT ORDER BY IDENTIFIANT_EQUIPEMENT"))) {
        setQueryError(error, query);
        return false;
    }

    QVector<EquipmentRecord> loaded;
    while (query.next()) {
        EquipmentRecord record;
        record.id = query.value(0).toString();
        record.type = query.value(1).toString();
        record.station = query.value(2).toString();
        record.location = query.value(3).toString();
        record.unit = query.value(4).toString();
        const QDateTime date = query.value(5).toDateTime();
        record.updated = date.isValid() ? date.toString(Qt::ISODate) : QString();
        record.status = query.value(6).toString();
        record.model = query.value(7).toString();
        record.serialNumber = query.value(8).toString();
        const QByteArray photo = query.value(9).toByteArray();
        record.photoBase64 = photo.isEmpty() ? QString() : QString::fromLatin1(photo.toBase64());
        loaded.append(record);
    }
    *records = loaded;
    if (error) error->clear();
    return true;
}

bool EquipmentModel::ajouter(const EquipmentRecord &record, QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen()) return false;
    QSqlQuery query(db);
    if (!query.prepare(insertSql())) {
        setQueryError(error, query);
        return false;
    }
    bindRecord(query, record);
    if (!query.exec()) {
        setQueryError(error, query);
        return false;
    }
    if (error) error->clear();
    return true;
}

bool EquipmentModel::modifier(const EquipmentRecord &record, const QString &originalSerial,
                              QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen()) return false;
    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "UPDATE EQUIPEMENT SET NUMERO_SERIE=:serial, IDENTIFIANT_EQUIPEMENT=:identifier, "
            "TYPE_EQUIPEMENT=:type, MODELE=:model, UNITE=:unit, EMPLACEMENT=:location, "
            "STATUT=:status, DATE_DERNIERE_MAJ=:updated, PHOTO=:photo, NOM_STATION=:station "
            "WHERE NUMERO_SERIE=:originalSerial"))) {
        setQueryError(error, query);
        return false;
    }
    bindRecord(query, record);
    query.bindValue(QStringLiteral(":originalSerial"), originalSerial.trimmed());
    if (!query.exec()) {
        setQueryError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error) *error = QStringLiteral("The equipment was not found in Oracle; refresh the list and try again.");
        return false;
    }
    if (error) error->clear();
    return true;
}

bool EquipmentModel::supprimer(const QString &serialNumber, QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen()) return false;
    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral("DELETE FROM EQUIPEMENT WHERE NUMERO_SERIE=:serial"))) {
        setQueryError(error, query);
        return false;
    }
    query.bindValue(QStringLiteral(":serial"), serialNumber.trimmed());
    if (!query.exec()) {
        setQueryError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error) *error = QStringLiteral("The equipment was not found in Oracle; refresh the list and try again.");
        return false;
    }
    if (error) error->clear();
    return true;
}

bool EquipmentModel::stations(QStringList *stationNames, QString *error) const
{
    if (!stationNames) {
        if (error) *error = QStringLiteral("The station list destination is invalid.");
        return false;
    }
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen()) return false;
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("SELECT NOM_STATION FROM STATION ORDER BY NOM_STATION"))) {
        setQueryError(error, query);
        return false;
    }
    QStringList loaded;
    while (query.next()) loaded.append(query.value(0).toString());
    *stationNames = loaded;
    if (error) error->clear();
    return true;
}
