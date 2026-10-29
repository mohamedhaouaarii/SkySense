#ifndef STATIONMODEL_H
#define STATIONMODEL_H

#include <QString>
#include <QVector>

// One row of the Oracle table SKYSENSE_STATIONS.
struct StationRecord {
    QString id;            // STATION_ID
    QString name;          // STATION_NAME
    QString type;          // STATION_TYPE
    QString cityRegion;    // CITY_REGION  ("City\nRegion")
    int equipment = 0;     // EQUIPMENT_COUNT
    int availability = 0;  // AVAILABILITY_PCT (0..100)
    QString status;        // Online / Maintenance / Offline
    QString lastUpdate;    // LAST_UPDATE, ISO date yyyy-MM-dd
};

// Pure C++ model for the Station entity (atelier CRUD architecture).
// All SQL lives here: prepared queries + bound values, never behind a button.
class StationModel final {
public:
    static bool isConnected();

    // Controle de saisie (done in C++ before any query is sent).
    static QString validate(const StationRecord &station);

    bool afficher(QVector<StationRecord> *stations, QString *error = nullptr) const;               // READ
    bool ajouter(const StationRecord &station, QString *error = nullptr) const;                    // CREATE
    bool modifier(const StationRecord &station, const QString &originalId,
                  QString *error = nullptr) const;                                                  // UPDATE
    bool supprimer(const QString &id, QString *error = nullptr) const;                             // DELETE
};

#endif // STATIONMODEL_H
