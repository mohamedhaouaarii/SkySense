#ifndef APPSESSION_H
#define APPSESSION_H
#include <QString>

namespace AppSession {
void start(const QString &email, const QString &fullName = QString(), const QString &role = QStringLiteral("Responsable"));
void clear();
bool active();
QString email();
QString displayName();
QString role();
QString nameFromEmail(const QString &email);
}
#endif
