#ifndef STAFFDATA_H
#define STAFFDATA_H

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QMap>
#include <QString>

namespace Staff {
struct Member {
    QString id, name, email, role, department, status, login, phone, joined;
    QString note{};
};

struct AuditEntry {
    QString timestamp, actor, memberId, memberName, action, details;
};

struct State {
    QList<Member> members;
    QHash<QString, QList<bool>> permissions;
    QList<AuditEntry> audit;
    int nextId = 13;
    int revision = 0;
};

struct Statistics {
    int total = 0, active = 0, inactive = 0;
    QMap<QString, int> departments, roles;
};

QString operatorName();
QString defaultStoragePath();
Statistics statistics(const State &state);
bool effectiveAccess(const State &state, const QString &id, int module);
QString validateMember(const State &state, const Member &member, const QString &exceptId = {});

// Local transactional storage. A failed write never replaces the previous file.
// Revisions and a lock also prevent two open instances from losing each other's changes.
class Store {
public:
    explicit Store(QString path);
    QString path() const { return filePath; }
    bool exists() const;
    bool load(State &state, QString &error) const;
    bool save(State &state, QString &error) const;
private:
    QString filePath;
};
}
#endif
