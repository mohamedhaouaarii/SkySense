#ifndef MAINTENANCEMODEL_H
#define MAINTENANCEMODEL_H

#include "intervention.h"

#include <QVector>

// Oracle persistence for Maintenance. UI classes call this model; SQL stays here.
class MaintenanceModel final {
public:
    bool loadAll(QVector<Intervention> &items, QString *error = nullptr) const;
    bool insert(Intervention &item, QString *error = nullptr) const;
    bool update(const Intervention &item, QString *error = nullptr) const;
    bool remove(const QString &id, QString *error = nullptr) const;
};

#endif // MAINTENANCEMODEL_H
