#include "maintenancemodel.h"

#include "connection.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
QSqlDatabase connectedDatabase(QString *error)
{
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

QVariant dateValue(const QDateTime &dateTime)
{
    if (!dateTime.isValid())
        // Keep the NULL bind typed as text for QODBC. An untyped QVariant()
        // can be sent by the Oracle ODBC driver as BINARY, which makes
        // TO_TIMESTAMP fail with ORA-00932 for optional actual dates.
        return QVariant::fromValue(QString());
    return dateTime.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

void bindInsertValues(QSqlQuery &query, const Intervention &item)
{
    query.addBindValue(item.id);
    query.addBindValue(item.equipId);
    query.addBindValue(item.title.trimmed());
    query.addBindValue(item.tech);
    query.addBindValue(item.priority);
    query.addBindValue(item.status);
    query.addBindValue(item.desc.trimmed());
    query.addBindValue(item.report);
    query.addBindValue(dateValue(item.plannedStart));
    query.addBindValue(dateValue(item.plannedEnd));
    query.addBindValue(dateValue(item.realStart));
    query.addBindValue(dateValue(item.realEnd));
    query.addBindValue(item.actionsMask);
}

void bindUpdateValues(QSqlQuery &query, const Intervention &item)
{
    query.addBindValue(item.equipId);
    query.addBindValue(item.title.trimmed());
    query.addBindValue(item.tech);
    query.addBindValue(item.priority);
    query.addBindValue(item.status);
    query.addBindValue(item.desc.trimmed());
    query.addBindValue(item.report);
    query.addBindValue(dateValue(item.plannedStart));
    query.addBindValue(dateValue(item.plannedEnd));
    query.addBindValue(dateValue(item.realStart));
    query.addBindValue(dateValue(item.realEnd));
    query.addBindValue(item.actionsMask);
    query.addBindValue(item.id);
}

const QString timestampExpression =
    QStringLiteral("TO_TIMESTAMP(?, 'YYYY-MM-DD HH24:MI:SS')");
}

bool MaintenanceModel::loadAll(QVector<Intervention> &items, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT ID, EQUIPMENT_ID, TITLE, TECHNICIAN, PRIORITY, STATUS, "
            "DESCRIPTION_TEXT, WORK_REPORT, PLANNED_START, PLANNED_END, "
            "REAL_START, REAL_END, ACTIONS_MASK "
            "FROM MAINTENANCE_INTERVENTION ORDER BY PLANNED_START, ID"))) {
        setSqlError(error, query);
        return false;
    }

    QVector<Intervention> loaded;
    while (query.next()) {
        Intervention item;
        item.id = query.value(0).toString();
        item.equipId = query.value(1).toString();
        item.title = query.value(2).toString();
        item.tech = query.value(3).toString();
        item.priority = query.value(4).toString();
        item.status = query.value(5).toString();
        item.desc = query.value(6).toString();
        item.report = query.value(7).toString();
        item.plannedStart = query.value(8).toDateTime();
        item.plannedEnd = query.value(9).toDateTime();
        if (!query.value(10).isNull())
            item.realStart = query.value(10).toDateTime();
        if (!query.value(11).isNull())
            item.realEnd = query.value(11).toDateTime();
        item.actionsMask = query.value(12).toInt();
        loaded.append(item);
    }

    items = loaded;
    if (error)
        error->clear();
    return true;
}

bool MaintenanceModel::insert(Intervention &item, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery idQuery(db);
    if (!idQuery.exec(QStringLiteral(
            "SELECT 'MT-' || TO_CHAR(SYSDATE, 'YYYY') || '-' || "
            "LPAD(SEQ_MAINTENANCE_INTERVENTION.NEXTVAL, 6, '0') FROM DUAL"))
        || !idQuery.next()) {
        setSqlError(error, idQuery);
        return false;
    }
    item.id = idQuery.value(0).toString();

    QSqlQuery query(db);
    const QString sql = QStringLiteral(
        "INSERT INTO MAINTENANCE_INTERVENTION "
        "(ID, EQUIPMENT_ID, TITLE, TECHNICIAN, PRIORITY, STATUS, DESCRIPTION_TEXT, "
        "WORK_REPORT, PLANNED_START, PLANNED_END, REAL_START, REAL_END, ACTIONS_MASK) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ") + timestampExpression
        + QStringLiteral(", ") + timestampExpression + QStringLiteral(", ")
        + timestampExpression + QStringLiteral(", ") + timestampExpression
        + QStringLiteral(", ?)");
    if (!query.prepare(sql)) {
        setSqlError(error, query);
        return false;
    }
    bindInsertValues(query, item);
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool MaintenanceModel::update(const Intervention &item, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    const QString sql = QStringLiteral(
        "UPDATE MAINTENANCE_INTERVENTION SET EQUIPMENT_ID = ?, TITLE = ?, "
        "TECHNICIAN = ?, PRIORITY = ?, STATUS = ?, DESCRIPTION_TEXT = ?, "
        "WORK_REPORT = ?, PLANNED_START = ") + timestampExpression
        + QStringLiteral(", PLANNED_END = ") + timestampExpression
        + QStringLiteral(", REAL_START = ") + timestampExpression
        + QStringLiteral(", REAL_END = ") + timestampExpression
        + QStringLiteral(", ACTIONS_MASK = ? WHERE ID = ?");
    if (!query.prepare(sql)) {
        setSqlError(error, query);
        return false;
    }
    bindUpdateValues(query, item);
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The intervention no longer exists in Oracle.");
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool MaintenanceModel::remove(const QString &id, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "DELETE FROM MAINTENANCE_INTERVENTION WHERE ID = ?"))) {
        setSqlError(error, query);
        return false;
    }
    query.addBindValue(id);
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The intervention no longer exists in Oracle.");
        return false;
    }
    if (error)
        error->clear();
    return true;
}
