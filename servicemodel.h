#ifndef SERVICEMODEL_H
#define SERVICEMODEL_H

#include <QString>
#include <QStringList>
#include <QVector>

// One row of the Oracle table SKYSENSE_SERVICES.
struct ServiceRecord {
    QString id, title, type, stationCode, stationName, assignee, date, startTime, endTime, priority, status;
    QString description;
    QStringList checklist;
    QString actualEndTime;   // real end time, filled when the service is Completed
    int stationId = 0, personnelId = 0;   // personnelId = first assignee (kept for older code)
    QVector<int> personnelIds;            // ALL assignees (table SKYSENSE_AFFECTER: one service -> 1..n people)
    double quantity = 1.0, unitPrice = 0.0;
    QVector<bool> checklistDone;

    // "assignee" holds the display text of every assignee, e.g. "Alex Martin, Sophie Chen".
    QStringList assigneeList() const {
        return assignee.isEmpty() ? QStringList() : assignee.split(QStringLiteral(", "));
    }
};

// Oracle persistence for Services (ODBC DSN SKYSENSE). The UI calls this
// model; all SQL stays here. Autocommit is ON in QODBC, so every successful
// INSERT / UPDATE / DELETE is immediately visible in SQL*Plus.
class ServiceModel final {
public:
    bool loadAll(QVector<ServiceRecord> &items, QString *error = nullptr) const;
    bool insert(const ServiceRecord &item, QString *error = nullptr) const;
    bool update(const ServiceRecord &item, QString *error = nullptr) const;
    bool remove(const QString &id, QString *error = nullptr) const;

    // Controle de saisie done in C++ before any query is sent (used by insert / update).
    static QString validate(const ServiceRecord &item);
};

#endif // SERVICEMODEL_H
