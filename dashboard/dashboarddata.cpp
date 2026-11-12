#include "dashboarddata.h"
#include "equipement.h"
#include <QHash>
#include <QSet>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QVariant>
#include <algorithm>
#include <memory>

// Aucune requete ici : les donnees viennent de Equipement::statistiques() (architecture modele-vue).
// Colonnes : 0 STATUT, 1 nb GERER, 2 IDENTIFIANT, 3 TYPE, 4 responsable, 5 DATE_DERNIERE_MAJ, 6 MODELE.
bool loadDashboardSnapshot(const QSqlDatabase &db, DashboardSnapshot *result, QString *error)
{
    *result = {};
    if (error) error->clear();
    if (!db.isValid() || !db.isOpen()) {
        if (error) *error = "Connect to Oracle to load your equipment overview.";
        return false;
    }
    std::unique_ptr<QSqlQueryModel> equipment(Equipement().statistiques(error));
    if (!equipment) return false;

    DashboardSnapshot next;
    const QDate today = QDate::currentDate();
    QHash<QString, int> perType;
    QSet<QString> followers;
    for (int row = 0; row < equipment->rowCount(); ++row) {
        const QSqlRecord r = equipment->record(row);
        DashboardEquipment item;
        item.status = r.value(0).toString().trimmed();
        item.id = r.value(2).toString();
        item.type = r.value(3).toString();
        item.follower = r.value(4).toString().trimmed();
        item.updated = r.value(5).toDateTime();
        item.model = r.value(6).toString();
        item.ageDays = item.updated.isValid() ? int(item.updated.date().daysTo(today)) : -1;

        ++next.equipment;
        const QString status = item.status.toCaseFolded();
        if (status == "online") ++next.online;
        else if (status == "offline") ++next.offline;
        else if (status == "maintenance") ++next.servicing;
        else ++next.other;
        const bool stale = item.ageDays < 0 || item.ageDays >= staleDays;
        next.stale += stale;
        item.attention = status != "online" || stale;
        next.attention += item.attention;
        if (!item.type.isEmpty()) ++perType[item.type];
        if (!item.follower.isEmpty()) followers.insert(item.follower);
        if (item.attention) next.watchList.append(item);
    }
    next.followers = int(followers.size());
    for (auto it = perType.cbegin(); it != perType.cend(); ++it) next.types.append({it.key(), it.value()});
    std::sort(next.types.begin(), next.types.end(), [](const auto &a, const auto &b) {
        return a.count != b.count ? a.count > b.count : a.type < b.type;
    });
    // Hors ligne d'abord, puis maintenance, puis les mises a jour les plus anciennes.
    auto rank = [](const DashboardEquipment &e) {
        const QString s = e.status.toCaseFolded();
        return s == "offline" ? 0 : s == "maintenance" ? 1 : 2;
    };
    std::stable_sort(next.watchList.begin(), next.watchList.end(), [&](const auto &a, const auto &b) {
        if (rank(a) != rank(b)) return rank(a) < rank(b);
        return a.ageDays > b.ageDays;
    });
    next.available = true;
    next.refreshed = QDateTime::currentDateTime();
    *result = next;
    return true;
}
