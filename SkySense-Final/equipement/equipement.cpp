#include "equipement.h"
#include "connection.h"

#include <QByteArray>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QVariant>
#include <algorithm>
#include <memory>

namespace
{
QVariant nullableString(const QString &value)
{
    return value.trimmed().isEmpty() ? QVariant(QMetaType::fromType<QString>()) : QVariant(value.trimmed());
}

QVariant nullablePhoto(const QString &photoBase64)
{
    return photoBase64.isEmpty() ? QVariant(QMetaType::fromType<QByteArray>())
                                 : QVariant(QByteArray::fromBase64(photoBase64.toLatin1()));
}

QVariant updateDate(const QString &value)
{
    const QDateTime date = QDateTime::fromString(value, Qt::ISODate);
    return date.isValid() ? QVariant(date) : QVariant(QMetaType::fromType<QDateTime>());
}

QSqlDatabase database(QString *error)
{
    Connection &connection = Connection::createInstance();
    if (!connection.isConnected())
    {
        if (error)
            *error = QStringLiteral("Oracle is not connected. Reopen the application and connect with the "
                                    "SKYSENSE ODBC data source.");
        return {};
    }
    return connection.database();
}

void setQueryError(QString *error, const QSqlQuery &query)
{
    if (error)
        *error = query.lastError().text();
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
    query.bindValue(QStringLiteral(":personnel"), record.personnelId > 0 ? QVariant(record.personnelId)
                                                                       : QVariant(QMetaType::fromType<int>()));
}

QString insertSql()
{
    return QStringLiteral(
        "INSERT INTO EQUIPEMENT (IDENTIFIANT_EQUIPEMENT, NUMERO_SERIE, TYPE_EQUIPEMENT, MODELE, UNITE, "
        "EMPLACEMENT, STATUT, DATE_DERNIERE_MAJ, PHOTO, ID_PERSONNEL) "
        "VALUES (:identifier, :serial, :type, :model, :unit, :location, :status, :updated, :photo, "
        ":personnel)");
}
} // namespace

bool Equipement::charger(QVector<EquipmentRecord> *records, QString *error) const
{
    if (!records)
    {
        if (error)
            *error = QStringLiteral("The equipment list destination is invalid.");
        return false;
    }
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;

    std::unique_ptr<QSqlQueryModel> model(afficher(error));
    if (!model)
        return false;
    while (model->canFetchMore())
        model->fetchMore();
    QVector<EquipmentRecord> loaded;
    for (int rowIndex = 0; rowIndex < model->rowCount(); ++rowIndex)
    {
        const QSqlRecord query = model->record(rowIndex);
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
        record.personnelId = query.value(10).isNull() ? 0 : query.value(10).toInt();
        loaded.append(record);
    }
    *records = loaded;
    if (error)
        error->clear();
    return true;
}

namespace
{
bool requireFollower(const EquipmentRecord &record, QString *error)
{
    if (record.personnelId > 0)
        return true;
    if (error)
        *error = QStringLiteral("Select the staff member who follows this equipment (SUIVRE is mandatory, 1,1).");
    return false;
}
} // namespace

bool Equipement::ajouter(QString *error) const
{
    const EquipmentRecord &record = getDonnees();
    if (!requireFollower(record, error))
        return false;
    const QString validation = validateEquipment({record});
    if (!validation.isEmpty())
    {
        if (error)
            *error = validation;
        return false;
    }
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;
    QSqlQuery query(db);
    if (!query.prepare(insertSql()))
    {
        setQueryError(error, query);
        return false;
    }
    bindRecord(query, record);
    if (!query.exec())
    {
        setQueryError(error, query);
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool Equipement::modifier(const QString &originalId, QString *error) const
{
    const EquipmentRecord &record = getDonnees();
    if (!requireFollower(record, error))
        return false;
    const QString validation = validateEquipment({record});
    if (!validation.isEmpty())
    {
        if (error)
            *error = validation;
        return false;
    }
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;
    QSqlQuery query(db);
    if (!query.prepare(
            QStringLiteral("UPDATE EQUIPEMENT SET NUMERO_SERIE=:serial, TYPE_EQUIPEMENT=:type, MODELE=:model, "
                           "UNITE=:unit, EMPLACEMENT=:location, STATUT=:status, DATE_DERNIERE_MAJ=:updated, "
                           "PHOTO=:photo, ID_PERSONNEL=:personnel WHERE IDENTIFIANT_EQUIPEMENT=:identifier")))
    {
        setQueryError(error, query);
        return false;
    }
    if (record.id.trimmed().compare(originalId.trimmed(), Qt::CaseInsensitive) != 0)
    {
        if (error)
            *error = QStringLiteral("The equipment identifier is the primary key and cannot be changed.");
        return false;
    }
    bindRecord(query, record);
    if (!query.exec())
    {
        setQueryError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0)
    {
        if (error)
            *error = QStringLiteral("The equipment was not found in Oracle; refresh the list and try again.");
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool Equipement::supprimer(const QString &identifier, QString *error) const
{
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;
    QSqlQuery query(db);
    if (!query.prepare(QStringLiteral("DELETE FROM EQUIPEMENT WHERE IDENTIFIANT_EQUIPEMENT=:identifier")))
    {
        setQueryError(error, query);
        return false;
    }
    query.bindValue(QStringLiteral(":identifier"), identifier.trimmed());
    if (!query.exec())
    {
        setQueryError(error, query);
        return false;
    }
    if (query.numRowsAffected() == 0)
    {
        if (error)
            *error = QStringLiteral("The equipment was not found in Oracle; refresh the list and try again.");
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool Equipement::personnel(QVector<EquipmentPersonChoice> *people, QString *error) const
{
    if (!people)
    {
        if (error)
            *error = QStringLiteral("The staff list destination is invalid.");
        return false;
    }
    const QSqlDatabase db = database(error);
    if (!db.isValid() || !db.isOpen())
        return false;
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("SELECT ID_PERSONNEL, PRENOM, NOM FROM PERSONNEL ORDER BY UPPER(PRENOM), UPPER(NOM)")))
    {
        setQueryError(error, query);
        return false;
    }
    QVector<EquipmentPersonChoice> loaded;
    while (query.next())
        loaded.append({query.value(0).toInt(),
                       (query.value(1).toString() + QLatin1Char(' ') + query.value(2).toString()).trimmed()});
    *people = loaded;
    if (error)
        error->clear();
    return true;
}

QSqlQueryModel *Equipement::afficher(QString *error) const
{
    auto *model = new QSqlQueryModel();
    if (!Connection::createInstance().isConnected())
    {
        if (error)
            *error = QStringLiteral("Oracle is not connected.");
        delete model;
        return nullptr;
    }
    // SUIVRE : le libelle du responsable est lu par jointure sur #ID_PERSONNEL.
    model->setQuery(QStringLiteral("SELECT E.IDENTIFIANT_EQUIPEMENT, E.TYPE_EQUIPEMENT, "
                                   "TRIM(P.PRENOM || ' ' || P.NOM), E.EMPLACEMENT, E.UNITE, E.DATE_DERNIERE_MAJ, "
                                   "E.STATUT, E.MODELE, E.NUMERO_SERIE, E.PHOTO, E.ID_PERSONNEL FROM EQUIPEMENT E "
                                   "LEFT JOIN PERSONNEL P ON P.ID_PERSONNEL=E.ID_PERSONNEL "
                                   "ORDER BY E.IDENTIFIANT_EQUIPEMENT"),
                    Connection::createInstance().database());
    if (model->lastError().isValid())
    {
        if (error)
            *error = model->lastError().text();
        delete model;
        return nullptr;
    }
    model->setHeaderData(0, Qt::Horizontal, QStringLiteral("Identifiant"));
    model->setHeaderData(1, Qt::Horizontal, QStringLiteral("Type"));
    model->setHeaderData(2, Qt::Horizontal, QStringLiteral("Responsable (suivre)"));
    model->setHeaderData(3, Qt::Horizontal, QStringLiteral("Emplacement"));
    model->setHeaderData(4, Qt::Horizontal, QStringLiteral("Unite"));
    model->setHeaderData(5, Qt::Horizontal, QStringLiteral("Derniere mise a jour"));
    model->setHeaderData(6, Qt::Horizontal, QStringLiteral("Statut"));
    model->setHeaderData(7, Qt::Horizontal, QStringLiteral("Modele"));
    model->setHeaderData(8, Qt::Horizontal, QStringLiteral("Numero de serie"));
    model->setHeaderData(9, Qt::Horizontal, QStringLiteral("Photo"));
    model->setHeaderData(10, Qt::Horizontal, QStringLiteral("ID personnel"));
    if (error)
        error->clear();
    return model;
}

namespace
{
bool sameEquipmentRecord(const EquipmentRecord &a, const EquipmentRecord &b)
{
    return a.id == b.id && a.type == b.type && a.station == b.station && a.unit == b.unit &&
           a.updated == b.updated && a.status == b.status && a.model == b.model &&
           a.serialNumber == b.serialNumber && a.photoBase64 == b.photoBase64 && a.location == b.location &&
           a.personnelId == b.personnelId;
}
} // namespace

bool Equipement::enregistrerListe(const QVector<EquipmentRecord> &previous,
                                  const QVector<EquipmentRecord> &records, QString *error) const
{
    QSqlDatabase db = Connection::createInstance().database();
    if (!db.transaction())
    {
        if (error)
            *error = db.lastError().text();
        return false;
    }
    Equipement model;
    auto fail = [&](const QString &message)
    {
        db.rollback();
        if (error)
            *error = message;
        return false;
    };
    for (const EquipmentRecord &oldRecord : previous)
    {
        const bool stillExists =
            std::any_of(records.cbegin(), records.cend(), [&](const EquipmentRecord &record)
                        { return record.id.compare(oldRecord.id, Qt::CaseInsensitive) == 0; });
        if (!stillExists)
        {
            QString dbError;
            if (!model.supprimer(oldRecord.id, &dbError))
                return fail(dbError);
        }
    }
    for (const EquipmentRecord &record : records)
    {
        const auto old =
            std::find_if(previous.cbegin(), previous.cend(), [&](const EquipmentRecord &candidate)
                         { return candidate.id.compare(record.id, Qt::CaseInsensitive) == 0; });
        QString dbError;
        if (old == previous.cend())
        {
            if (!Equipement(record).ajouter(&dbError))
                return fail(dbError);
        }
        else if (!sameEquipmentRecord(*old, record))
        {
            if (!Equipement(record).modifier(old->id, &dbError))
                return fail(dbError);
        }
    }
    if (!db.commit())
        return fail(db.lastError().text());
    if (error)
        error->clear();
    return true;
}

Equipement::Equipement(const EquipmentRecord &valeur)
{
    this->id = valeur.id;
    this->type = valeur.type;
    this->station = valeur.station;
    this->unit = valeur.unit;
    this->updated = valeur.updated;
    this->status = valeur.status;
    this->model = valeur.model;
    this->serialNumber = valeur.serialNumber;
    this->photoBase64 = valeur.photoBase64;
    this->location = valeur.location;
    this->personnelId = valeur.personnelId;
}

EquipmentRecord Equipement::getDonnees() const
{
    EquipmentRecord valeur;
    valeur.id = this->id;
    valeur.type = this->type;
    valeur.station = this->station;
    valeur.unit = this->unit;
    valeur.updated = this->updated;
    valeur.status = this->status;
    valeur.model = this->model;
    valeur.serialNumber = this->serialNumber;
    valeur.photoBase64 = this->photoBase64;
    valeur.location = this->location;
    valeur.personnelId = this->personnelId;
    return valeur;
}

void Equipement::setDonnees(const EquipmentRecord &valeur)
{
    this->id = valeur.id;
    this->type = valeur.type;
    this->station = valeur.station;
    this->unit = valeur.unit;
    this->updated = valeur.updated;
    this->status = valeur.status;
    this->model = valeur.model;
    this->serialNumber = valeur.serialNumber;
    this->photoBase64 = valeur.photoBase64;
    this->location = valeur.location;
    this->personnelId = valeur.personnelId;
}

// Lecture seule pour le tableau de bord (meme principe que afficher()).
QSqlQueryModel *Equipement::statistiques(QString *error) const
{
    auto *model = new QSqlQueryModel();
    if (!Connection::createInstance().isConnected())
    {
        if (error)
            *error = QStringLiteral("Oracle is not connected.");
        delete model;
        return nullptr;
    }
    model->setQuery(QStringLiteral("SELECT E.STATUT, (SELECT COUNT(*) FROM GERER G WHERE G.ID_PERSONNEL=E.ID_PERSONNEL), "
                                   "E.IDENTIFIANT_EQUIPEMENT, E.TYPE_EQUIPEMENT, TRIM(P.PRENOM || ' ' || P.NOM), "
                                   "E.DATE_DERNIERE_MAJ, E.MODELE FROM EQUIPEMENT E LEFT JOIN PERSONNEL P ON P.ID_PERSONNEL=E.ID_PERSONNEL"), Connection::createInstance().database());
    if (model->lastError().isValid())
    {
        if (error)
            *error = model->lastError().text();
        delete model;
        return nullptr;
    }
    while (model->canFetchMore())
        model->fetchMore();
    if (error)
        error->clear();
    return model;
}
