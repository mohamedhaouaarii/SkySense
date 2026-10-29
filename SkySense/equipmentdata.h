#ifndef EQUIPMENTDATA_H
#define EQUIPMENTDATA_H
#include <QString>
#include <QDate>
#include <QVector>

struct EquipmentRecord {
    QString id, type, station, unit, updated, status, model, serialNumber;
    QString photoBase64{};
};

QString validateEquipment(const QVector<EquipmentRecord> &records);
QVector<EquipmentRecord> filterEquipment(const QVector<EquipmentRecord> &records,
    const QString &query, const QString &type, const QString &station, const QString &status,
    int column = 1, bool ascending = true);

class EquipmentStore {
public:
    explicit EquipmentStore(const QString &filePath = QString());
    bool load(const QVector<EquipmentRecord> &initialRecords, QString *error);
    bool save(const QVector<EquipmentRecord> &records, QString *error);
    const QVector<EquipmentRecord> &records() const { return items; }
    QString nextId() const;
    QString path() const { return filePath; }
private:
    QString filePath;
    QVector<EquipmentRecord> items;
    QByteArray loadedHash;
    int nextNumber = 1;
    bool writable = false;
};

bool exportEquipmentPdf(const QString &filePath, const QVector<EquipmentRecord> &records,
                        const QDate &reportDate, QString *error);
#endif
