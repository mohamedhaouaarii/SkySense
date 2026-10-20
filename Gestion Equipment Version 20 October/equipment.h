#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include "equipmentdata.h"
#include <QStringList>

// Pure C++ model for the EQUIPEMENT entity. All Oracle statements live here,
// separate from the Qt widgets that display and edit the records.
class EquipmentModel final {
public:
    bool afficher(QVector<EquipmentRecord> *records, QString *error = nullptr) const;
    bool ajouter(const EquipmentRecord &record, QString *error = nullptr) const;
    bool modifier(const EquipmentRecord &record, const QString &originalSerial,
                  QString *error = nullptr) const;
    bool supprimer(const QString &serialNumber, QString *error = nullptr) const;
    bool stations(QStringList *stationNames, QString *error = nullptr) const;
};

#endif // EQUIPMENT_H
