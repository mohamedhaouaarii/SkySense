#ifndef INTERVENTION_H
#define INTERVENTION_H

#include <QDateTime>
#include <QString>
#include <QVector>

// The Oracle schema links interventions to staff through REALISER. Equipment
// and checklist fields remain available only in the standalone preview UI.
struct Intervention {
    QString id;                 // Human-readable CODE stored in MAINTENANCE.
    qint64 databaseId = 0;      // Numeric ID_MAINTENANCE primary key.
    QString equipId;            // No MAINTENANCE-to-EQUIPEMENT link in this schema.
    QString title;
    QString tech;
    int technicianId = 0;
    QVector<int> technicianIds; // Preserve all REALISER rows when editing one technician.
    QString priority;
    QString status;
    QString desc;
    QString report;
    QDateTime plannedStart;     // DATE_PREVUE
    QDateTime plannedEnd;       // Preview-only; Oracle stores one planned timestamp.
    QDateTime realStart;        // Preview-only; Oracle stores one completion timestamp.
    QDateTime realEnd;          // DATE_REALISEE
    int actionsMask = 0;        // Preview-only; no checklist column in Oracle.
};

struct Technician {
    QString name;
    bool active = true;
    bool certified = false;    // Certification is not a PERSONNEL column.
    int id = 0;
};

#endif // INTERVENTION_H
