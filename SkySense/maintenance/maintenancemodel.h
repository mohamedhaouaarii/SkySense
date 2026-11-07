#ifndef MAINTENANCEMODEL_H
#define MAINTENANCEMODEL_H

#include "intervention.h"

#include <QVector>

// Oracle persistence for the approved SkySense MAINTENANCE + REALISER schema.
class MaintenanceModel final {
public:
    bool loadAll(QVector<Intervention> &items, QString *error = nullptr) const;
    bool loadTechnicians(QVector<Technician> &items, QString *error = nullptr) const;
    bool insert(Intervention &item, QString *error = nullptr) const;
    bool update(const Intervention &item, QString *error = nullptr) const;
    bool remove(const Intervention &item, QString *error = nullptr) const;
};

#endif // MAINTENANCEMODEL_H
