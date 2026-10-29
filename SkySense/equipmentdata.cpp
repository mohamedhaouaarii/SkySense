#include "equipmentdata.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <QCollator>
#include <algorithm>

static QString normalized(QString value)
{
    value = value.normalized(QString::NormalizationForm_D).toCaseFolded();
    value.remove(QRegularExpression("[\\p{Mn}]"));
    return value.simplified();
}

QString validateEquipment(const QVector<EquipmentRecord> &records)
{
    QSet<QString> ids, serials;
    const QStringList types{"Weather Station","Anemometer","Rain Gauge","Pyranometer","Barometer","Sky Camera"};
    const QStringList statuses{"Online","Offline","Maintenance"};
    for (const auto &r : records) {
        const QString serial = r.serialNumber.trimmed().toCaseFolded();
        if (!QRegularExpression("^EQP-[0-9]{3,9}$").match(r.id).hasMatch() || ids.contains(r.id))
            return "Equipment IDs must be valid and unique.";
        if (r.model.trimmed().isEmpty() || r.model.size() > 100)
            return "Enter a model (1–100 characters).";
        if (serial.isEmpty() || r.serialNumber.size() > 80 || r.serialNumber.contains(QRegularExpression("[\\r\\n\\t]")))
            return "Enter a serial number (1–80 characters, on one line).";
        if (serials.contains(serial)) return "This serial number already belongs to another equipment item.";
        if (!types.contains(r.type) || !statuses.contains(r.status) || r.station.trimmed().isEmpty() || r.station.size() > 200)
            return "Select a valid type, station and status.";
        if (!QDateTime::fromString(r.updated, Qt::ISODate).isValid())
            return "The equipment update date is invalid.";
        ids.insert(r.id); serials.insert(serial);
    }
    return {};
}

QVector<EquipmentRecord> filterEquipment(const QVector<EquipmentRecord> &records,
    const QString &query, const QString &type, const QString &station, const QString &status,
    int column, bool ascending)
{
    QVector<EquipmentRecord> result;
    const QStringList words = normalized(query).split(' ', Qt::SkipEmptyParts);
    for (const auto &r : records) {
        if ((!type.isEmpty() && r.type != type) || (!station.isEmpty() && r.station != station) ||
            (!status.isEmpty() && r.status != status)) continue;
        const QString haystack = normalized(r.id + " " + r.type + " " + r.station + " " + r.unit + " " + r.status + " " + r.model + " " + r.serialNumber);
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

EquipmentStore::EquipmentStore(const QString &path)
    : filePath(path) {}

QString EquipmentStore::nextId() const { return QString("EQP-%1").arg(nextNumber,3,10,QChar('0')); }

bool EquipmentStore::load(const QVector<EquipmentRecord> &initialRecords, QString *error)
{
    if (filePath.isEmpty()) filePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/equipment/demo_inventory_v2.json";
    writable = false; items.clear(); nextNumber = 1; loadedHash.clear();
    if (!QFileInfo::exists(filePath)) {
        items = initialRecords;
    } else {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) { *error = file.errorString(); return false; }
        const QByteArray bytes = file.readAll();
        QJsonParseError parse;
        const auto doc = QJsonDocument::fromJson(bytes,&parse);
        const auto obj = doc.object();
        if (parse.error != QJsonParseError::NoError || obj.value("version").toInt() != 1 || !obj.value("equipment").isArray()) {
            *error = "The inventory file is damaged or has an unsupported format. Your file has been preserved. Restore a valid copy and restart.";
            return false;
        }
        for (const auto value : obj.value("equipment").toArray()) {
            const auto row = value.toObject();
            items.append({row["id"].toString(),row["type"].toString(),row["station"].toString(),row["unit"].toString(),
                          row["updated"].toString(),row["status"].toString(),row["model"].toString(),row["serialNumber"].toString(),row["photoBase64"].toString()});
        }
        nextNumber = qMax(1,obj.value("nextNumber").toInt(1));
        loadedHash = QCryptographicHash::hash(bytes,QCryptographicHash::Sha256);
    }
    *error = validateEquipment(items);
    if (!error->isEmpty()) { items.clear(); return false; }
    for (const auto &r : items) nextNumber = qMax(nextNumber,r.id.mid(4).toInt()+1);
    writable = true;
    return true;
}

bool EquipmentStore::save(const QVector<EquipmentRecord> &records, QString *error)
{
    if (!writable) { *error = "The inventory could not be loaded. Restore the data file and restart before making changes."; return false; }
    *error = validateEquipment(records);
    if (!error->isEmpty()) return false;
    if (!QDir().mkpath(QFileInfo(filePath).absolutePath())) { *error = "Cannot create the inventory folder."; return false; }
    QLockFile lock(filePath + ".lock");
    if (!lock.tryLock(0)) { *error = "Another application is saving this inventory. Try again shortly."; return false; }
    QByteArray currentHash;
    if (QFileInfo::exists(filePath)) {
        QFile existing(filePath);
        if (!existing.open(QIODevice::ReadOnly)) { *error = existing.errorString(); return false; }
        currentHash = QCryptographicHash::hash(existing.readAll(),QCryptographicHash::Sha256);
    }
    if (currentHash != loadedHash) {
        *error = "The inventory was changed by another window or application. Restart SkySense to load the latest version before saving.";
        return false;
    }
    QJsonArray array;
    int newNext = nextNumber;
    for (const auto &r : records) {
        array.append(QJsonObject{{"id",r.id},{"type",r.type},{"station",r.station},{"unit",r.unit},{"updated",r.updated},
                                {"status",r.status},{"model",r.model},{"serialNumber",r.serialNumber},{"photoBase64",r.photoBase64}});
        newNext = qMax(newNext,r.id.mid(4).toInt()+1);
    }
    const QByteArray bytes = QJsonDocument(QJsonObject{{"version",1},{"nextNumber",newNext},{"equipment",array}}).toJson();
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        *error = "Could not save the inventory: " + file.errorString(); return false;
    }
    items = records; nextNumber = newNext;
    loadedHash = QCryptographicHash::hash(bytes,QCryptographicHash::Sha256);
    return true;
}
