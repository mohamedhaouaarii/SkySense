#pragma once
#include "staffdata.h"

namespace StaffProfile {
bool load(QHash<QString,QList<bool>> &permissions, QList<Staff::AuditEntry> &history, QString &error);
bool saveGrants(const QString &id, const QList<bool> &previous, const QList<bool> &grants, QString &error);
}
