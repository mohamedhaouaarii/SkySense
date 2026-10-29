#ifndef STAFFDATA_H
#define STAFFDATA_H

#include <QDateTime>
#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

namespace Staff {
struct Member {
    QString id, name, email, role, status, phone, joined;
    QDate unavailabilityStart, unavailabilityEnd;
};

struct State {
    QList<Member> members;
    int nextId = 13;
    int revision = 0;
};

struct Statistics {
    int total = 0, active = 0, inactive = 0;
    QMap<QString, int> functions;
};

QString operatorName();
Statistics statistics(const State &state);
QString validateMember(const State &state, const Member &member, const QString &exceptId = {});

}
#endif
