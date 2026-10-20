#include "equipmentdata.h"
#include "equipment.h"
#include "connection.h"
#include <QDateTime>
#include <QRegularExpression>
#include <QCollator>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <algorithm>

static QString normalized(QString value)
{
    value = value.normalized(QString::NormalizationForm_D).toCaseFolded();
    value.remove(QRegularExpression(QStringLiteral("\\p{M}+")));
    return value.simplified();
}

QString validateEquipment(const QVector<EquipmentRecord> &records)
{
    QSet<QString> ids, serials;
    const QStringList types{"Weather Station","Anemometer","Rain Gauge","Pyranometer","Barometer","Sky Camera"};
    const QStringList statuses{"Online","Offline","Maintenance"};
    for (const auto &r : records) {
        const QString id = r.id.trimmed().toUpper();
        const QString serial = r.serialNumber.trimmed().toCaseFolded();
        if (!QRegularExpression("^EQP-[0-9]{3,9}$").match(id).hasMatch() || ids.contains(id))
            return "Equipment IDs must be valid and unique.";
        if (r.model.trimmed().isEmpty() || r.model.size() > 100)
            return "Enter a model (1–100 characters).";
        if (serial.isEmpty() || r.serialNumber.size() > 80 || r.serialNumber.contains(QRegularExpression("[\\r\\n\\t]")))
            return "Enter a serial number (1–80 characters, on one line).";
        if (serials.contains(serial)) return "This serial number already belongs to another equipment item.";
        if (!types.contains(r.type) || !statuses.contains(r.status) || r.station.size() > 100 ||
            r.location.size() > 150 || r.unit.size() > 20)
            return "Select a valid equipment type and status, and keep the station, location and unit within their limits.";
        if (!QDateTime::fromString(r.updated, Qt::ISODate).isValid() ||
            QDateTime::fromString(r.updated, Qt::ISODate) > QDateTime::currentDateTime().addSecs(60))
            return "The equipment update date is invalid.";
        ids.insert(id); serials.insert(serial);
    }
    return {};
}

EquipmentAssessment assessEquipment(const EquipmentRecord &record, const QDateTime &now)
{
    EquipmentAssessment result;
    const QDateTime updated = QDateTime::fromString(record.updated, Qt::ISODate);
    if (!updated.isValid() || updated > now.addSecs(60)) {
        result.freshness = QStringLiteral("Update date missing or invalid");
        result.recommendation = QStringLiteral("Correct the last update date before relying on this equipment record.");
        return result;
    }

    result.daysSinceUpdate = qMax(0, updated.date().daysTo(now.date()));
    result.needsReview = result.daysSinceUpdate >= EquipmentUpdateReviewDays ||
                         record.status != QStringLiteral("Online");
    result.freshness = result.daysSinceUpdate == 0
        ? QStringLiteral("Updated today")
        : QStringLiteral("Last updated %1 day%2 ago")
              .arg(result.daysSinceUpdate)
              .arg(result.daysSinceUpdate == 1 ? QString() : QStringLiteral("s"));
    if (result.daysSinceUpdate >= EquipmentUpdateReviewDays)
        result.freshness += QStringLiteral(" · older than %1 days").arg(EquipmentUpdateReviewDays);

    if (record.status == QStringLiteral("Offline"))
        result.recommendation = QStringLiteral("Check power, reception and communication, then update the equipment status.");
    else if (record.status == QStringLiteral("Maintenance"))
        result.recommendation = QStringLiteral("Follow the maintenance intervention and update the record after the work.");
    else if (result.daysSinceUpdate >= EquipmentUpdateReviewDays)
        result.recommendation = QStringLiteral("Verify the current status and location at the next site visit.");
    else
        result.recommendation = QStringLiteral("No immediate action is indicated; review the equipment at the next scheduled check.");
    return result;
}

QVector<EquipmentRecord> compatibleReserveReplacements(
    const QVector<EquipmentRecord> &records, const EquipmentRecord &selected)
{
    QVector<EquipmentRecord> result;
    if (selected.station.trimmed().isEmpty())
        return result;

    for (const EquipmentRecord &candidate : records) {
        if (candidate.id == selected.id || !candidate.station.trimmed().isEmpty() ||
            candidate.status.compare(QStringLiteral("Online"), Qt::CaseInsensitive) != 0 ||
            candidate.type.compare(selected.type, Qt::CaseInsensitive) != 0 ||
            candidate.model.compare(selected.model, Qt::CaseInsensitive) != 0 ||
            candidate.unit.compare(selected.unit, Qt::CaseInsensitive) != 0 ||
            candidate.serialNumber.compare(selected.serialNumber, Qt::CaseInsensitive) == 0)
            continue;
        result.append(candidate);
    }
    return result;
}

QVector<EquipmentRecord> filterEquipment(const QVector<EquipmentRecord> &records,
    const QString &query, const QString &type, const QString &station, const QString &status,
    int column, bool ascending)
{
    QVector<EquipmentRecord> result;
    const QStringList words = normalized(query).split(' ', Qt::SkipEmptyParts);
    for (const auto &r : records) {
        const bool reserveFilter = station == QStringLiteral("__RESERVE__");
        if ((!type.isEmpty() && r.type != type) ||
            (!station.isEmpty() && (reserveFilter ? !r.station.trimmed().isEmpty() : r.station != station)) ||
            (!status.isEmpty() && r.status != status)) continue;
        const QString haystack = normalized(r.id + " " + r.type + " " + r.station + " " + r.location + " " + r.unit + " " + r.status + " " + r.model + " " + r.serialNumber);
        if (std::all_of(words.begin(), words.end(), [&](const QString &word){ return haystack.contains(word); }))
            result.append(r);
    }
    auto key = [column](const EquipmentRecord &r) {
        switch(column) {
        case 2: return r.type;
        case 3: return r.station;
        case 4: return r.unit;
        case 5: return r.updated;
        case 6: return r.status;
        default: return r.id;
        }
    };
    QCollator collator; collator.setNumericMode(true); collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::stable_sort(result.begin(), result.end(), [&](const EquipmentRecord &a, const EquipmentRecord &b) {
        int order = column == 5 ? (QDateTime::fromString(a.updated,Qt::ISODate) < QDateTime::fromString(b.updated,Qt::ISODate) ? -1 :
                                  QDateTime::fromString(a.updated,Qt::ISODate) > QDateTime::fromString(b.updated,Qt::ISODate) ? 1 : 0)
                                : collator.compare(key(a),key(b));
        if (!order) order = collator.compare(a.id,b.id);
        return ascending ? order < 0 : order > 0;
    });
    return result;
}

QString EquipmentStore::nextId() const { return QString("EQP-%1").arg(nextNumber, 3, 10, QChar('0')); }

namespace {
bool sameEquipment(const EquipmentRecord &a, const EquipmentRecord &b)
{
    return a.id == b.id && a.type == b.type && a.station == b.station && a.unit == b.unit
        && a.updated == b.updated && a.status == b.status && a.model == b.model
        && a.serialNumber == b.serialNumber && a.photoBase64 == b.photoBase64
        && a.location == b.location;
}
}

bool EquipmentStore::load(const QVector<EquipmentRecord> &initialRecords, QString *error)
{
    if (Connection::createInstance().isConnected()) {
        EquipmentModel model;
        if (!model.afficher(&items, error)) {
            items.clear();
            nextNumber = 1;
            return false;
        }
    } else {
        items = initialRecords;
        if (items.isEmpty()) items = {
            {"EQP-001","Weather Station","Alpine Ridge","", "2026-10-18T21:45:00","Online","Vaisala WXT536","WS536-78421",{}},
            {"EQP-002","Anemometer","Coastal Bay","m/s","2026-10-18T22:07:00","Maintenance","WindMaster Pro","ANE-20489",{}},
            {"EQP-003","Rain Gauge","Pine Forest","mm","2026-10-12T10:20:00","Offline","RG-200","RG-20470",{}},
            {"EQP-004","Pyranometer","Green Valley","W/m²","2026-10-18T10:41:00","Online","SMP10","PYR-49024",{}},
            {"EQP-005","Barometer","Alpine Ridge","hPa","2026-10-17T10:15:00","Maintenance","PTB110","BAR-11142",{}},
            {"EQP-006","Sky Camera","Desert View","","2026-10-18T10:37:00","Online","SkyCam 4K","CAM-38001",{}},
            {"EQP-007","Weather Station","","","2026-10-18T09:32:00","Online","Vaisala WXT536","WS536-78422",{}},
            {"EQP-008","Rain Gauge","","mm","2026-10-17T11:40:00","Online","RG-200","RG-20471",{}}
        };
    }
    nextNumber = 1;
    for (const auto &item : items) nextNumber = qMax(nextNumber, item.id.mid(4).toInt() + 1);
    const QString validation = validateEquipment(items);
    if (!validation.isEmpty()) {
        items.clear();
        nextNumber = 1;
        if (error) *error = validation;
        return false;
    }
    if (error) error->clear();
    return true;
}

bool EquipmentStore::save(const QVector<EquipmentRecord> &records, QString *error)
{
    const QString validation = validateEquipment(records);
    if (!validation.isEmpty()) { if (error) *error = validation; return false; }

    if (Connection::createInstance().isConnected()) {
        QSqlDatabase db = Connection::createInstance().database();
        if (!db.transaction()) {
            if (error) *error = db.lastError().text();
            return false;
        }
        EquipmentModel model;
        auto fail = [&](const QString &message) {
            db.rollback();
            if (error) *error = message;
            return false;
        };
        for (const EquipmentRecord &oldRecord : items) {
            const bool stillExists = std::any_of(records.cbegin(), records.cend(), [&](const EquipmentRecord &record) {
                return record.id.compare(oldRecord.id, Qt::CaseInsensitive) == 0;
            });
            if (!stillExists) {
                QString dbError;
                if (!model.supprimer(oldRecord.serialNumber, &dbError)) return fail(dbError);
            }
        }
        for (const EquipmentRecord &record : records) {
            const auto old = std::find_if(items.cbegin(), items.cend(), [&](const EquipmentRecord &candidate) {
                return candidate.id.compare(record.id, Qt::CaseInsensitive) == 0;
            });
            QString dbError;
            if (old == items.cend()) {
                if (!model.ajouter(record, &dbError)) return fail(dbError);
            } else if (!sameEquipment(*old, record)) {
                if (!model.modifier(record, old->serialNumber, &dbError)) return fail(dbError);
            }
        }
        if (!db.commit()) return fail(db.lastError().text());
    }

    items = records;
    for (const auto &item : items) nextNumber = qMax(nextNumber, item.id.mid(4).toInt() + 1);
    if (error) error->clear();
    return true;
}

QStringList EquipmentStore::stationNames(QString *error) const
{
    if (Connection::createInstance().isConnected()) {
        QStringList names;
        EquipmentModel model;
        if (!model.stations(&names, error)) return {};
        return names;
    }
    QStringList names;
    for (const EquipmentRecord &record : items) {
        const QString name = record.station.trimmed();
        if (!name.isEmpty() && !names.contains(name, Qt::CaseInsensitive)) names.append(name);
    }
    if (error) error->clear();
    return names;
}
