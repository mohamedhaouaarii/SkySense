#include "appsession.h"
#include <QRegularExpression>

namespace {
QString currentEmail;
QString currentName;
QString currentRole;
}

namespace AppSession {
QString nameFromEmail(const QString &email)
{
    QString local = email.trimmed().section('@', 0, 0);
    local.replace(QRegularExpression(QStringLiteral("[._-]+")), QStringLiteral(" "));
    QStringList parts = local.split(' ', Qt::SkipEmptyParts);
    for (QString &part : parts)
        part = part.left(1).toUpper() + part.mid(1).toLower();
    return parts.isEmpty() ? QStringLiteral("Local operator") : parts.join(' ');
}
void start(const QString &email, const QString &fullName, const QString &role) {
    currentEmail = email.trimmed();
    currentName = fullName.trimmed().isEmpty() ? nameFromEmail(currentEmail) : fullName.trimmed();
    currentRole = role.trimmed().isEmpty() ? QStringLiteral("Responsable") : role.trimmed();
}
void clear() { currentEmail.clear(); currentName.clear(); currentRole.clear(); }
bool active() { return !currentEmail.isEmpty(); }
QString email() { return currentEmail; }
QString displayName() { return currentName.isEmpty() ? QStringLiteral("Local operator") : currentName; }
QString role() { return currentRole.isEmpty() ? QStringLiteral("Responsable") : currentRole; }
}
