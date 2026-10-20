#ifndef EQUIPMENTDATA_H
#define EQUIPMENTDATA_H
#include <QString>
#include <QDate>
#include <QDateTime>
#include <QVector>
#include <QStringList>

struct EquipmentRecord {
    QString id, type, station, unit, updated, status, model, serialNumber;
    QString photoBase64{};
    QString location{};
};

struct EquipmentAssessment {
    int daysSinceUpdate = -1;
    bool needsReview = true;
    QString freshness;
    QString recommendation;
};

constexpr int EquipmentUpdateReviewDays = 30;

QString validateEquipment(const QVector<EquipmentRecord> &records);
EquipmentAssessment assessEquipment(const EquipmentRecord &record,
                                     const QDateTime &now = QDateTime::currentDateTime());
QVector<EquipmentRecord> compatibleReserveReplacements(
    const QVector<EquipmentRecord> &records, const EquipmentRecord &selected);
QVector<EquipmentRecord> filterEquipment(const QVector<EquipmentRecord> &records,
    const QString &query, const QString &type, const QString &station, const QString &status,
    int column = 1, bool ascending = true);

class EquipmentStore {
public:
    EquipmentStore() = default;
    bool load(const QVector<EquipmentRecord> &initialRecords, QString *error);
    bool save(const QVector<EquipmentRecord> &records, QString *error);
    const QVector<EquipmentRecord> &records() const { return items; }
    QString nextId() const;
    QStringList stationNames(QString *error = nullptr) const;
private:
    QVector<EquipmentRecord> items;
    int nextNumber = 1;
};

bool exportEquipmentPdf(const QString &filePath, const QVector<EquipmentRecord> &records,
                        const QDate &reportDate, QString *error, bool selectedOnly = false);
#endif
