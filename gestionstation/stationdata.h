#ifndef SKYSENSE_GESTIONSTATION_STATIONDATA_H
#define SKYSENSE_GESTIONSTATION_STATIONDATA_H

#include <QString>
#include <QVector>

namespace GestionStation {

// Pure C++ model for the STATION entity. Database statements stay outside the UI.
struct StationRecord {
    QString id;
    QString name;
    QString type;
    QString cityRegion;
    int equipment = 0;
    int availability = 0;
    QString status;
    QString lastUpdate;
};

class StationModel final {
public:
    bool ensureSchema(QString *error = nullptr) const;
    bool afficher(QVector<StationRecord> *records, QString *error = nullptr) const;
    bool ajouter(const StationRecord &record, QString *error = nullptr) const;
    bool modifier(const StationRecord &record, const QString &originalId,
                  QString *error = nullptr) const;
    bool supprimer(const QString &stationId, QString *error = nullptr) const;
};

} // namespace GestionStation

#endif // SKYSENSE_GESTIONSTATION_STATIONDATA_H
