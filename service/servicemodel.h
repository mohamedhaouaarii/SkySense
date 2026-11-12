#ifndef SERVICEMODEL_H
#define SERVICEMODEL_H

#include <QString>
#include <QStringList>
#include <QVector>

// Une ligne de l'ecran Services.
//
// Correspondance avec le schema Oracle approuve (tables SERVICE, AFFECTER, PERSONNEL) :
//   id            <-> SERVICE.ID_SERVICE (numerique, attribue par le modele a l'ajout)
//   title         <-> SERVICE.TITRE (UNIQUE dans le schema approuve)
//   type          <-> SERVICE.TYPE_SERVICE
//   description   <-> SERVICE.DESCRIPTION
//   date+startTime<-> SERVICE.DATE_PREVUE
//   endTime       <-> SERVICE.DUREE_ESTIMEE (minutes, depuis startTime)
//   actualEndTime <-> SERVICE.DUREE_REELLE (minutes, depuis startTime)
//   priority      <-> SERVICE.PRIORITE
//   status        <-> SERVICE.STATUT
//   unitPrice     <-> SERVICE.PRIX (2 decimales)
//   personnelIds  <-> AFFECTER (ID_PERSONNEL, ID_SERVICE)
// Non enregistres (absents du schema approuve) : station, checklist, quantite.
struct ServiceRecord {
    QString id, title, type, stationCode, stationName, assignee, date, startTime, endTime, priority, status;
    QString description;
    QStringList checklist;
    QString actualEndTime;                // heure de fin reelle, renseignee quand le service est Completed
    int stationId = 0, personnelId = 0;   // personnelId = premier membre affecte
    QVector<int> personnelIds;            // tous les membres affectes (AFFECTER)
    double quantity = 1.0, unitPrice = 0.0;
    QVector<bool> checklistDone;

    // "assignee" contient les noms affiches, ex. "Alex Martin, Sophie Chen".
    QStringList assigneeList() const {
        return assignee.isEmpty() ? QStringList() : assignee.split(QStringLiteral(", "));
    }
};

struct ServicePerson {
    int id = 0;
    QString name;   // "Prenom Nom"
    bool active = true;
};

// Persistance Oracle des services. La vue n'a aucune requete SQL : elle appelle ce modele.
// Utilise la connexion partagee Connection (aucune seconde connexion Oracle).
class ServiceModel final {
public:
    bool loadAll(QVector<ServiceRecord> &items, QString *error = nullptr) const;
    bool loadPersonnel(QVector<ServicePerson> &people, QString *error = nullptr) const;
    // Attribue l'ID (MAX+1 sous verrou) et le renseigne dans item.id.
    bool insert(ServiceRecord &item, QString *error = nullptr) const;
    bool update(const ServiceRecord &item, QString *error = nullptr) const;
    bool remove(const QString &id, QString *error = nullptr) const;

    // Controle de saisie en C++ avant toute requete (id non controle pour un ajout).
    static QString validate(const ServiceRecord &item, bool requireId);
};

#endif // SERVICEMODEL_H
