#include "maintenancemodel.h"

#include "../commun/connection.h"

#include <QDate>
#include <QHash>
#include <QMetaType>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

namespace {
QSqlDatabase database(QString *error)
{
    QSqlDatabase db = Connection::createInstance().database();
    if (!db.isValid() || !db.isOpen()) {
        if (error)
            *error = QStringLiteral("Oracle is not connected. Reopen SkySense and connect to the SKYSENSE ODBC data source.");
        return {};
    }
    return db;
}

QString sqlErrorDetails(const QSqlError &sqlError)
{
    QStringList details;
    const QString nativeCode = sqlError.nativeErrorCode().trimmed();
    const QString databaseText = sqlError.databaseText().trimmed();
    const QString driverText = sqlError.driverText().trimmed();

    if (!nativeCode.isEmpty())
        details.append(QStringLiteral("Oracle/ODBC code: %1").arg(nativeCode));
    if (!databaseText.isEmpty())
        details.append(databaseText);
    if (!driverText.isEmpty() && driverText != databaseText)
        details.append(driverText);
    if (details.isEmpty() && !sqlError.text().trimmed().isEmpty())
        details.append(sqlError.text().trimmed());

    return details.join(QLatin1Char('\n'));
}

void setSqlError(QString *error, const QSqlQuery &query, const QString &operation = QString())
{
    if (!error)
        return;

    const QString details = sqlErrorDetails(query.lastError());
    *error = operation.isEmpty() ? details : operation + QLatin1Char('\n') + details;
}

void setDatabaseError(QString *error, const QSqlDatabase &db, const QString &operation)
{
    if (!error)
        return;

    const QString details = sqlErrorDetails(db.lastError());
    *error = operation;
    if (!details.isEmpty())
        *error += QLatin1Char('\n') + details;
}

void clearError(QString *error)
{
    if (error)
        error->clear();
}

QVariant nullableDate(const QDateTime &dateTime)
{
    return dateTime.isValid() ? QVariant(dateTime)
                              : QVariant(QMetaType::fromType<QDateTime>());
}

QVector<int> assignedTechnicians(const Intervention &item)
{
    QVector<int> ids = item.technicianIds;
    if (ids.isEmpty() && item.technicianId > 0)
        ids.append(item.technicianId);

    QVector<int> unique;
    QSet<int> seen;
    for (int id : ids) {
        if (id > 0 && !seen.contains(id)) {
            seen.insert(id);
            unique.append(id);
        }
    }
    return unique;
}

bool replaceTechnicians(const QSqlDatabase &db, const Intervention &item, QString *error)
{
    const QVector<int> ids = assignedTechnicians(item);
    // An edit to the description of a legacy row without REALISER data must
    // not erase unrelated assignments. The edit dialog supplies a technician
    // before a relationship is changed.
    if (ids.isEmpty())
        return true;

    QSqlQuery remove(db);
    if (!remove.prepare(QStringLiteral("DELETE FROM REALISER WHERE ID_MAINTENANCE = ?"))) {
        setSqlError(error, remove);
        return false;
    }
    remove.addBindValue(item.databaseId);
    if (!remove.exec()) {
        setSqlError(error, remove);
        return false;
    }

    QSqlQuery insert(db);
    if (!insert.prepare(QStringLiteral(
            "INSERT INTO REALISER (ID_PERSONNEL, ID_MAINTENANCE) VALUES (?, ?)"))) {
        setSqlError(error, insert);
        return false;
    }
    for (int technicianId : ids) {
        insert.bindValue(0, technicianId);
        insert.bindValue(1, item.databaseId);
        if (!insert.exec()) {
            setSqlError(error, insert);
            return false;
        }
    }
    return true;
}

bool writeMaintenanceFields(QSqlQuery &query, const Intervention &item)
{
    query.bindValue(QStringLiteral(":code"), item.id.trimmed());
    query.bindValue(QStringLiteral(":title"), item.title.trimmed());
    query.bindValue(QStringLiteral(":planned"), nullableDate(item.plannedStart));
    const QDateTime completedAt = item.status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0
                                      ? item.realEnd : QDateTime();
    query.bindValue(QStringLiteral(":realised"), nullableDate(completedAt));
    query.bindValue(QStringLiteral(":priority"), item.priority.trimmed());
    query.bindValue(QStringLiteral(":status"), item.status.trimmed());
    query.bindValue(QStringLiteral(":description"), item.desc.trimmed());
    query.bindValue(QStringLiteral(":report"), item.report.trimmed());
    return true;
}
} // namespace

bool MaintenanceModel::loadAll(QVector<Intervention> &items, QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT ID_MAINTENANCE, CODE, TITRE, PRIORITE, STATUT, DESCRIPTION, COMPTE_RENDU, "
            "DATE_PREVUE, DATE_REALISEE FROM MAINTENANCE ORDER BY DATE_PREVUE, ID_MAINTENANCE"))) {
        setSqlError(error, query);
        return false;
    }

    QVector<Intervention> loaded;
    QHash<qint64, int> rowForId;
    while (query.next()) {
        Intervention item;
        item.databaseId = query.value(0).toLongLong();
        item.id = query.value(1).toString();
        item.title = query.value(2).toString();
        item.priority = query.value(3).toString();
        item.status = query.value(4).toString();
        item.desc = query.value(5).toString();
        item.report = query.value(6).toString();
        item.plannedStart = query.value(7).toDateTime();
        item.plannedEnd = item.plannedStart;
        if (!query.value(8).isNull())
            item.realEnd = query.value(8).toDateTime();
        rowForId.insert(item.databaseId, loaded.size());
        loaded.append(item);
    }
    query.finish();

    QSqlQuery assignments(db);
    if (!assignments.exec(QStringLiteral(
            "SELECT R.ID_MAINTENANCE, P.ID_PERSONNEL, TRIM(P.PRENOM || ' ' || P.NOM) "
            "FROM REALISER R JOIN PERSONNEL P ON P.ID_PERSONNEL = R.ID_PERSONNEL "
            "ORDER BY R.ID_MAINTENANCE, P.ID_PERSONNEL"))) {
        setSqlError(error, assignments);
        return false;
    }
    while (assignments.next()) {
        const qint64 maintenanceId = assignments.value(0).toLongLong();
        const auto row = rowForId.constFind(maintenanceId);
        if (row == rowForId.cend())
            continue;
        Intervention &item = loaded[row.value()];
        const int technicianId = assignments.value(1).toInt();
        item.technicianIds.append(technicianId);
        if (item.technicianId == 0) {
            item.technicianId = technicianId;
            item.tech = assignments.value(2).toString();
        }
    }
    assignments.finish();

    items = loaded;
    clearError(error);
    return true;
}

bool MaintenanceModel::loadTechnicians(QVector<Technician> &items, QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT ID_PERSONNEL, TRIM(PRENOM || ' ' || NOM), STATUT FROM PERSONNEL "
            "WHERE UPPER(NVL(STATUT, 'ACTIVE')) <> 'INACTIVE' ORDER BY PRENOM, NOM"))) {
        setSqlError(error, query);
        return false;
    }

    QVector<Technician> loaded;
    while (query.next()) {
        Technician technician;
        technician.id = query.value(0).toInt();
        technician.name = query.value(1).toString();
        technician.active = query.value(2).toString().compare(QStringLiteral("Inactive"), Qt::CaseInsensitive) != 0;
        loaded.append(technician);
    }
    items = loaded;
    clearError(error);
    return true;
}

bool MaintenanceModel::insert(Intervention &item, QString *error) const
{
    QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    const QVector<int> technicians = assignedTechnicians(item);
    if (technicians.isEmpty()) {
        if (error)
            *error = QStringLiteral("Choose an active staff member to assign this intervention.");
        return false;
    }

    if (!db.transaction()) {
        setDatabaseError(error, db, QStringLiteral("Could not start the Oracle transaction."));
        return false;
    }

    // The approved database scripts do not create a MAINTENANCE sequence.
    // Locking this low-volume table makes MAX(ID)+1 safe across concurrent adds.
    QSqlQuery lock(db);
    if (!lock.exec(QStringLiteral("LOCK TABLE MAINTENANCE IN EXCLUSIVE MODE"))) {
        setSqlError(error, lock);
        db.rollback();
        return false;
    }
    QSqlQuery nextId(db);
    if (!nextId.exec(QStringLiteral(
            "SELECT NVL(MAX(ID_MAINTENANCE), 0) + 1 FROM MAINTENANCE")) || !nextId.next()) {
        setSqlError(error, nextId);
        db.rollback();
        return false;
    }

    item.databaseId = nextId.value(0).toLongLong();
    item.id = QStringLiteral("MT-%1-%2")
                  .arg(QDate::currentDate().year())
                  .arg(item.databaseId, 3, 10, QLatin1Char('0'));
    item.technicianIds = technicians;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "INSERT INTO MAINTENANCE (ID_MAINTENANCE, CODE, TITRE, DATE_PREVUE, DATE_REALISEE, "
            "PRIORITE, STATUT, DESCRIPTION, COMPTE_RENDU) VALUES "
            "(:id, :code, :title, :planned, :realised, :priority, :status, :description, :report)"))) {
        setSqlError(error, query);
        db.rollback();
        return false;
    }
    query.bindValue(QStringLiteral(":id"), item.databaseId);
    writeMaintenanceFields(query, item);
    if (!query.exec()) {
        setSqlError(error, query);
        db.rollback();
        return false;
    }
    if (!replaceTechnicians(db, item, error)) {
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        if (error)
            *error = db.lastError().text();
        db.rollback();
        return false;
    }
    clearError(error);
    return true;
}

bool MaintenanceModel::update(const Intervention &item, QString *error) const
{
    QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;
    if (item.databaseId <= 0) {
        if (error)
            *error = QStringLiteral("The intervention has no valid Oracle ID. Reload the maintenance list and try again.");
        return false;
    }

    if (!db.transaction()) {
        setDatabaseError(error, db, QStringLiteral("Could not start the Oracle transaction for editing."));
        return false;
    }

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "UPDATE MAINTENANCE SET CODE = :code, TITRE = :title, DATE_PREVUE = :planned, "
            "DATE_REALISEE = :realised, PRIORITE = :priority, STATUT = :status, "
            "DESCRIPTION = :description, COMPTE_RENDU = :report "
            "WHERE ID_MAINTENANCE = :id"))) {
        setSqlError(error, query);
        db.rollback();
        return false;
    }
    writeMaintenanceFields(query, item);
    query.bindValue(QStringLiteral(":id"), item.databaseId);
    if (!query.exec()) {
        setSqlError(error, query);
        db.rollback();
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("This intervention no longer exists in Oracle. Reload the list and try again.");
        db.rollback();
        return false;
    }
    if (!replaceTechnicians(db, item, error)) {
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        if (error)
            *error = db.lastError().text();
        db.rollback();
        return false;
    }
    clearError(error);
    return true;
}

bool MaintenanceModel::remove(const Intervention &item, QString *error) const
{
    QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;
    if (item.databaseId <= 0) {
        if (error)
            *error = QStringLiteral("The intervention has no valid Oracle ID. Reload the maintenance list and try again.");
        return false;
    }

    if (!db.transaction()) {
        setDatabaseError(error, db, QStringLiteral("Could not start the Oracle transaction for deletion."));
        return false;
    }
    // This key is a validated integer loaded from Oracle. Formatting it as a
    // decimal literal avoids driver-specific ODBC binding for these DELETEs.
    const QString maintenanceId = QString::number(item.databaseId);
    QSqlQuery assignments(db);
    if (!assignments.exec(QStringLiteral("DELETE FROM REALISER WHERE ID_MAINTENANCE = %1")
                              .arg(maintenanceId))) {
        setSqlError(error, assignments, QStringLiteral("Could not delete linked technician assignments (REALISER)."));
        db.rollback();
        return false;
    }
    assignments.finish();

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("DELETE FROM MAINTENANCE WHERE ID_MAINTENANCE = %1")
                        .arg(maintenanceId))) {
        setSqlError(error, query, QStringLiteral("Could not delete the maintenance intervention from MAINTENANCE."));
        db.rollback();
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("This intervention no longer exists in Oracle. Reload the list and try again.");
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        setDatabaseError(error, db, QStringLiteral("Oracle could not commit the deletion."));
        db.rollback();
        return false;
    }
    clearError(error);
    return true;
}
