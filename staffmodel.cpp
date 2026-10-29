#include "staffmodel.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

using Staff::Member;

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
QVariant dateText(const QDate &date)
{
    return date.isValid() ? QVariant(date.toString(QStringLiteral("yyyy-MM-dd")))
                          : QVariant::fromValue(QString());
}

void bindMember(QSqlQuery &query, const Member &m)
{
    query.bindValue(QStringLiteral(":id"), m.id.trimmed());
    query.bindValue(QStringLiteral(":name"), m.name.trimmed());
    query.bindValue(QStringLiteral(":email"), m.email.trimmed());
    query.bindValue(QStringLiteral(":role"), m.role.trimmed());
    query.bindValue(QStringLiteral(":status"), m.status);
    query.bindValue(QStringLiteral(":phone"), m.phone.trimmed());
    query.bindValue(QStringLiteral(":joined"), dateText(QDate::fromString(m.joined, Qt::ISODate)));
    query.bindValue(QStringLiteral(":from"), dateText(m.unavailabilityStart));
    query.bindValue(QStringLiteral(":to"), dateText(m.unavailabilityEnd));
}
}

bool StaffModel::isConnected()
{
    const QSqlDatabase db = QSqlDatabase::database();
    return db.isValid() && db.isOpen();
}

QString StaffModel::validate(const Member &m)
{
    if (m.id.trimmed().isEmpty() || m.id.trimmed().size() > 20)
        return QStringLiteral("Invalid staff identifier.");
    if (m.name.trimmed().isEmpty() || m.name.size() > 150)
        return QStringLiteral("Enter the staff member's full name (maximum 150 characters).");
    if (m.email.trimmed().isEmpty() || m.email.size() > 254)
        return QStringLiteral("Enter a valid email address.");
    if (m.role.trimmed().isEmpty() || m.role.size() > 120)
        return QStringLiteral("Enter the professional function (maximum 120 characters).");
    if (m.phone.size() > 40)
        return QStringLiteral("The phone number cannot exceed 40 characters.");
    if (m.status != QLatin1String("Active") && m.status != QLatin1String("Inactive"))
        return QStringLiteral("Invalid staff status.");
    if (m.unavailabilityStart.isValid() != m.unavailabilityEnd.isValid()
        || (m.unavailabilityStart.isValid() && m.unavailabilityEnd < m.unavailabilityStart))
        return QStringLiteral("The unavailability period is invalid.");
    return {};
}

bool StaffModel::afficher(QList<Member> *members, QString *error) const
{
    if (!members) {
        if (error) *error = QStringLiteral("The staff list destination is invalid.");
        return false;
    }
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
            "SELECT STAFF_ID, FULL_NAME, EMAIL, PROFESSIONAL_ROLE, STATUS, PHONE, JOINED_DATE, "
            "UNAVAILABLE_FROM, UNAVAILABLE_TO FROM SKYSENSE_STAFF ORDER BY STAFF_ID DESC"))) {
        setSqlError(error, query);
        return false;
    }

    QList<Member> loaded;
    while (query.next()) {
        Member m;
        m.id = query.value(0).toString();
        m.name = query.value(1).toString();
        m.email = query.value(2).toString();
        m.role = query.value(3).toString();
        m.status = query.value(4).toString();
        m.phone = query.value(5).toString();
        const QDate joined = query.value(6).toDate();
        m.joined = joined.isValid() ? joined.toString(Qt::ISODate) : QString();
        m.unavailabilityStart = query.value(7).toDate();
        m.unavailabilityEnd = query.value(8).toDate();
        loaded.append(m);
    }
    *members = loaded;
    if (error) error->clear();
    return true;
}

bool StaffModel::ajouter(const Member &member, QString *error) const
{
    const QString problem = validate(member);
    if (!problem.isEmpty()) {
        if (error) *error = problem;
        return false;
    }
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "INSERT INTO SKYSENSE_STAFF (STAFF_ID, FULL_NAME, EMAIL, PROFESSIONAL_ROLE, STATUS, "
            "PHONE, JOINED_DATE, UNAVAILABLE_FROM, UNAVAILABLE_TO) "
            "VALUES (:id, :name, :email, :role, :status, :phone, "
            "TO_DATE(:joined, 'YYYY-MM-DD'), TO_DATE(:from, 'YYYY-MM-DD'), "
            "TO_DATE(:to, 'YYYY-MM-DD'))"))) {
        setSqlError(error, query);
        return false;
    }
    bindMember(query, member);
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (error) error->clear();
    return true;
}

bool StaffModel::modifier(const Member &member, QString *error) const
{
    const QString problem = validate(member);
    if (!problem.isEmpty()) {
        if (error) *error = problem;
        return false;
    }
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral(
            "UPDATE SKYSENSE_STAFF SET FULL_NAME = :name, EMAIL = :email, "
            "PROFESSIONAL_ROLE = :role, STATUS = :status, PHONE = :phone, "
            "JOINED_DATE = TO_DATE(:joined, 'YYYY-MM-DD'), "
            "UNAVAILABLE_FROM = TO_DATE(:from, 'YYYY-MM-DD'), "
            "UNAVAILABLE_TO = TO_DATE(:to, 'YYYY-MM-DD') WHERE STAFF_ID = :id"))) {
        setSqlError(error, query);
        return false;
    }
    bindMember(query, member);
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error) *error = QStringLiteral("The staff member no longer exists in Oracle.");
        return false;
    }
    if (error) error->clear();
    return true;
}

bool StaffModel::supprimer(const QString &id, QString *error) const
{
    QSqlDatabase db = connectedDatabase(error);
    if (!db.isValid())
        return false;

    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral("DELETE FROM SKYSENSE_STAFF WHERE STAFF_ID = :id"))) {
        setSqlError(error, query);
        return false;
    }
    query.bindValue(QStringLiteral(":id"), id.trimmed());
    if (!query.exec()) {
        setSqlError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error) *error = QStringLiteral("The staff member no longer exists in Oracle.");
        return false;
    }
    if (error) error->clear();
    return true;
}
