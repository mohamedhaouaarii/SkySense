#include "servicemodel.h"

#include "connection.h"

#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QLocale>
#include <QMetaType>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>
#include <QVariant>
#include <cmath>

namespace {
const QString screenDateFormat = QStringLiteral("MMM dd, yyyy");   // format de l'ecran Services
const QString timeFormat = QStringLiteral("HH:mm");

QSqlDatabase connectedDatabase(QString *error)
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
        details << QStringLiteral("Oracle/ODBC code: %1").arg(nativeCode);
    if (!databaseText.isEmpty())
        details << databaseText;
    if (!driverText.isEmpty() && driverText != databaseText)
        details << driverText;
    if (details.isEmpty() && !sqlError.text().trimmed().isEmpty())
        details << sqlError.text().trimmed();
    return details.join(QLatin1Char('\n'));
}

// Messages lisibles pour les contraintes du schema approuve.
QString friendlySqlError(const QSqlError &sqlError)
{
    const QString all = sqlError.text() + sqlError.databaseText() + sqlError.nativeErrorCode();
    if (all.contains(QLatin1String("UQ_SERVICE_TITRE"), Qt::CaseInsensitive))
        return QStringLiteral("A service with this title already exists.\n\n"
                              "The approved schema declares SERVICE.TITRE as UNIQUE (UQ_SERVICE_TITRE), "
                              "so each title can be used by only one service.");
    if (all.contains(QLatin1String("CK_SERVICE_PRIX"), Qt::CaseInsensitive))
        return QStringLiteral("The price must be zero or positive.");
    if (all.contains(QLatin1String("CK_SERVICE_DUREES"), Qt::CaseInsensitive))
        return QStringLiteral("The planned and actual durations must be greater than zero.");
    if (all.contains(QLatin1String("FK_AFFECTER_PERSONNEL"), Qt::CaseInsensitive))
        return QStringLiteral("One of the selected staff members no longer exists in Oracle. Reload the page and select the assignees again.");
    return sqlErrorDetails(sqlError);
}

void setSqlError(QString *error, const QSqlQuery &query, const QString &operation = QString())
{
    if (!error)
        return;
    const QString details = friendlySqlError(query.lastError());
    *error = operation.isEmpty() ? details : operation + QLatin1Char('\n') + details;
}

void setDatabaseError(QString *error, const QSqlDatabase &db, const QString &operation)
{
    if (!error)
        return;
    *error = operation + QLatin1Char('\n') + friendlySqlError(db.lastError());
}

QDate parseScreenDate(const QString &text)
{
    QDate d = QDate::fromString(text, screenDateFormat);
    if (!d.isValid())
        d = QLocale::c().toDate(text, screenDateFormat);   // "Oct 20, 2026"
    return d;
}

int minutesBetween(const QString &from, const QString &to)
{
    const QTime a = QTime::fromString(from, timeFormat), b = QTime::fromString(to, timeFormat);
    return (a.isValid() && b.isValid()) ? a.secsTo(b) / 60 : 0;
}

QString addMinutes(const QString &from, int minutes)
{
    const QTime start = QTime::fromString(from, timeFormat);
    if (!start.isValid() || minutes <= 0)
        return {};
    const int total = start.hour() * 60 + start.minute() + minutes;
    if (total >= 24 * 60)
        return QStringLiteral("23:59");   // un service reste dans la journee de sa date prevue
    return QTime(total / 60, total % 60).toString(timeFormat);
}

QVariant nullInt() { return QVariant(QMetaType::fromType<int>()); }
QVariant nullString() { return QVariant(QMetaType::fromType<QString>()); }

double roundedPrice(double price) { return std::round(price * 100.0) / 100.0; }

int plannedMinutes(const ServiceRecord &r) { return minutesBetween(r.startTime, r.endTime); }
int actualMinutes(const ServiceRecord &r)
{
    return r.actualEndTime.isEmpty() ? 0 : minutesBetween(r.startTime, r.actualEndTime);
}

void bindCommonValues(QSqlQuery &q, const ServiceRecord &r)
{
    // L'ordre doit correspondre aux requetes de insert() / update() (tout sauf l'ID).
    const QDate date = parseScreenDate(r.date);
    q.addBindValue(r.title.trimmed());
    q.addBindValue(r.type.isEmpty() ? nullString() : QVariant(r.type));
    q.addBindValue(r.description.trimmed().isEmpty() ? nullString() : QVariant(r.description.trimmed()));
    q.addBindValue(date.toString(QStringLiteral("yyyy-MM-dd")) + QLatin1Char(' ') + r.startTime);
    q.addBindValue(plannedMinutes(r));
    q.addBindValue(r.actualEndTime.isEmpty() ? nullInt() : QVariant(actualMinutes(r)));
    q.addBindValue(r.priority);
    q.addBindValue(r.status);
    q.addBindValue(roundedPrice(r.unitPrice));
}

bool saveAssignees(QSqlDatabase &db, const QString &serviceId, const ServiceRecord &r, QString *error)
{
    QSqlQuery del(db);
    if (!del.prepare(QStringLiteral("DELETE FROM AFFECTER WHERE ID_SERVICE = ?"))) {
        setSqlError(error, del);
        return false;
    }
    del.addBindValue(serviceId.toInt());
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
        if (!ins.prepare(QStringLiteral("INSERT INTO AFFECTER (ID_PERSONNEL, ID_SERVICE) VALUES (?, ?)"))) {
            setSqlError(error, ins);
            return false;
        }
        ins.addBindValue(personnelId);
        ins.addBindValue(serviceId.toInt());
        if (!ins.exec()) {
            setSqlError(error, ins);
            return false;
        }
    }
    return true;
}
} // namespace

QString ServiceModel::validate(const ServiceRecord &item, bool requireId)
{
    if (requireId && item.id.trimmed().toInt() <= 0)
        return QStringLiteral("The service identifier is missing.");
    if (item.title.trimmed().isEmpty() || item.title.trimmed().size() > 150)
        return QStringLiteral("The service title is required (maximum 150 characters).");
    if (item.type.size() > 50)
        return QStringLiteral("The service type is too long (maximum 50 characters).");
    if (item.priority != QLatin1String("High") && item.priority != QLatin1String("Medium")
        && item.priority != QLatin1String("Low"))
        return QStringLiteral("Invalid service priority.");
    if (item.status != QLatin1String("Scheduled") && item.status != QLatin1String("In Progress")
        && item.status != QLatin1String("Completed") && item.status != QLatin1String("Overdue"))
        return QStringLiteral("Invalid service status.");
    if (!parseScreenDate(item.date).isValid())
        return QStringLiteral("The scheduled date is invalid.");
    if (!QTime::fromString(item.startTime, timeFormat).isValid())
        return QStringLiteral("The start time is invalid.");
    const int planned = plannedMinutes(item);
    if (planned <= 0 || planned > 999999)
        return QStringLiteral("The end time must be later than the start time.");
    if (!item.actualEndTime.isEmpty()) {
        const int actual = actualMinutes(item);
        if (actual <= 0 || actual > 999999)
            return QStringLiteral("The actual end time must be later than the start time.");
    }
    if (item.description.trimmed().size() > 4000)
        return QStringLiteral("The description is too long (maximum 4000 characters).");
    if (item.unitPrice < 0.0 || item.unitPrice > 99999999.99)
        return QStringLiteral("The price must be between 0 and 99 999 999.99 DT.");
    if (item.personnelIds.isEmpty() && item.personnelId <= 0)
        return QStringLiteral("Select at least one staff member to assign this service to.");
    return {};
}

bool ServiceModel::loadPersonnel(QVector<ServicePerson> &people, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;
    QSqlQuery q(db);
    if (!q.exec(QStringLiteral("SELECT ID_PERSONNEL, PRENOM, NOM, STATUT FROM PERSONNEL "
                               "ORDER BY NOM, PRENOM"))) {
        setSqlError(error, q, QStringLiteral("The staff list could not be loaded."));
        return false;
    }
    QVector<ServicePerson> loaded;
    while (q.next()) {
        ServicePerson p;
        p.id = q.value(0).toInt();
        p.name = (q.value(1).toString().trimmed() + QLatin1Char(' ') + q.value(2).toString().trimmed()).trimmed();
        p.active = q.value(3).toString().compare(QStringLiteral("Inactive"), Qt::CaseInsensitive) != 0;
        loaded.append(p);
    }
    people = loaded;
    if (error)
        error->clear();
    return true;
}

bool ServiceModel::loadAll(QVector<ServiceRecord> &items, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery q(db);
    if (!q.exec(QStringLiteral(
            "SELECT ID_SERVICE, TITRE, TYPE_SERVICE, DESCRIPTION, "
            "TO_CHAR(DATE_PREVUE, 'YYYY-MM-DD HH24:MI'), DUREE_ESTIMEE, DUREE_REELLE, "
            "PRIORITE, STATUT, NVL(PRIX, 0) "
            "FROM SERVICE ORDER BY ID_SERVICE"))) {
        setSqlError(error, q, QStringLiteral("The services could not be loaded."));
        return false;
    }

    QVector<ServiceRecord> loaded;
    while (q.next()) {
        ServiceRecord r;
        r.id = q.value(0).toString();
        r.title = q.value(1).toString();
        r.type = q.value(2).toString();
        r.description = q.value(3).toString();
        const QDateTime planned = QDateTime::fromString(q.value(4).toString(), QStringLiteral("yyyy-MM-dd HH:mm"));
        r.date = planned.isValid() ? planned.date().toString(screenDateFormat) : QString();
        r.startTime = planned.isValid() ? planned.time().toString(timeFormat) : QStringLiteral("08:00");
        r.endTime = addMinutes(r.startTime, q.value(5).toInt());
        r.actualEndTime = q.value(6).isNull() ? QString() : addMinutes(r.startTime, q.value(6).toInt());
        r.priority = q.value(7).toString();
        r.status = q.value(8).toString();
        r.unitPrice = q.value(9).toDouble();
        loaded.append(r);
    }

    // Membres affectes : AFFECTER -> PERSONNEL.
    QSqlQuery links(db);
    if (!links.exec(QStringLiteral(
            "SELECT A.ID_SERVICE, P.ID_PERSONNEL, P.PRENOM, P.NOM "
            "FROM AFFECTER A JOIN PERSONNEL P ON P.ID_PERSONNEL = A.ID_PERSONNEL "
            "ORDER BY A.ID_SERVICE, P.NOM, P.PRENOM"))) {
        setSqlError(error, links, QStringLiteral("The service assignments could not be loaded."));
        return false;
    }
    QHash<QString, QVector<int>> idsByService;
    QHash<QString, QStringList> namesByService;
    while (links.next()) {
        const QString serviceId = links.value(0).toString();
        idsByService[serviceId].append(links.value(1).toInt());
        namesByService[serviceId].append(
            (links.value(2).toString().trimmed() + QLatin1Char(' ') + links.value(3).toString().trimmed()).trimmed());
    }
    for (ServiceRecord &r : loaded) {
        r.personnelIds = idsByService.value(r.id);
        r.personnelId = r.personnelIds.isEmpty() ? 0 : r.personnelIds.first();
        r.assignee = namesByService.value(r.id).join(QStringLiteral(", "));
    }

    items = loaded;
    if (error)
        error->clear();
    return true;
}

bool ServiceModel::insert(ServiceRecord &item, QString *error) const
{
    const QString problem = validate(item, false);
    if (!problem.isEmpty()) {
        if (error)
            *error = problem;
        return false;
    }
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    if (!db.transaction()) {
        setDatabaseError(error, db, QStringLiteral("Could not start the Oracle transaction."));
        return false;
    }
    // Le schema approuve n'a pas de sequence pour SERVICE : verrou de table + MAX(ID)+1
    // (meme methode que la gestion Maintenance) pour que deux ajouts simultanes ne se melangent pas.
    QSqlQuery lock(db);
    if (!lock.exec(QStringLiteral("LOCK TABLE SERVICE IN EXCLUSIVE MODE"))) {
        setSqlError(error, lock, QStringLiteral("The SERVICE table could not be locked."));
        db.rollback();
        return false;
    }
    QSqlQuery nextId(db);
    if (!nextId.exec(QStringLiteral("SELECT NVL(MAX(ID_SERVICE), 0) + 1 FROM SERVICE")) || !nextId.next()) {
        setSqlError(error, nextId, QStringLiteral("The next service identifier could not be read."));
        db.rollback();
        return false;
    }
    const int newId = nextId.value(0).toInt();

    QSqlQuery q(db);
    if (!q.prepare(QStringLiteral(
            "INSERT INTO SERVICE (ID_SERVICE, TITRE, TYPE_SERVICE, DESCRIPTION, DATE_PREVUE, "
            "DUREE_ESTIMEE, DUREE_REELLE, PRIORITE, STATUT, PRIX) "
            "VALUES (?, ?, ?, ?, TO_TIMESTAMP(?, 'YYYY-MM-DD HH24:MI'), ?, ?, ?, ?, ?)"))) {
        setSqlError(error, q);
        db.rollback();
        return false;
    }
    q.addBindValue(newId);
    bindCommonValues(q, item);
    if (!q.exec()) {
        setSqlError(error, q, QStringLiteral("The service could not be added."));
        db.rollback();
        return false;
    }
    if (!saveAssignees(db, QString::number(newId), item, error)) {
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        setDatabaseError(error, db, QStringLiteral("Oracle could not commit the new service."));
        db.rollback();
        return false;
    }
    item.id = QString::number(newId);
    if (error)
        error->clear();
    return true;
}

bool ServiceModel::update(const ServiceRecord &item, QString *error) const
{
    const QString problem = validate(item, true);
    if (!problem.isEmpty()) {
        if (error)
            *error = problem;
        return false;
    }
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    if (!db.transaction()) {
        setDatabaseError(error, db, QStringLiteral("Could not start the Oracle transaction for editing."));
        return false;
    }
    QSqlQuery q(db);
    if (!q.prepare(QStringLiteral(
            "UPDATE SERVICE SET TITRE = ?, TYPE_SERVICE = ?, DESCRIPTION = ?, "
            "DATE_PREVUE = TO_TIMESTAMP(?, 'YYYY-MM-DD HH24:MI'), DUREE_ESTIMEE = ?, DUREE_REELLE = ?, "
            "PRIORITE = ?, STATUT = ?, PRIX = ? WHERE ID_SERVICE = ?"))) {
        setSqlError(error, q);
        db.rollback();
        return false;
    }
    bindCommonValues(q, item);
    q.addBindValue(item.id.toInt());
    if (!q.exec()) {
        setSqlError(error, q, QStringLiteral("The service could not be updated."));
        db.rollback();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The service no longer exists in Oracle.");
        db.rollback();
        return false;
    }
    if (!saveAssignees(db, item.id, item, error)) {
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        setDatabaseError(error, db, QStringLiteral("Oracle could not commit the changes."));
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
    if (id.toInt() <= 0) {
        if (error)
            *error = QStringLiteral("The service identifier is missing.");
        return false;
    }
    if (!db.transaction()) {
        setDatabaseError(error, db, QStringLiteral("Could not start the Oracle transaction for deletion."));
        return false;
    }
    // Les lignes d'association sont supprimees avec le service, dans la meme transaction.
    QSqlQuery links(db);
    if (!links.prepare(QStringLiteral("DELETE FROM AFFECTER WHERE ID_SERVICE = ?"))) {
        setSqlError(error, links);
        db.rollback();
        return false;
    }
    links.addBindValue(id.toInt());
    if (!links.exec()) {
        setSqlError(error, links, QStringLiteral("The service assignments could not be removed."));
        db.rollback();
        return false;
    }
    QSqlQuery q(db);
    if (!q.prepare(QStringLiteral("DELETE FROM SERVICE WHERE ID_SERVICE = ?"))) {
        setSqlError(error, q);
        db.rollback();
        return false;
    }
    q.addBindValue(id.toInt());
    if (!q.exec()) {
        setSqlError(error, q, QStringLiteral("The service could not be deleted."));
        db.rollback();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("The service no longer exists in Oracle.");
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        setDatabaseError(error, db, QStringLiteral("Oracle could not commit the deletion."));
        db.rollback();
        return false;
    }
    if (error)
        error->clear();
    return true;
}
