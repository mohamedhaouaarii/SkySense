#include "servicemodel.h"

#include "connection.h"

#include <QDate>
#include <QHash>
#include <QLocale>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QVector>

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
    q.addBindValue(r.assignee.left(120));   // text copy of all assignees (column is 120 chars)
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
    q.addBindValue(r.personnelIds.isEmpty() ? r.personnelId : r.personnelIds.first());
}

// Association "affecter": one service can be assigned to several people.
// Table SKYSENSE_AFFECTER (SERVICE_ID, PERSONNEL_ID), created by service_sqlplus.sql.
bool saveAssignees(QSqlDatabase &db, const ServiceRecord &r, QString *error)
{
    QSqlQuery del(db);
    if (!del.prepare(QStringLiteral("DELETE FROM SKYSENSE_AFFECTER WHERE SERVICE_ID = ?"))) {
        setSqlError(error, del);
        return false;
    }
    del.addBindValue(r.id);
    if (!del.exec()) {
        setSqlError(error, del);
        return false;
    }
    QVector<int> ids = r.personnelIds;
    if (ids.isEmpty() && r.personnelId > 0)
        ids.append(r.personnelId);
    QVector<int> done;
    for (int personnelId : ids) {
        if (personnelId <= 0 || done.contains(personnelId))
            continue;
        done.append(personnelId);
        QSqlQuery ins(db);
        if (!ins.prepare(QStringLiteral(
                "INSERT INTO SKYSENSE_AFFECTER (SERVICE_ID, PERSONNEL_ID) VALUES (?, ?)"))) {
            setSqlError(error, ins);
            return false;
        }
        ins.addBindValue(r.id);
        ins.addBindValue(personnelId);
        if (!ins.exec()) {
            setSqlError(error, ins);
            return false;
        }
    }
    return true;
}

// Creates SKYSENSE_AFFECTER the first time it is needed (same as service_sqlplus.sql),
// so nothing has to be run by hand. Existing services keep their single assignee.
void ensureAffecterTable(QSqlDatabase &db)
{
    static bool checked = false;
    if (checked)
        return;
    QSqlQuery q(db);
    if (!q.exec(QStringLiteral("SELECT COUNT(*) FROM USER_TABLES WHERE TABLE_NAME = 'SKYSENSE_AFFECTER'")) || !q.next())
        return;
    if (q.value(0).toInt() == 0) {
        QSqlQuery create(db);
        if (!create.exec(QStringLiteral(
                "CREATE TABLE SKYSENSE_AFFECTER ("
                "SERVICE_ID VARCHAR2(30 CHAR) NOT NULL, "
                "PERSONNEL_ID NUMBER(10) NOT NULL, "
                "CONSTRAINT PK_SKYSENSE_AFFECTER PRIMARY KEY (SERVICE_ID, PERSONNEL_ID), "
                "CONSTRAINT FK_AFFECTER_SERVICE FOREIGN KEY (SERVICE_ID) "
                "REFERENCES SKYSENSE_SERVICES (SERVICE_ID) ON DELETE CASCADE)")))
            return;   // saveAssignees() will report the problem
        QSqlQuery copy(db);
        copy.exec(QStringLiteral(
            "INSERT INTO SKYSENSE_AFFECTER (SERVICE_ID, PERSONNEL_ID) "
            "SELECT SERVICE_ID, PERSONNEL_ID FROM SKYSENSE_SERVICES "
            "WHERE PERSONNEL_ID IS NOT NULL AND PERSONNEL_ID > 0"));
    }
    checked = true;
}

QString affecterHint(const QString &sqlError)
{
    return sqlError + QStringLiteral("\n\nThe assignees are stored in the table SKYSENSE_AFFECTER. "
                                    "The app tries to create it automatically; if that is not allowed, run service_sqlplus.sql once in SQL*Plus.");
}
}

QString ServiceModel::validate(const ServiceRecord &item)
{
    if (item.id.trimmed().isEmpty() || item.id.trimmed().size() > 30)
        return QStringLiteral("The service ID is required (maximum 30 characters).");
    if (item.title.trimmed().isEmpty() || item.title.trimmed().size() > 150)
        return QStringLiteral("The service title is required (maximum 150 characters).");
    if (item.type.size() > 50 || item.stationCode.size() > 30 || item.stationName.size() > 120)
        return QStringLiteral("A field exceeds its maximum length.");
    if (item.priority != QLatin1String("High") && item.priority != QLatin1String("Medium")
        && item.priority != QLatin1String("Low"))
        return QStringLiteral("Invalid service priority.");
    if (item.status != QLatin1String("Scheduled") && item.status != QLatin1String("In Progress")
        && item.status != QLatin1String("Completed") && item.status != QLatin1String("Overdue"))
        return QStringLiteral("Invalid service status.");
    if (item.startTime.size() > 8 || item.endTime.size() > 8 || item.actualEndTime.size() > 8)
        return QStringLiteral("A time value is too long.");
    return {};
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
        if (r.personnelId > 0)
            r.personnelIds.append(r.personnelId);   // older rows: single assignee
        loaded.append(r);
    }

    // Every assignee of every service (table SKYSENSE_AFFECTER). If the table does not
    // exist yet, the single PERSONNEL_ID of each row is used instead.
    ensureAffecterTable(db);
    QSqlQuery links(db);
    if (links.exec(QStringLiteral("SELECT SERVICE_ID, PERSONNEL_ID FROM SKYSENSE_AFFECTER ORDER BY SERVICE_ID, PERSONNEL_ID"))) {
        QHash<QString, QVector<int>> byService;
        while (links.next())
            byService[links.value(0).toString()].append(links.value(1).toInt());
        for (ServiceRecord &r : loaded) {
            if (byService.contains(r.id)) {
                r.personnelIds = byService.value(r.id);
                r.personnelId = r.personnelIds.first();
            }
        }
    }

    items = loaded;
    if (error)
        error->clear();
    return true;
}

bool ServiceModel::insert(const ServiceRecord &item, QString *error) const
{
    const QString problem = validate(item);
    if (!problem.isEmpty()) {
        if (error)
            *error = problem;
        return false;
    }
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
    ensureAffecterTable(db);
    const bool inTransaction = db.transaction();
    if (!q.exec()) {
        setSqlError(error, q);
        if (inTransaction)
            db.rollback();
        return false;
    }
    QString linkError;
    if (!saveAssignees(db, item, &linkError)) {
        if (inTransaction)
            db.rollback();
        else {
            QSqlQuery undo(db);
            undo.prepare(QStringLiteral("DELETE FROM SKYSENSE_SERVICES WHERE SERVICE_ID = ?"));
            undo.addBindValue(item.id);
            undo.exec();
        }
        if (error)
            *error = affecterHint(linkError);
        return false;
    }
    if (inTransaction && !db.commit()) {
        if (error)
            *error = db.lastError().text();
        db.rollback();
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool ServiceModel::update(const ServiceRecord &item, QString *error) const
{
    const QString problem = validate(item);
    if (!problem.isEmpty()) {
        if (error)
            *error = problem;
        return false;
    }
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
    ensureAffecterTable(db);
    const bool inTransaction = db.transaction();
    if (!q.exec()) {
        setSqlError(error, q);
        if (inTransaction)
            db.rollback();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The service no longer exists in Oracle.");
        if (inTransaction)
            db.rollback();
        return false;
    }
    QString linkError;
    if (!saveAssignees(db, item, &linkError)) {
        if (inTransaction)
            db.rollback();
        if (error)
            *error = affecterHint(linkError);
        return false;
    }
    if (inTransaction && !db.commit()) {
        if (error)
            *error = db.lastError().text();
        db.rollback();
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

    {   // remove the assignee links first (ignored if the table does not exist yet)
        QSqlQuery links(db);
        if (links.prepare(QStringLiteral("DELETE FROM SKYSENSE_AFFECTER WHERE SERVICE_ID = ?"))) {
            links.addBindValue(id);
            links.exec();
        }
    }
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
