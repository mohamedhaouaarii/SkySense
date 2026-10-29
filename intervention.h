#ifndef INTERVENTION_H
#define INTERVENTION_H

#include <QDateTime>
#include <QString>

// Maintenance entity shared by the UI and the persistence model.
struct Intervention {
    QString id;
    QString equipId;
    QString title;
    QString tech;
    QString priority;
    QString status;
    QString desc;
    QString report;
    QDateTime plannedStart;
    QDateTime plannedEnd;
    QDateTime realStart;
    QDateTime realEnd;
    int actionsMask = 0;
};

#endif // INTERVENTION_H
