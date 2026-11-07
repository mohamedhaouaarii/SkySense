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

// Les dates et les CLOB ne sont plus liés comme QDateTime / QString longs :
// le pilote QODBC + Oracle répond "unknown error" sur ces liaisons.
// On envoie du texte et Oracle convertit (TO_TIMESTAMP / TO_CLOB), comme le
// font déjà les modules Stations et Equipment (chaînes simples seulement).
constexpr int kMaxTitle = 150;
constexpr int kMaxCode = 30;
constexpr int kMaxText = 3900; // limite sûre d'un VARCHAR2 lié vers TO_CLOB

bool isOneOf(const QString &value, const QStringList &allowed)
{
    for (const QString &candidate : allowed)
        if (value.compare(candidate, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

// Règles métier vérifiées à chaque ajout et modification.
bool validateIntervention(const Intervention &item, QString *error)
{
    QString problem;
    const QString status = item.status.trimmed();
    if (item.title.trimmed().isEmpty())
        problem = QStringLiteral("Le titre de l'intervention est obligatoire.");
    else if (item.title.trimmed().size() > kMaxTitle)
        problem = QStringLiteral("Le titre dépasse %1 caractères.").arg(kMaxTitle);
    else if (item.id.trimmed().size() > kMaxCode)
        problem = QStringLiteral("Le code dépasse %1 caractères.").arg(kMaxCode);
    else if (!isOneOf(item.priority.trimmed(), {QStringLiteral("High"), QStringLiteral("Medium"), QStringLiteral("Low")}))
        problem = QStringLiteral("Priorité invalide (High, Medium ou Low).");
    else if (!isOneOf(status, {QStringLiteral("Planned"), QStringLiteral("In Progress"), QStringLiteral("Completed")}))
        problem = QStringLiteral("Statut invalide (Planned, In Progress ou Completed).");
    else if (!item.plannedStart.isValid())
        problem = QStringLiteral("La date prévue est obligatoire et doit être valide.");
    else if (item.desc.trimmed().size() > kMaxText)
        problem = QStringLiteral("La description est trop longue (%1 caractères max).").arg(kMaxText);
    else if (item.report.trimmed().size() > kMaxText)
        problem = QStringLiteral("Le compte rendu est trop long (%1 caractères max).").arg(kMaxText);
    else if (status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0) {
        if (item.report.trimmed().isEmpty())
            problem = QStringLiteral("Un compte rendu est obligatoire pour terminer une intervention.");
        else if (!item.realEnd.isValid())
            problem = QStringLiteral("La date de réalisation est obligatoire pour une intervention terminée.");
        else if (item.realEnd > QDateTime::currentDateTime().addSecs(60))
            problem = QStringLiteral("La date de réalisation ne peut pas être dans le futur.");
    }
    if (problem.isEmpty())
        return true;
    if (error)
        *error = problem;
    return false;
}

QVector<int> assignedTechnicians(const Intervention &item);
QString timestampSql(const QDateTime &dateTime);

// Vérifications côté Oracle : technicien existant, code et doublon.
bool checkAgainstDatabase(const QSqlDatabase &db, const Intervention &item, bool isNew, QString *error)
{
    const QVector<int> technicians = assignedTechnicians(item);
    for (int technicianId : technicians) {
        QSqlQuery query(db);
        if (!query.prepare(QStringLiteral("SELECT COUNT(*) FROM PERSONNEL WHERE ID_PERSONNEL = :id"))) {
            setSqlError(error, query);
            return false;
        }
        query.bindValue(QStringLiteral(":id"), technicianId);
        if (!query.exec() || !query.next()) {
            setSqlError(error, query);
            return false;
        }
        if (query.value(0).toInt() == 0) {
            if (error)
                *error = QStringLiteral("Le technicien choisi n'existe plus dans PERSONNEL. Rechargez la liste.");
            return false;
        }
    }

    // Doublon : même titre + même date prévue (hors l'intervention modifiée).
    QSqlQuery duplicate(db);
    if (!duplicate.prepare(QStringLiteral(
            "SELECT COUNT(*) FROM MAINTENANCE WHERE UPPER(TITRE) = UPPER(:title) "
            "AND DATE_PREVUE = %1 AND ID_MAINTENANCE <> %2").arg(timestampSql(item.plannedStart)).arg(isNew ? 0 : item.databaseId))) {
        setSqlError(error, duplicate);
        return false;
    }
    duplicate.bindValue(QStringLiteral(":title"), item.title.trimmed());
    if (!duplicate.exec() || !duplicate.next()) {
        setSqlError(error, duplicate);
        return false;
    }
    if (duplicate.value(0).toInt() > 0) {
        if (error)
            *error = QStringLiteral("Une intervention avec le même titre et la même date prévue existe déjà.");
        return false;
    }
    duplicate.finish();
    return true;
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

    // Entiers validés (issus d'Oracle) : écrits en littéral, la liaison ODBC échoue ici.
    const QString maintenanceId = QString::number(item.databaseId);
    QSqlQuery remove(db);
    if (!remove.exec(QStringLiteral("DELETE FROM REALISER WHERE ID_MAINTENANCE = %1").arg(maintenanceId))) {
        setSqlError(error, remove, QStringLiteral("Suppression des anciennes affectations (REALISER)."));
        return false;
    }
    remove.finish();

    for (int technicianId : ids) {
        QSqlQuery insert(db);
        if (!insert.exec(QStringLiteral(
                "INSERT INTO REALISER (ID_PERSONNEL, ID_MAINTENANCE) VALUES (%1, %2)")
                             .arg(technicianId).arg(maintenanceId))) {
            setSqlError(error, insert, QStringLiteral("Insertion de l'affectation (REALISER) pour le technicien %1, intervention %2.")
                                           .arg(technicianId).arg(maintenanceId));
            return false;
        }
        insert.finish();
    }
    return true;
}

// SQL littéral sûr pour un timestamp : valeur issue d'un QDateTime formaté.
QString timestampSql(const QDateTime &dateTime)
{
    if (!dateTime.isValid())
        return QStringLiteral("NULL");
    return QStringLiteral("TO_TIMESTAMP('%1', 'YYYY-MM-DD HH24:MI:SS')")
        .arg(dateTime.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
}

// Les NULL sont écrits en littéral SQL (jamais liés) : QODBC échoue sur eux avec Oracle.
QString clobSql(const QString &name)
{
    return QStringLiteral("TO_CLOB(:%1)").arg(name);
}

QString completedAtSql(const Intervention &item)
{
    const bool completed = item.status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0;
    return timestampSql(completed ? item.realEnd : QDateTime());
}

// Prépare INSERT/UPDATE : les colonnes texte vides deviennent NULL dans le SQL.
QString textSql(const QString &value, const QString &name)
{
    return value.trimmed().isEmpty() ? QStringLiteral("NULL") : clobSql(name);
}

void bindMaintenanceFields(QSqlQuery &query, const Intervention &item)
{
    query.bindValue(QStringLiteral(":code"), item.id.trimmed());
    query.bindValue(QStringLiteral(":title"), item.title.trimmed());
    query.bindValue(QStringLiteral(":priority"), item.priority.trimmed());
    query.bindValue(QStringLiteral(":status"), item.status.trimmed());
    if (!item.desc.trimmed().isEmpty())
        query.bindValue(QStringLiteral(":description"), item.desc.trimmed());
    if (!item.report.trimmed().isEmpty())
        query.bindValue(QStringLiteral(":report"), item.report.trimmed());
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
            *error = QStringLiteral("Choisissez un membre du personnel actif pour cette intervention.");
        return false;
    }
    if (!validateIntervention(item, error))
        return false;

    if (!db.transaction()) {
        setDatabaseError(error, db, QStringLiteral("Impossible de démarrer la transaction Oracle."));
        return false;
    }

    item.technicianIds = technicians;
    if (!checkAgainstDatabase(db, item, true, error)) {
        db.rollback();
        return false;
    }

    // Pas de LOCK TABLE (refusé par QODBC) : l'ID est MAX+1 comme dans le module Compte,
    // et on recommence si un autre poste a pris le même ID entre-temps.
    bool inserted = false;
    QString lastError;
    for (int attempt = 0; attempt < 5 && !inserted; ++attempt) {
        QSqlQuery nextId(db);
        if (!nextId.exec(QStringLiteral("SELECT NVL(MAX(ID_MAINTENANCE), 0) + 1 FROM MAINTENANCE"))
            || !nextId.next()) {
            setSqlError(error, nextId, QStringLiteral("[1/3] Calcul du nouvel ID_MAINTENANCE."));
            db.rollback();
            return false;
        }
        item.databaseId = nextId.value(0).toLongLong();
        nextId.finish();
        item.id = QStringLiteral("MT-%1-%2")
                      .arg(QDate::currentDate().year())
                      .arg(item.databaseId, 3, 10, QLatin1Char('0'));

        QSqlQuery query(db);
        const QString sql = QStringLiteral(
            "INSERT INTO MAINTENANCE (ID_MAINTENANCE, CODE, TITRE, DATE_PREVUE, DATE_REALISEE, "
            "PRIORITE, STATUT, DESCRIPTION, COMPTE_RENDU) VALUES "
            "(%1, :code, :title, %2, %3, :priority, :status, %4, %5)")
            .arg(item.databaseId)
            .arg(timestampSql(item.plannedStart), completedAtSql(item),
                 textSql(item.desc, QStringLiteral("description")),
                 textSql(item.report, QStringLiteral("report")));
        if (!query.prepare(sql)) {
            setSqlError(error, query, QStringLiteral("[2/3] Préparation de l'INSERT MAINTENANCE."));
            db.rollback();
            return false;
        }
        bindMaintenanceFields(query, item);
        if (query.exec()) {
            inserted = true;
            break;
        }
        lastError = sqlErrorDetails(query.lastError());
        const bool duplicateKey = lastError.contains(QStringLiteral("ORA-00001"))
                                  || lastError.contains(QStringLiteral("UQ_MAINTENANCE_CODE"))
                                  || lastError.contains(QStringLiteral("PK_MAINTENANCE"));
        if (!duplicateKey) {
            setSqlError(error, query, QStringLiteral("[2/3] INSERT dans MAINTENANCE refusé."));
            db.rollback();
            return false;
        }
    }
    if (!inserted) {
        if (error)
            *error = QStringLiteral("Impossible d'obtenir un identifiant libre. Réessayez.\n") + lastError;
        db.rollback();
        return false;
    }

    if (!replaceTechnicians(db, item, error)) {
        if (error)
            *error = QStringLiteral("[3/3] Affectation du technicien (REALISER).\n") + *error;
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        setDatabaseError(error, db, QStringLiteral("Oracle n'a pas pu valider (COMMIT) l'ajout."));
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
            *error = QStringLiteral("L'intervention n'a pas d'ID Oracle valide. Rechargez la liste.");
        return false;
    }
    if (!validateIntervention(item, error))
        return false;

    if (!db.transaction()) {
        setDatabaseError(error, db, QStringLiteral("Impossible de démarrer la transaction Oracle."));
        return false;
    }
    if (!checkAgainstDatabase(db, item, false, error)) {
        db.rollback();
        return false;
    }

    QSqlQuery query(db);
    const QString sql = QStringLiteral(
        "UPDATE MAINTENANCE SET CODE = :code, TITRE = :title, DATE_PREVUE = %1, "
        "DATE_REALISEE = %2, PRIORITE = :priority, STATUT = :status, "
        "DESCRIPTION = %3, COMPTE_RENDU = %4 WHERE ID_MAINTENANCE = %5")
        .arg(timestampSql(item.plannedStart), completedAtSql(item),
             textSql(item.desc, QStringLiteral("description")),
             textSql(item.report, QStringLiteral("report")))
        .arg(item.databaseId);
    if (!query.prepare(sql)) {
        setSqlError(error, query, QStringLiteral("Préparation de l'UPDATE MAINTENANCE."));
        db.rollback();
        return false;
    }
    bindMaintenanceFields(query, item);
    if (!query.exec()) {
        setSqlError(error, query, QStringLiteral("UPDATE de MAINTENANCE refusé."));
        db.rollback();
        return false;
    }
    if (query.numRowsAffected() == 0) {
        if (error)
            *error = QStringLiteral("Cette intervention n'existe plus dans Oracle. Rechargez la liste.");
        db.rollback();
        return false;
    }
    query.finish();
    if (!replaceTechnicians(db, item, error)) {
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        setDatabaseError(error, db, QStringLiteral("Oracle n'a pas pu valider (COMMIT) la modification."));
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
