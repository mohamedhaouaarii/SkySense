#include "servicemodel.h"

#include "connection.h"

#include <QDate>
#include <QLocale>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
const QString dateFormat = QStringLiteral("MMM dd, yyyy");   // format used by the Service screen

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

QDate parseScreenDate(const QString &text)
{
    QDate d = QDate::fromString(text, dateFormat);                 // same locale as the form
    if (!d.isValid())
        d = QLocale::c().toDate(text, dateFormat);                 // "Oct 20, 2026"
    return d;
}

QString checklistToText(const QStringList &items)
{
    return items.join(QLatin1Char('\n'));
}

QString doneToText(const QVector<bool> &done)
{
    QStringList parts;
    for (bool b : done)
        parts << (b ? QStringLiteral("1") : QStringLiteral("0"));
    return parts.join(QLatin1Char(','));
}

void bindCommonValues(QSqlQuery &q, const ServiceRecord &r)
{
    // Order must match the SQL in insert() / update() (everything except the ID).
    q.addBindValue(r.title.trimmed());
    q.addBindValue(r.type);
    q.addBindValue(r.stationCode);
    q.addBindValue(r.stationName);
    q.addBindValue(r.assignee);
    q.addBindValue(parseScreenDate(r.date).toString(QStringLiteral("yyyy-MM-dd")));
    q.addBindValue(r.startTime);
    q.addBindValue(r.endTime);
    q.addBindValue(r.priority);
    q.addBindValue(r.status);
    q.addBindValue(r.description.trimmed());
    q.addBindValue(checklistToText(r.checklist));
    q.addBindValue(doneToText(r.checklistDone));
    q.addBindValue(r.actualEndTime);
    q.addBindValue(r.stationId);
    q.addBindValue(r.personnelId);
}
}

bool ServiceModel::loadAll(QVector<ServiceRecord> &items, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery q(db);
    if (!q.exec(QStringLiteral(
            "SELECT SERVICE_ID, TITLE, SERVICE_TYPE, STATION_CODE, STATION_NAME, ASSIGNEE, "
            "TO_CHAR(SERVICE_DATE, 'YYYY-MM-DD'), START_TIME, END_TIME, PRIORITY, STATUS, "
            "DESCRIPTION_TEXT, CHECKLIST_TEXT, CHECKLIST_DONE, ACTUAL_END_TIME, "
            "STATION_ID, PERSONNEL_ID "
            "FROM SKYSENSE_SERVICES ORDER BY SERVICE_ID"))) {
        setSqlError(error, q);
        return false;
    }

    QVector<ServiceRecord> loaded;
    while (q.next()) {
        ServiceRecord r;
        r.id = q.value(0).toString();
        r.title = q.value(1).toString();
        r.type = q.value(2).toString();
        r.stationCode = q.value(3).toString();
        r.stationName = q.value(4).toString();
        r.assignee = q.value(5).toString();
        const QDate d = QDate::fromString(q.value(6).toString(), QStringLiteral("yyyy-MM-dd"));
        r.date = d.isValid() ? d.toString(dateFormat) : QString();
        r.startTime = q.value(7).toString();
        r.endTime = q.value(8).toString();
        r.priority = q.value(9).toString();
        r.status = q.value(10).toString();
        r.description = q.value(11).toString();
        const QString checklistText = q.value(12).toString();
        r.checklist = checklistText.isEmpty() ? QStringList()
                                              : checklistText.split(QLatin1Char('\n'));
        const QStringList done = q.value(13).toString().split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (int i = 0; i < r.checklist.size(); ++i)
            r.checklistDone.append(i < done.size() && done.at(i) == QLatin1String("1"));
        r.actualEndTime = q.value(14).toString();
        r.stationId = q.value(15).toInt();
        r.personnelId = q.value(16).toInt();
        loaded.append(r);
    }

    items = loaded;
    if (error)
        error->clear();
    return true;
}

bool ServiceModel::insert(const ServiceRecord &item, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery q(db);
    if (!q.prepare(QStringLiteral(
            "INSERT INTO SKYSENSE_SERVICES "
            "(SERVICE_ID, TITLE, SERVICE_TYPE, STATION_CODE, STATION_NAME, ASSIGNEE, "
            "SERVICE_DATE, START_TIME, END_TIME, PRIORITY, STATUS, DESCRIPTION_TEXT, "
            "CHECKLIST_TEXT, CHECKLIST_DONE, ACTUAL_END_TIME, STATION_ID, PERSONNEL_ID) "
            "VALUES (?, ?, ?, ?, ?, ?, TO_DATE(?, 'YYYY-MM-DD'), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"))) {
        setSqlError(error, q);
        return false;
    }
    // The ID is the first "?", then the 16 common values.
    // addBindValue is positional, so bind the ID first.
    q.addBindValue(item.id);
    bindCommonValues(q, item);
    if (!q.exec()) {
        setSqlError(error, q);
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool ServiceModel::update(const ServiceRecord &item, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery q(db);
    if (!q.prepare(QStringLiteral(
            "UPDATE SKYSENSE_SERVICES SET TITLE = ?, SERVICE_TYPE = ?, STATION_CODE = ?, "
            "STATION_NAME = ?, ASSIGNEE = ?, SERVICE_DATE = TO_DATE(?, 'YYYY-MM-DD'), "
            "START_TIME = ?, END_TIME = ?, PRIORITY = ?, STATUS = ?, DESCRIPTION_TEXT = ?, "
            "CHECKLIST_TEXT = ?, CHECKLIST_DONE = ?, ACTUAL_END_TIME = ?, STATION_ID = ?, "
            "PERSONNEL_ID = ? WHERE SERVICE_ID = ?"))) {
        setSqlError(error, q);
        return false;
    }
    bindCommonValues(q, item);
    q.addBindValue(item.id);
    if (!q.exec()) {
        setSqlError(error, q);
        return false;
    }
    if (q.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The service no longer exists in Oracle.");
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool ServiceModel::remove(const QString &id, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery q(db);
    if (!q.prepare(QStringLiteral("DELETE FROM SKYSENSE_SERVICES WHERE SERVICE_ID = ?"))) {
        setSqlError(error, q);
        return false;
    }
    q.addBindValue(id);
    if (!q.exec()) {
        setSqlError(error, q);
        return false;
    }
    if (q.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The service no longer exists in Oracle.");
        return false;
    }
    if (error)
        error->clear();
    return true;
}
