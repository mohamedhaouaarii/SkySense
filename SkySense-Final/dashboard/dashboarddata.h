#pragma once
#include <QDateTime>
#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include "equipmentdata.h"

// Un equipement tel que le tableau de bord l'affiche.
struct DashboardEquipment {
    QString id, type, model, follower, status;
    QDateTime updated;
    int ageDays = -1;        // jours depuis la derniere mise a jour (-1 = inconnue)
    bool attention = false;  // hors ligne, en maintenance ou mise a jour trop ancienne
};
// Nombre d'equipements pour un type.
struct DashboardType {
    QString type;
    int count = 0;
};
struct DashboardSnapshot {
    bool available = false;
    QDateTime refreshed;
    int equipment = 0, online = 0, offline = 0, servicing = 0, other = 0;
    int stale = 0;           // mises a jour plus anciennes que staleDays
    int attention = 0;       // equipements a surveiller
    int followers = 0;       // responsables distincts (SUIVRE)
    QVector<DashboardType> types;          // tries par nombre decroissant
    QVector<DashboardEquipment> watchList; // equipements a surveiller, les plus urgents d'abord
};

// Meme seuil que la gestion Equipement : une mise a jour de staleDays jours ou plus est signalee.
constexpr int staleDays = EquipmentUpdateReviewDays;

// Projection en lecture seule de la table EQUIPEMENT (via Equipement::statistiques()).
bool loadDashboardSnapshot(const QSqlDatabase &db, DashboardSnapshot *result, QString *error);
